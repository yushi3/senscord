/*
 * SPDX-FileCopyrightText: 2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "src/senscord_wamr_context.h"

#include <inttypes.h>
#include <signal.h>

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "senscord/osal.h"
#include "c_api/c_common.h"
#include "src/senscord_wamr_util.h"
#include "src/wasm_memory_pool.h"

#if 0
#include <stdio.h>
#define LOG_E(fmt, ...) fprintf(stderr, (fmt "\n"), ##__VA_ARGS__)
#define LOG_W(fmt, ...) fprintf(stderr, (fmt "\n"), ##__VA_ARGS__)
#define LOG_I(fmt, ...) fprintf(stderr, (fmt "\n"), ##__VA_ARGS__)
#define LOG_D(fmt, ...) fprintf(stderr, (fmt "\n"), ##__VA_ARGS__)
#else
#include "senscord/logger.h"
#define LOG_E(...) SENSCORD_LOG_ERROR_TAGGED(kBlockName, __VA_ARGS__)
#define LOG_W(...) SENSCORD_LOG_WARNING_TAGGED(kBlockName, __VA_ARGS__)
#define LOG_I(...) SENSCORD_LOG_INFO_TAGGED(kBlockName, __VA_ARGS__)
#define LOG_D(...) SENSCORD_LOG_DEBUG_TAGGED(kBlockName, __VA_ARGS__)
#endif

namespace {

namespace c_api = senscord::c_api;

// status and log block name
const char kBlockName[] = "wasm";

const int kSignalNumber = SIGUSR1;
const uint64_t kInterruptInterval = 500000000;  // 500ms

struct CoreParam {
  std::set<senscord_stream_t> streams;
};

typedef std::map<std::string, senscord::WasmMemoryPool*> MemoryPoolList;

/**
 * @brief SensCord context.
 */
struct SensCordContext {
  wasm_thread_t thread;
  senscord::osal::OSMutex* mutex;
  senscord::osal::OSCond* cond;
  std::set<senscord_config_t> config_handles;
  std::map<senscord_core_t, CoreParam> core_handles;
  std::set<senscord_stream_t> blocking_stream_handles;
  std::set<wasm_addr_t> context_memory;
  MemoryPoolList memory_pools;

  SensCordContext()
      : thread(), mutex(), cond() {
    senscord::osal::OSCreateMutex(&mutex);
    senscord::osal::OSCreateCond(&cond);
  }

  ~SensCordContext() {
    senscord::osal::OSDestroyCond(cond);
    cond = NULL;
    senscord::osal::OSDestroyMutex(mutex);
    mutex = NULL;
  }
};

static void* g_context_key = NULL;
static senscord::osal::OSMutex* g_mutex = NULL;
static bool g_signal_setup = false;

static volatile sig_atomic_t g_interrupt_flag = 0;
static struct sigaction g_prev_sigaction;

/**
 * @brief Signal handler for interrupt.
 */
void senscord_wamr_sigaction(int sig, siginfo_t* siginfo, void* sig_context) {
  LOG_I("senscord_wamr_sigaction: %d", sig);
  g_interrupt_flag = 1;

  if (g_prev_sigaction.sa_flags & SA_SIGINFO) {
    g_prev_sigaction.sa_sigaction(sig, siginfo, sig_context);
  } else if (g_prev_sigaction.sa_handler != NULL) {
    g_prev_sigaction.sa_handler(sig);
  }
}

/**
 * @brief Deteles the memory pool instance.
 */
struct senscord_context_delete_memory_pool {
  explicit senscord_context_delete_memory_pool(
      SensCordContext* context, wasm_module_inst_t module_inst)
      : context_(context), module_inst_(module_inst) {}

  void operator()(senscord_stream_t stream) {
    const char* stream_key = senscord_stream_get_key(stream);
    if (stream_key != NULL) {
      LockGuard _lock(context_->mutex);
      MemoryPoolList::iterator itr = context_->memory_pools.find(stream_key);
      if (itr != context_->memory_pools.end()) {
        itr->second->Close(stream, module_inst_);
        if (itr->second->IsClosed()) {
          delete itr->second;
          context_->memory_pools.erase(itr);
        }
      }
    }
  }

 private:
  SensCordContext* context_;
  wasm_module_inst_t module_inst_;
};

/**
 * @brief Closes the stream.
 */
struct senscord_core_close_stream_force {
  explicit senscord_core_close_stream_force(
      SensCordContext* context, wasm_module_inst_t module_inst)
      : context_(context), module_inst_(module_inst) {}

  void operator()(const std::pair<senscord_core_t, senscord_stream_t>& itr) {
    LOG_D("senscord_core_close_stream(force): core=%" PRIx64
        ", stream=%" PRIx64, itr.first, itr.second);
    senscord_context_delete_memory_pool func(context_, module_inst_);
    func(itr.second);
    senscord_core_close_stream(itr.first, itr.second);
  }

 private:
  SensCordContext* context_;
  wasm_module_inst_t module_inst_;
};

/**
 * @brief Extracts the blocking streams.
 */
void senscord_context_extract_blocking_streams(
    SensCordContext* context,
    std::vector<std::pair<senscord_core_t, senscord_stream_t> >* streams) {
  for (std::set<senscord_stream_t>::const_iterator
      itr = context->blocking_stream_handles.begin(),
      end = context->blocking_stream_handles.end();
      itr != end; ++itr) {
    senscord_stream_t stream = *itr;
    for (std::map<senscord_core_t, CoreParam>::iterator
        itr2 = context->core_handles.begin(),
        end2 = context->core_handles.end();
        itr2 != end2; ++itr2) {
      senscord_core_t core = itr2->first;
      if (itr2->second.streams.erase(stream) > 0) {
        streams->push_back(std::make_pair(core, stream));
      }
    }
  }
  context->blocking_stream_handles.clear();
}

/**
 * @brief Context thread.
 */
void* senscord_context_thread(
    wasm_exec_env_t exec_env, void* args) {
  LOG_D("senscord_context_thread <S>");
  SensCordContext* context = reinterpret_cast<SensCordContext*>(args);
  wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(exec_env);
  LockGuard _lock(context->mutex);

  while (true) {
    senscord::osal::OSRelativeTimedWaitCond(
        context->cond, context->mutex, kInterruptInterval);
    if (context->thread == 0) {
      break;
    }
    if (g_interrupt_flag) {
      g_interrupt_flag = 0;
      LOG_D("senscord_context_thread: interrupt");
      std::vector<std::pair<senscord_core_t, senscord_stream_t> > streams;
      senscord_context_extract_blocking_streams(context, &streams);
      senscord::osal::OSUnlockMutex(context->mutex);
      std::for_each(
          streams.begin(), streams.end(),
          senscord_core_close_stream_force(context, module_inst));
      senscord::osal::OSLockMutex(context->mutex);
    }
  }

  LOG_D("senscord_context_thread <E>");
  return 0;
}

/**
 * @brief Creates the context thread.
 */
int32_t senscord_context_create_thread(
    wasm_exec_env_t exec_env, SensCordContext* context) {
  int32_t ret = wasm_runtime_spawn_thread(
      exec_env,
      &context->thread,
      senscord_context_thread,
      context);
  LOG_D("senscord_context_create_thread: ret=%" PRId32 ", tid=%" PRIxPTR,
        ret, context->thread);
  return ret;
}

/**
 * @brief Joins the context thread.
 */
void senscord_context_join_thread(SensCordContext* context) {
  wasm_thread_t thread = 0;
  if (context != NULL) {
    LockGuard _lock(context->mutex);
    thread = context->thread;
    context->thread = 0;
    senscord::osal::OSSignalCond(context->cond);
  }
  if (thread != 0) {
    LOG_D("senscord_context_join_thread: tid=%" PRIxPTR, thread);
    wasm_runtime_join_thread(thread, NULL);
  }
}

/**
 * @brief Destroys the config handle.
 */
void senscord_config_destroy_force(senscord_config_t config) {
  LOG_D("senscord_config_destroy(force): config=%" PRIx64, config);
  senscord_config_destroy(config);
}

/**
 * @brief Exits the core handle.
 */
struct senscord_core_exit_force {
  explicit senscord_core_exit_force(
      SensCordContext* context, wasm_module_inst_t module_inst)
      : context_(context), module_inst_(module_inst) {}

  void operator()(const std::pair<senscord_core_t, CoreParam>& itr) {
    LOG_D("senscord_core_exit(force): core=%" PRIx64, itr.first);
    std::for_each(
        itr.second.streams.begin(), itr.second.streams.end(),
        senscord_context_delete_memory_pool(context_, module_inst_));
    senscord_core_exit(itr.first);
  }

 private:
  SensCordContext* context_;
  wasm_module_inst_t module_inst_;
};

/**
 * @brief Destroys the senscord context.
 */
void senscord_context_destroy(
    wasm_module_inst_t module_inst,
    void* context) {
  if (context != NULL) {
    LOG_D("senscord_context_destroy");
    SensCordContext* sc_context = reinterpret_cast<SensCordContext*>(context);
    {
      LockGuard _lock(sc_context->mutex);
      // clear context memory (Don't call `wasm_runtime_module_free`)
      sc_context->context_memory.clear();
      // force release config handles
      std::for_each(
          sc_context->config_handles.begin(),
          sc_context->config_handles.end(),
          senscord_config_destroy_force);
      sc_context->config_handles.clear();
      // force release core handles
      // (module_inst is NULL, so `wasm_runtime_module_free` is not called)
      std::for_each(
          sc_context->core_handles.begin(),
          sc_context->core_handles.end(),
          senscord_core_exit_force(sc_context, NULL));
      sc_context->core_handles.clear();
    }
    senscord_context_join_thread(sc_context);
    delete sc_context;
  }
}

/**
 * @brief Gets the senscord context.
 */
SensCordContext* senscord_context_get_instance(
    wasm_module_inst_t module_inst) {
  SensCordContext* context = NULL;
  if (g_context_key != NULL) {
    context = reinterpret_cast<SensCordContext*>(
        wasm_runtime_get_context(module_inst, g_context_key));
    if (context == NULL) {
      senscord::osal::OSLockMutex(g_mutex);
      context = reinterpret_cast<SensCordContext*>(
          wasm_runtime_get_context(module_inst, g_context_key));
      if (context == NULL) {
        context = new SensCordContext();
        wasm_runtime_set_context_spread(module_inst, g_context_key, context);
      }
      senscord::osal::OSUnlockMutex(g_mutex);
    }
  }
  return context;
}

/**
 * @brief Gets the memory pool instance.
 */
senscord::WasmMemoryPool* senscord_context_get_memory_pool(
    wasm_module_inst_t module_inst, senscord_stream_t stream) {
  senscord::WasmMemoryPool* memory_pool = NULL;
  const char* stream_key = senscord_stream_get_key(stream);
  if (stream_key != NULL) {
    SensCordContext* context = senscord_context_get_instance(module_inst);
    if (context != NULL) {
      LockGuard _lock(context->mutex);
      std::pair<MemoryPoolList::iterator, bool> ret =
          context->memory_pools.insert(
              std::make_pair(stream_key, memory_pool));
      if (ret.second) {
        ret.first->second = new senscord::WasmMemoryPool();
      }
      memory_pool = ret.first->second;
    }
  }
  return memory_pool;
}

}  // namespace

/**
 * @brief Initializes the senscord context.
 * @return 0 for success, -1 for failure.
 */
int32_t senscord_context_init() {
  LOG_D("senscord_context_init");
  if (g_context_key != NULL) {
    LOG_W("senscord_context_init: already initialized");
    return 0;
  }
  // context key
  g_context_key = wasm_runtime_create_context_key(senscord_context_destroy);
  if (g_context_key == NULL) {
    LOG_E("senscord_context_init: wasm_runtime_create_context_key failed");
    return -1;
  }
  // mutex
  senscord::osal::OSCreateMutex(&g_mutex);
  // signal (setup)
  struct sigaction sa;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = senscord_wamr_sigaction;
  int32_t ret = sigaction(kSignalNumber, &sa, &g_prev_sigaction);
  if (ret != 0) {
    LOG_E("senscord_context_init: sigaction failed: %d", errno);
    senscord_context_exit();
    return -1;
  }
  g_signal_setup = true;
  return 0;
}

/**
 * @brief Exits the senscord context.
 */
void senscord_context_exit() {
  LOG_D("senscord_context_exit");
  // signal (restore)
  if (g_signal_setup) {
    g_signal_setup = false;
    sigaction(kSignalNumber, &g_prev_sigaction, NULL);
  }
  // mutex
  senscord::osal::OSDestroyMutex(g_mutex);
  g_mutex = NULL;
  // context key
  wasm_runtime_destroy_context_key(g_context_key);
  g_context_key = NULL;
}

/**
 * @brief Sets the config handle to context.
 * @param[in] exec_env    WASM execution environment.
 * @param[in] config      Config handle.
 * @param[in] operation   Operation type.
 */
void senscord_context_set_config(
    wasm_exec_env_t exec_env,
    senscord_config_t config,
    enum senscord_context_op_t operation) {
  wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(exec_env);
  SensCordContext* context = senscord_context_get_instance(module_inst);
  if (context != NULL) {
    LockGuard _lock(context->mutex);
    if (operation == SENSCORD_CONTEXT_OP_ENTER) {
      LOG_D("senscord_context_set_config: add: config=%" PRIx64, config);
      context->config_handles.insert(config);
    } else if (operation == SENSCORD_CONTEXT_OP_EXIT) {
      LOG_D("senscord_context_set_config: remove: config=%" PRIx64, config);
      context->config_handles.erase(config);
    }
  }
}

/**
 * @brief Sets the core handle to context.
 * @param[in] exec_env    WASM execution environment.
 * @param[in] core        Core handle.
 * @param[in] operation   Operation type.
 * @return 0 for success, -1 for failure.
 */
int32_t senscord_context_set_core(
    wasm_exec_env_t exec_env,
    senscord_core_t core,
    enum senscord_context_op_t operation) {
  wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(exec_env);
  SensCordContext* context = senscord_context_get_instance(module_inst);
  if (context != NULL) {
    if (operation == SENSCORD_CONTEXT_OP_ENTER) {
      LOG_D("senscord_context_set_core: add: core=%" PRIx64, core);
      LockGuard _lock(context->mutex);
      context->core_handles[core] = CoreParam();
      // create thread
      if (context->thread == 0) {
        int32_t ret = senscord_context_create_thread(exec_env, context);
        if (ret != 0) {
          context->core_handles.erase(core);
          c_api::SetLastError(SENSCORD_STATUS_FAIL(
              kBlockName, senscord::Status::kCauseResourceExhausted,
              "wasm_runtime_spawn_thread() failed."));
          return -1;
        }
      }
    } else if (operation == SENSCORD_CONTEXT_OP_EXIT) {
      LOG_D("senscord_context_set_core: remove: core=%" PRIx64, core);
      bool empty = false;
      {
        LockGuard _lock(context->mutex);
        std::map<senscord_core_t, CoreParam>::iterator itr =
            context->core_handles.find(core);
        if (itr != context->core_handles.end()) {
          std::set<senscord_stream_t>& streams = itr->second.streams;
          std::for_each(
              streams.begin(), streams.end(),
              senscord_context_delete_memory_pool(context, module_inst));
          std::set<senscord_stream_t> result;
          std::set_difference(
              context->blocking_stream_handles.begin(),
              context->blocking_stream_handles.end(),
              streams.begin(), streams.end(),
              std::inserter(result, result.end()));
          context->blocking_stream_handles.swap(result);
          context->core_handles.erase(itr);
        }
        empty = context->core_handles.empty();
      }
      if (empty) {
        senscord_context_join_thread(context);
      }
    }
  }
  return 0;
}

/**
 * @brief Sets the stream handle to context.
 * @param[in] exec_env    WASM execution environment.
 * @param[in] stream      Stream handle.
 * @param[in] parent_core Parent core handle.
 * @param[in] operation   Operation type.
 */
void senscord_context_set_stream(
    wasm_exec_env_t exec_env,
    senscord_stream_t stream,
    senscord_core_t parent_core,
    enum senscord_context_op_t operation) {
  wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(exec_env);
  SensCordContext* context = senscord_context_get_instance(module_inst);
  if (context != NULL) {
    LockGuard _lock(context->mutex);
    std::map<senscord_core_t, CoreParam>::iterator itr =
        context->core_handles.find(parent_core);
    if (itr != context->core_handles.end()) {
      if (operation == SENSCORD_CONTEXT_OP_ENTER) {
        LOG_D("senscord_context_set_stream: add: stream=%" PRIx64, stream);
        senscord::WasmMemoryPool* memory_pool =
            senscord_context_get_memory_pool(module_inst, stream);
        if (memory_pool != NULL) {
          memory_pool->Open(stream);
        }
        itr->second.streams.insert(stream);
      } else if (operation == SENSCORD_CONTEXT_OP_EXIT) {
        LOG_D("senscord_context_set_stream: remove: stream=%" PRIx64, stream);
        senscord_context_delete_memory_pool func(context, module_inst);
        func(stream);
        itr->second.streams.erase(stream);
      }
    }
  }
}

/**
 * @brief Sets the blocking stream to context.
 * @param[in] exec_env    WASM execution environment.
 * @param[in] stream      Stream handle.
 * @param[in] operation   Operation type.
 */
void senscord_context_set_blocking_stream(
    wasm_exec_env_t exec_env,
    senscord_stream_t stream,
    enum senscord_context_op_t operation) {
  wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(exec_env);
  SensCordContext* context = senscord_context_get_instance(module_inst);
  if (context != NULL) {
    LockGuard _lock(context->mutex);
    if (operation == SENSCORD_CONTEXT_OP_ENTER) {
      context->blocking_stream_handles.insert(stream);
    } else if (operation == SENSCORD_CONTEXT_OP_EXIT) {
      context->blocking_stream_handles.erase(stream);
    }
  }
}

/**
 * @brief Sets the stream state.
 * @param[in] exec_env    WASM execution environment.
 * @param[in] stream      Stream handle.
 * @param[in] operation   Operation type. (enter=Running, exit=Ready)
 * @return 0 for success, -1 for failure.
 */
int32_t senscord_context_set_stream_running(
    wasm_exec_env_t exec_env,
    senscord_stream_t stream,
    enum senscord_context_op_t operation) {
  wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(exec_env);
  senscord::WasmMemoryPool* memory_pool =
      senscord_context_get_memory_pool(module_inst, stream);
  if (memory_pool != NULL) {
    if (operation == SENSCORD_CONTEXT_OP_ENTER) {
      senscord::Status status = memory_pool->Start(stream, module_inst);
      if (!status.ok()) {
        c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
        return -1;
      }
    } else if (operation == SENSCORD_CONTEXT_OP_EXIT) {
      memory_pool->Stop(stream);
    }
  }
  return 0;
}

/**
 * @brief Configures the memory pool.
 * @param[in] exec_env    WASM execution environment.
 * @param[in] stream      Stream handle.
 * @param[in] num         Number of memory chunks.
 * @param[in] size        Memory chunk size.
 * @return 0 for success, -1 for failure.
 */
int32_t senscord_context_set_memory_pool(
    wasm_exec_env_t exec_env,
    senscord_stream_t stream,
    uint32_t num, uint32_t size) {
  wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(exec_env);
  senscord::WasmMemoryPool* memory_pool =
      senscord_context_get_memory_pool(module_inst, stream);
  if (memory_pool == NULL) {
    c_api::SetLastError(SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseInvalidOperation,
        "memory pool is not available."));
    return -1;
  }
  if (memory_pool->IsRunning()) {
    c_api::SetLastError(SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseInvalidOperation,
        "Cannot be set while running."));
    return -1;
  }
  memory_pool->SetNum(num);
  memory_pool->SetSize(size);
  return 0;
}

/* Frame memory information (for senscord_frame_memory_t) */
struct FrameMemoryInfo {
  wasm_module_inst_t module_inst;
  SensCordContext* context;
  std::string parent_stream_key;
  senscord_frame_t frame;
};

/**
 * @brief Reserves frame memory in memory pool.
 * @param[in] exec_env    WASM execution environment.
 * @param[in] frame       Frame handle.
 * @param[out] frame_memory Frame memory handle.
 * @return 0 for success, -1 for failure.
 */
int32_t senscord_context_reserve_frame_memory(
    wasm_exec_env_t exec_env,
    senscord_frame_t frame,
    senscord_frame_memory_t* frame_memory) {
  SENSCORD_C_API_ARGUMENT_CHECK(frame_memory == NULL);
  senscord_stream_t stream = senscord_frame_get_parent_stream(frame);
  wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(exec_env);
  senscord::WasmMemoryPool* memory_pool =
      senscord_context_get_memory_pool(module_inst, stream);
  if (memory_pool == NULL) {
    c_api::SetLastError(SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseInvalidOperation,
        "memory pool is not available."));
    return -1;
  }
  senscord::Status status = memory_pool->ReserveFrameMemory(module_inst, frame);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
  FrameMemoryInfo* memory_info = new FrameMemoryInfo;
  memory_info->module_inst = module_inst;
  memory_info->context = senscord_context_get_instance(module_inst);
  memory_info->parent_stream_key = senscord_stream_get_key(stream);
  memory_info->frame = frame;
  *frame_memory = c_api::ToHandle(memory_info);
  return 0;
}

/**
 * @brief Releases frame memory.
 * @param[in] frame_memory Frame memory handle.
 */
void senscord_context_release_frame_memory(
    senscord_frame_memory_t frame_memory) {
  FrameMemoryInfo* memory_info =
      c_api::ToPointer<FrameMemoryInfo*>(frame_memory);
  if (memory_info != NULL) {
    SensCordContext* context = memory_info->context;
    LockGuard _lock(context->mutex);
    MemoryPoolList::const_iterator itr =
        context->memory_pools.find(memory_info->parent_stream_key);
    if (itr != context->memory_pools.end()) {
      senscord::WasmMemoryPool* memory_pool = itr->second;
      memory_pool->ReleaseFrameMemory(
          memory_info->module_inst, memory_info->frame);
    }
    delete memory_info;
  }
}

/**
 * @brief Obtains channel memory from memory pool.
 * @param[in] exec_env     WASM execution environment.
 * @param[in] frame        Frame handle.
 * @param[in] channel      Channel handle.
 * @param[out] memory_area Memory area.
 * @return 0 for success, -1 for failure.
 */
int32_t senscord_context_get_channel_memory(
    wasm_exec_env_t exec_env,
    senscord_frame_t frame,
    senscord_channel_t channel,
    struct senscord_wasm_memory_area_t* memory_area) {
  SENSCORD_C_API_ARGUMENT_CHECK(memory_area == NULL);
  senscord_stream_t stream = senscord_frame_get_parent_stream(frame);
  wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(exec_env);
  senscord::WasmMemoryPool* memory_pool =
      senscord_context_get_memory_pool(module_inst, stream);
  if (memory_pool == NULL) {
    c_api::SetLastError(SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseInvalidOperation,
        "memory pool is not available."));
    return -1;
  }
  senscord::WasmMemoryArea area = {};
  senscord::Status status = memory_pool->GetChannelMemory(
      module_inst, frame, channel, &area);
  if (!status.ok()) {
    c_api::SetLastError(SENSCORD_STATUS_TRACE(status));
    return -1;
  }
  memory_area->memory = c_api::ToHandle(area.memory);
  memory_area->offset = area.offset;
  memory_area->size = area.size;
  return 0;
}

/**
 * @brief Gets memory pool information.
 * @param[in] exec_env  WASM execution environment.
 * @param[in] stream    Stream handle.
 * @param[out] info     Memory pool information.
 * @return 0 for success, -1 for failure.
 */
int32_t senscord_context_get_memory_pool_info(
    wasm_exec_env_t exec_env,
    senscord_stream_t stream,
    struct senscord_wasm_memory_pool_info_t* info) {
  SENSCORD_C_API_ARGUMENT_CHECK(info == NULL);
  info->num = 0;
  info->size = 0;
  wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(exec_env);
  senscord::WasmMemoryPool* memory_pool =
      senscord_context_get_memory_pool(module_inst, stream);
  if (memory_pool != NULL) {
    info->num = memory_pool->GetNum();
    info->size = memory_pool->GetSize();
  }
  return 0;
}

/* Context memory information (for senscord_context_memory_t) */
struct ContextMemoryInfo {
  wasm_module_inst_t module_inst;
  SensCordContext* context;
  wasm_addr_t wasm_addr;
};

/**
 * @brief Allocates memory and copy data.
 * @param[in] exec_env  WASM execution environment.
 * @param[in] data      Data to copy.
 * @param[in] size      Size of data.
 * @param[out] memory   Context memory handle.
 * @return 0 for success, -1 for failure.
 */
int32_t senscord_context_duplicate_memory(
    wasm_exec_env_t exec_env,
    const void* data,
    uint32_t size,
    senscord_context_memory_t* memory) {
  SENSCORD_C_API_ARGUMENT_CHECK(memory == NULL);
  wasm_module_inst_t module_inst = wasm_runtime_get_module_inst(exec_env);
  SensCordContext* context = senscord_context_get_instance(module_inst);
  if (context == NULL) {
    c_api::SetLastError(SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseInvalidOperation,
        "context is not available."));
    return -1;
  }
  LockGuard _lock(context->mutex);
  wasm_addr_t wasm_addr = wasm_runtime_module_dup_data(
      module_inst, reinterpret_cast<const char*>(data), size);
  if (wasm_addr == 0) {
    c_api::SetLastError(SENSCORD_STATUS_FAIL(
        kBlockName, senscord::Status::kCauseResourceExhausted,
        "wasm_runtime_module_dup_data() failed."));
    return -1;
  }
  LOG_D("[context memory] alloc: %" PRIu32, wasm_addr);
  ContextMemoryInfo* memory_info = new ContextMemoryInfo;
  memory_info->module_inst = module_inst;
  memory_info->context = context;
  memory_info->wasm_addr = wasm_addr;
  context->context_memory.insert(memory_info->wasm_addr);
  *memory = c_api::ToHandle(memory_info);
  return 0;
}

/**
 * @brief Frees memory.
 * @param[in] memory  Context memory handle.
 */
void senscord_context_free_memory(
    senscord_context_memory_t memory) {
  ContextMemoryInfo* memory_info =
      c_api::ToPointer<ContextMemoryInfo*>(memory);
  if (memory_info != NULL) {
    SensCordContext* context = memory_info->context;
    LockGuard _lock(context->mutex);
    std::set<wasm_addr_t>::iterator itr =
        context->context_memory.find(memory_info->wasm_addr);
    if (itr != context->context_memory.end()) {
      wasm_addr_t wasm_addr = *itr;
      LOG_D("[context memory] free: %" PRIu32, wasm_addr);
      wasm_runtime_module_free(
          memory_info->module_inst, wasm_addr);
      context->context_memory.erase(itr);
    }
    delete memory_info;
  }
}

/**
 * @brief Gets the Wasm address.
 * @param[in] memory  Context memory handle.
 */
wasm_addr_t senscord_context_get_wasm_address(
    senscord_context_memory_t memory) {
  wasm_addr_t wasm_addr = 0;
  ContextMemoryInfo* memory_info =
      c_api::ToPointer<ContextMemoryInfo*>(memory);
  if (memory_info != NULL) {
    wasm_addr = memory_info->wasm_addr;
  }
  return wasm_addr;
}
