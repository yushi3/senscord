/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "senscord/develop/standard_component.h"
#include <string>
#include <map>
#include <vector>
#include <utility>
#include "senscord/osal.h"
#include "senscord/status.h"
#include "logger/logger.h"
#include "component/stream_source_adapter.h"

namespace senscord {

/**
 * @brief Initialize this component, called at once.
 * @param[in] (core) Core instance.
 * @param[in] (port_manager) Port manager for this component.
 * @param[in] (args) Arguments of component starting.
 * @return Status object.
 */
Status StandardComponent::InitComponent(
    Core* core,
    ComponentPortManager* port_manager, const ComponentArgument& args) {
  if (factory_ == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "no factory");
  }

  // get supported ports
  StreamSourceFactory::SourceTypeList list;
  factory_->GetSupportedList(args, &list);
  if (list.size() == 0) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "no supported sources");
  }

  {
    // create adapters
    StreamSourceFactory::SourceTypeList::const_iterator itr = list.begin();
    StreamSourceFactory::SourceTypeList::const_iterator end = list.end();

    SENSCORD_LOG_DEBUG("[%s] ports:", args.instance_name.c_str());
    for (; itr != end; ++itr) {
      SENSCORD_LOG_DEBUG(" - %s.%" PRId32, itr->first.c_str(), itr->second);

      // create port
      ComponentPort* port = NULL;
      Status status = port_manager->CreatePort(itr->first, itr->second, &port);
      if (status.ok()) {
        // create source adapter
        StreamSourceAdapter* adapter = new StreamSourceAdapter(
            core, port, args);
        adapters_.push_back(adapter);
      } else {
        SENSCORD_STATUS_TRACE(status);
        ExitComponent();
        return status;
      }
    }
  }
#ifdef SENSCORD_LOG_ENABLED
  if (SENSCORD_LOG_SEVERITY >= util::Logger::kLogDebug) {
    // print allocators
    SENSCORD_LOG_DEBUG("[%s] allocators:", args.instance_name.c_str());

    std::map<std::string, senscord::MemoryAllocator*>::const_iterator itr;
    for (itr = args.allocators.begin();
         itr != args.allocators.end(); ++itr) {
      SENSCORD_LOG_DEBUG(
          " - name=\"%s\", key=\"%s\", type=\"%s\"",
          itr->first.c_str(), itr->second->GetKey().c_str(),
          itr->second->GetType().c_str());
    }
  }
#endif  // SENSCORD_LOG_ENABLED
  return Status::OK();
}

/**
 * @brief Exit this component, called at all ports closed.
 * @return Status object.
 */
Status StandardComponent::ExitComponent() {
  while (!adapters_.empty()) {
    StreamSourceAdapterList::iterator itr = adapters_.begin();
    StreamSourceAdapter* adapter = static_cast<StreamSourceAdapter*>(*itr);
    adapters_.erase(itr);

    factory_->ReleaseSource(adapter->GetSource());
    adapter->ResetSourceInformation();
    delete adapter;
  }
  return Status::OK();
}

/**
 * @brief Open the port.
 * @param[in] (port_type) Port type to open.
 * @param[in] (port_id) Port ID to open.
 * @param[in] (args) Arguments of port starting.
 * @return Status object.
 */
Status StandardComponent::OpenPort(
    const std::string& port_type, int32_t port_id,
    const ComponentPortArgument& args) {
  StreamSourceAdapter* adapter = static_cast<StreamSourceAdapter*>(
      GetAdapter(port_type, port_id));
  if (adapter == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "unknown port id");
  }

  Status status;
  StreamSource* source = NULL;

  // create source
  status = factory_->CreateSource(
      std::make_pair(port_type, port_id), &source);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // set source and name
  adapter->SetSource(source);

  // open source
  status = adapter->Open(args);
  if (!status.ok()) {
    // release source
    factory_->ReleaseSource(source);
    adapter->ResetSourceInformation();
    return SENSCORD_STATUS_TRACE(status);
  }

  // register the mandatory properties.
  source->RegisterMandatoryProperties(adapter);
  return Status::OK();
}

/**
 * @brief Close the port.
 * @param[in] (port_type) Port type to close.
 * @param[in] (port_id) Port ID to close.
 * @return Status object.
 */
Status StandardComponent::ClosePort(
    const std::string& port_type, int32_t port_id) {
  StreamSourceAdapter* adapter = static_cast<StreamSourceAdapter*>(
      GetAdapter(port_type, port_id));
  if (adapter == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "unknown port id");
  }

  // if not stopped threading, try to force stop.
  adapter->StopThreadingApply();

  // close source
  Status status = adapter->Close();
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // release source
  factory_->ReleaseSource(adapter->GetSource());
  adapter->ResetSourceInformation();
  return Status::OK();
}

/**
 * @brief Start the port.
 * @param[in] (port_type) Port type to start.
 * @param[in] (port_id) Port ID to start.
 * @return Status object.
 */
Status StandardComponent::StartPort(
    const std::string& port_type, int32_t port_id) {
  StreamSourceAdapter* adapter = static_cast<StreamSourceAdapter*>(
      GetAdapter(port_type, port_id));
  if (adapter == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "unknown port id");
  }

  // start source
  Status status = adapter->Start();
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // start threading
  status = adapter->StartThreading();
  if (!status.ok()) {
    SENSCORD_STATUS_TRACE(status);

    Status stopstatus = adapter->Stop();
    if (!stopstatus.ok()) {
      SENSCORD_STATUS_TRACE(stopstatus);
      SENSCORD_LOG_ERROR("%s", stopstatus.ToString().c_str());
    }
  }
  return status;
}

/**
 * @brief Stop the port.
 * @param[in] (port_type) Port type to stop.
 * @param[in] (port_id) Port ID to stop.
 * @return Status object.
 */
Status StandardComponent::StopPort(
    const std::string& port_type, int32_t port_id) {
  StreamSourceAdapter* adapter = static_cast<StreamSourceAdapter*>(
      GetAdapter(port_type, port_id));
  if (adapter == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "unknown port id");
  }

  // ready to stop theading
  adapter->StopThreadingNotify();

  // stop source
  Status status = adapter->Stop();

  // wait to stop threading
  Status threadstatus = adapter->StopThreadingApply();
  if (!threadstatus.ok()) {
    SENSCORD_STATUS_TRACE(threadstatus);
    SENSCORD_LOG_WARNING("%s", threadstatus.ToString().c_str());
  }

  // check the source stopped
  if (!status.ok()) {
    SENSCORD_STATUS_TRACE(status);

    // restart threading if failed to stop
    threadstatus = adapter->StartThreading();
    if (!threadstatus.ok()) {
      SENSCORD_STATUS_TRACE(threadstatus);
      SENSCORD_LOG_ERROR("%s", threadstatus.ToString().c_str());
    }
  }
  return status;
}

/**
 * @brief Release the frame pushed from the port.
 * @param[in] (port_type) Port type to release frame.
 * @param[in] (port_id) Port ID to release frame.
 * @param[in] (frameinfo) Informations to release frame.
 * @param[in] (referenced_channel_ids) List of referenced channel IDs.
 *                                     (NULL is the same as empty)
 * @return Status object.
 */
Status StandardComponent::ReleasePortFrame(
    const std::string& port_type, int32_t port_id,
    const FrameInfo& frameinfo,
    const std::vector<uint32_t>* referenced_channel_ids) {
  StreamSourceAdapter* adapter = static_cast<StreamSourceAdapter*>(
      GetAdapter(port_type, port_id));
  if (adapter == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "unknown port id");
  }
  Status status;
  if ((referenced_channel_ids != NULL) && (!referenced_channel_ids->empty())) {
    status = adapter->ReleaseFrame(frameinfo, referenced_channel_ids);
  } else {
    status = adapter->ReleaseFrame(frameinfo, NULL);
  }
  return SENSCORD_STATUS_TRACE(status);
}

/**
 * @brief Search and get the stream source adapter.
 * @param[in] (type) Port type.
 * @param[in] (id) Port ID.
 * @return The stream source adapter. NULL means not found.
 */
StreamSourceUtility* StandardComponent::GetAdapter(
    const std::string& type, int32_t id) const {
  StreamSourceAdapterList::const_iterator itr = adapters_.begin();
  StreamSourceAdapterList::const_iterator end = adapters_.end();
  for (; itr != end; ++itr) {
    StreamSourceAdapter* adapter = static_cast<StreamSourceAdapter*>(*itr);
    if ((adapter->GetType() == type) && (adapter->GetId() == id)) {
      return *itr;
    }
  }
  return NULL;
}

/**
 * @brief Constructor
 * @param[in] (factory) Created stream source fatcory.
 */
StandardComponent::StandardComponent(StreamSourceFactory* factory)
    : factory_(factory) {}

/**
 * @brief Destructor
 */
StandardComponent::~StandardComponent() {
  ExitComponent();
  delete factory_;
  factory_ = NULL;
}

}   // namespace senscord
