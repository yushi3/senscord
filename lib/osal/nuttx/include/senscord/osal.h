/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_OSAL_H_
#define SENSCORD_OSAL_H_

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <vector>
#include <string>

#include "senscord/osal_error.h"
#include "senscord/osal_inttypes.h"

namespace senscord {
namespace osal {

/* Standard IO */
/**
 * @brief Print formatted output to the standard output stream.
 * @param[in] format  Format string.
 * @param[in] ...     Optional arguments.
 * @return On success, the total number of written characters is returned.
 *         Negative value is fail. (error code)
 */
int32_t OSPrintf(const char* format, ...);

/**
 * @brief Print formatted output specified by va_list to the standard output
 *        stream.
 * @param[in] format  Format string.
 * @param[in] args    List of arguments.
 * @return On success, the total number of written characters is returned.
 *         Negative value is fail. (error code)
 */
int32_t OSVprintf(const char* format, va_list args);

/**
 * @brief Outputs the converted character string to the buffer.
 *
 * For the following parameters, an error occurs.
 *   buffer == NULL : kErrorInvalidArgument
 *   format == NULL : kErrorInvalidArgument
 *   size   == 0    : kErrorInvalidArgument
 *
 * If the length of the converted string is greater than the buffer size,
 * the string is truncated and the return value is the total number of
 * written characters.
 *
 * @param[out] buffer  Pointer to the buffer that stores the converted
 *                     character string.
 * @param[in]  size    Size of buffer.
 * @param[in]  format  Format string.
 * @param[in]  args    List of arguments.
 * @return On success, the total number of written characters is returned.
 *         Negative value is fail. (error code)
 */
int32_t OSVsnprintf(char* buffer, size_t size,
                    const char* format, va_list args);

/* String */
/**
 * @brief Radix auto.
 */
const uint8_t kOSRadixAuto = 0;
/**
 * @brief Radix max.
 * Character is converted to number. ('A' represents 10, 'Z' represents 35)
 */
const uint8_t kOSRadixMax  = 36;
/**
 * @brief Radix min.
 * It means binary number.
 */
const uint8_t kOSRadixMin  = 2;

/**
 * @brief Convert a string to a 64-bit signed integer.
 * @param[in]  target_string  String to convert.
 * @param[out] end_string     Pointer to the variable that receives the stop
 *                            position.
 * @param[in]  radix          Radix to use.
 * @param[out] convert_value  Pointer to the variable that receives the
 *                            converted number.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSStrtoll(const char* target_string,
                  char** end_string, uint8_t radix, int64_t* convert_value);

/**
 * @brief Convert a string to a 64-bit unsigned integer.
 * @param[in]  target_string  String to convert.
 * @param[out] end_string     Pointer to the variable that receives the stop
 *                            position.
 * @param[in]  radix          Radix to use.
 * @param[out] convert_value  Pointer to the variable that receives the
 *                            converted number.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSStrtoull(const char* target_string,
                   char** end_string, uint8_t radix, uint64_t* convert_value);

/* Thread */
/**
 * @brief Thread object.
 */
typedef struct OSThread OSThread;

/**
 * @brief Thread end result.
 */
typedef uintptr_t OSThreadResult;

/**
 * @brief Thread function pointer.
 */
typedef OSThreadResult (*OSThreadFunc)(void* argument);

/**
 * @brief Detached state of thread.
 */
enum OSThreadDetachState {
  kOSThreadJoinable = 0,  /**< Joinable state. (default) */
  kOSThreadDetached,      /**< Detached state. */
};

/**
 * @brief Priority of thread.
 */
enum OSThreadPriority {
  kOSThreadPriorityDefault = 0,  /**< Default priority. (Same as Normal) */
  kOSThreadPriorityIdle,         /**< Lower priority than Lowest. */
  kOSThreadPriorityLowest,       /**< Lowest priority. */
  kOSThreadPriorityBelowNormal,  /**< Priority between Lowest and Normal. */
  kOSThreadPriorityNormal,       /**< Normal priority. */
  kOSThreadPriorityAboveNormal,  /**< Priority between Highest and Normal. */
  kOSThreadPriorityHighest,      /**< Highest priority. */
};

/**
 * @brief Thread attributes.
 */
struct OSThreadAttribute {
  OSThreadDetachState detach_state;  /**< Detached state of thread */
  OSThreadPriority    priority;      /**< Priority of thread */
};

/**
 * @brief Create a new thread.
 * @param[out] thread          Pointer to the variable that receives the
 *                             thread object.
 * @param[in]  thread_func     Functions to be executed in new thread.
 * @param[in]  thread_argument Argument to be passed to a new thread.
 * @param[in]  thread_attr     Attributes for a new thread.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSCreateThread(OSThread** thread,
                       OSThreadFunc thread_func,
                       void* thread_argument,
                       const OSThreadAttribute* thread_attr);

/**
 * @brief Detach a thread.
 * @param[in] thread  Thread object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSDetachThread(OSThread* thread);

/**
 * @brief Join with a terminated thread.
 * @param[in]  thread  Thread object.
 * @param[out] result  Pointer to the variable that receives the thread end
 *                     result. (optional)
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSJoinThread(OSThread* thread, OSThreadResult* result);

/**
 * @brief Get the current thread.
 * @param[out] thread  Pointer to the variable that receives the thread object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSGetCurrentThread(OSThread** thread);

/* Mutex exclusion */
/**
 * @brief Mutex(Mutual exclusion) object.
 */
typedef struct OSMutex OSMutex;

/**
 * @brief Create a mutex object.
 * @param[out] mutex  Pointer to the variable that receives the mutex object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSCreateMutex(OSMutex** mutex);

/**
 * @brief Destroy a mutex object.
 * @param[in] mutex  Mutex object to destroy.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSDestroyMutex(OSMutex* mutex);

/**
 * @brief Lock a mutex.
 * @param[in] mutex  Mutex object to lock.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSLockMutex(OSMutex* mutex);

/**
 * @brief Try to lock a mutex.
 * @param[in] mutex  Mutex object to lock.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSTryLockMutex(OSMutex* mutex);

/**
 * @brief Unlock a mutex.
 * @param[in] mutex  Mutex object to unlock.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSUnlockMutex(OSMutex* mutex);

/* Condition Variable */
/**
 * @brief Condition Variable object.
 */
typedef struct OSCond OSCond;

/**
 * @brief Create a condition variable.
 * @param[out] cond  Pointer to the variable that receives the condition
 *                   variable.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSCreateCond(OSCond** cond);

/**
 * @brief Destroy a condition variable.
 * @param[in] cond  Condition variable object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSDestroyCond(OSCond* cond);

/**
 * @brief Wait until notified.
 * @param[in] cond   Condition variable object.
 * @param[in] mutex  Mutex object that is currently locked by this thread.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSWaitCond(OSCond* cond, OSMutex* mutex);

/**
 * @brief Wait for timeout or until notified. (specify absolute time)
 * @param[in] cond   Condition variable object.
 * @param[in] mutex  Mutex object that is currently locked by this thread.
 * @param[in] nano_seconds  Timeout absolute time, in nanoseconds.
 * @return 0 is success. Negative value is fail. (error code)
 * @retval kErrorTimedOut When the low-order 1 byte is kErrorTimedOut,
 *                        it means timeout.
 */
int32_t OSTimedWaitCond(OSCond* cond,
                        OSMutex* mutex,
                        uint64_t nano_seconds);

/**
 * @brief Wait for timeout or until notified. (specify relative time)
 * @param[in] cond   Condition variable object.
 * @param[in] mutex  Mutex object that is currently locked by this thread.
 * @param[in] nano_seconds  Timeout relative time, in nanoseconds.
 * @return 0 is success. Negative value is fail. (error code)
 * @retval kErrorTimedOut When the low-order 1 byte is kErrorTimedOut,
 *                        it means timeout.
 */
int32_t OSRelativeTimedWaitCond(OSCond* cond,
                                OSMutex* mutex,
                                uint64_t nano_seconds);

/**
 * @brief Unblocks one of the threads waiting for the condition variable.
 * @param[in] cond  Condition variable object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSSignalCond(OSCond* cond);

/**
 * @brief Unblocks all threads waiting for the condition variable.
 * @param[in] cond  Condition variable object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSBroadcastCond(OSCond* cond);

/* Thread Sleep */
/**
 * @brief Sleep for the specified time.
 * @param[in] nano_seconds  Sleep time, in nanoseconds.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSSleep(uint64_t nano_seconds);

/* Memory */
/**
 * @brief Search character in block of memory.
 * @param[in] source  Pointer to the block of memory where the search is
 *                    performed.
 * @param[in] character  Character to be searched.
 * @param[in] length  Length of bytes to be analyzed.
 * @return A pointer to the first occurrence of value in the block of memory
 *         pointed by source.
 *         If the character is not found, the function returns a null pointer.
 */
const void* OSMemchr(const void* source, int32_t character, size_t length);
void* OSMemchr(void* source, int32_t character, size_t length);

/**
 * @brief Compare two blocks of memory.
 * @param[in] source1  Pointer to block of memory.
 * @param[in] source2  Pointer to block of memory.
 * @param[in] length   Length of bytes to compare.
 * @retval >0  source1 larger than source2.
 * @retval 0   source1 identical to source2.
 * @retval <0  source1 smaller than source2.
 */
int32_t OSMemcmp(const void* source1, const void* source2, size_t length);

/**
 * @brief Copy block of memory.
 * @param[out] dest      Pointer to the destination array where the content is
 *                       to be copied.
 * @param[in]  dest_size Size of the destination array.
 * @param[in]  source    Pointer to the source of data to be copied.
 * @param[in]  count     Number of bytes to copy.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSMemcpy(void* dest, size_t dest_size,
                 const void* source, size_t count);

/**
 * @brief Move block of memory.
 * @param[out] dest      Pointer to the destination array where the content is
 *                       to be copied.
 * @param[in]  dest_size Size of the destination array.
 * @param[in]  source    Pointer to the source of data to be copied.
 * @param[in]  count     Number of bytes to copy.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSMemmove(void* dest, size_t dest_size,
                  const void* source, size_t count);

/**
 * @brief Fill block of memory.
 * @param[out] buffer    Pointer to the block of memory to fill.
 * @param[in]  character Value to be set.
 * @param[in]  length    Number of bytes to be set.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSMemset(void* buffer, int32_t character, size_t length);

/**
 * @brief Allocate memory block.
 * @param[in] length  Length of the memory block, in bytes.
 * @return On success, it returns a pointer to the memory block.
 *         If it failed, it returns a null pointer.
 */
void* OSMalloc(size_t length);

/**
 * @brief Deallocate memory block.
 * @param[in] ptr Pointer to a memory block previously allocated with OSMalloc.
 * @return Always returns zero.
 */
int32_t OSFree(void* ptr);

/* Time*/
/**
 * @brief Get current time.
 * Time in nanoseconds since the epoch time(1970-1-1 00:00:00 UTC).
 * @param[out] nano_seconds  Pointer to the variable that receives the current
 *                           time. (in nanoseconds)
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSGetTime(uint64_t* nano_seconds);

/**
 * @brief Fast exclusive lock class.
 */
class OSExclusiveLock {
 public:
  OSExclusiveLock();
  ~OSExclusiveLock();

  /**
   * @brief Exclusive lock.
   */
  void Lock();
  /**
   * @brief Exclusive unlock.
   */
  void Unlock();

 private:
  OSExclusiveLock(const OSExclusiveLock&);  // = delete;
  OSExclusiveLock& operator =(const OSExclusiveLock&);  // = delete;

 private:
  void* lock_object_;
};

}  //  namespace osal
}  //  namespace senscord

#endif  // SENSCORD_OSAL_H_
