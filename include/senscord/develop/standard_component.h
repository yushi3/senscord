/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_DEVELOP_STANDARD_COMPONENT_H_
#define SENSCORD_DEVELOP_STANDARD_COMPONENT_H_

#include <string>
#include <vector>

#include "senscord/config.h"
#include "senscord/noncopyable.h"
#include "senscord/develop/component.h"
#include "senscord/develop/stream_source.h"
#include "senscord/develop/stream_source_factory.h"
#include "senscord/develop/stream_source_utility.h"

namespace senscord {

/**
 * @brief The standard component.
 */
class StandardComponent : public Component {
 public:
  /**
   * @brief Initialize this component, called at once.
   * @param[in] (core) Core instance.
   * @param[in] (port_manager) Port manager for this component.
   * @param[in] (args) Arguments of component starting.
   * @return Status object.
   */
  virtual Status InitComponent(
    Core* core,
    ComponentPortManager* port_manager,
    const ComponentArgument& args);

  /**
   * @brief Exit this component, called at all ports closed.
   * @return Status object.
   */
  virtual Status ExitComponent();

  /**
   * @brief Open the port.
   * @param[in] (port_type) Port type to open.
   * @param[in] (port_id) Port ID to open.
   * @param[in] (args) Arguments of port starting.
   * @return Status object.
   */
  virtual Status OpenPort(
    const std::string& port_type,
    int32_t port_id,
    const ComponentPortArgument& args);

  /**
   * @brief Close the port.
   * @param[in] (port_type) Port type to close.
   * @param[in] (port_id) Port ID to close.
   * @return Status object.
   */
  virtual Status ClosePort(const std::string& port_type, int32_t port_id);

  /**
   * @brief Start the port.
   * @param[in] (port_type) Port type to start.
   * @param[in] (port_id) Port ID to start.
   * @return Status object.
   */
  virtual Status StartPort(const std::string& port_type, int32_t port_id);

  /**
   * @brief Stop the port.
   * @param[in] (port_type) Port type to stop.
   * @param[in] (port_id) Port ID to stop.
   * @return Status object.
   */
  virtual Status StopPort(const std::string& port_type, int32_t port_id);

  /**
   * @brief Release the frame pushed from the port.
   * @param[in] (port_type) Port type to release frame.
   * @param[in] (port_id) Port ID to release frame.
   * @param[in] (frameinfo) Informations to release frame.
   * @param[in] (referenced_channel_ids) List of referenced channel IDs.
   *                                     (NULL is the same as empty)
   * @return Status object.
   */
  virtual Status ReleasePortFrame(
    const std::string& port_type,
    int32_t port_id,
    const FrameInfo& frameinfo,
    const std::vector<uint32_t>* referenced_channel_ids);

  /**
   * @brief Constructor
   * @param[in] (factory) Created stream source fatcory.
   */
  explicit StandardComponent(StreamSourceFactory* factory);

  /**
   * @brief Destructor
   */
  ~StandardComponent();

 private:
  /**
   * @brief Search and get the stream source adapter.
   * @param[in] (type) Port type.
   * @param[in] (id) Port ID.
   * @return The stream source adapter. NULL means not found.
   */
  StreamSourceUtility* GetAdapter(const std::string& type, int32_t id) const;

  // factory of stream sources
  StreamSourceFactory* factory_;

  // adapters
  typedef std::vector<StreamSourceUtility*> StreamSourceAdapterList;
  StreamSourceAdapterList adapters_;
};

}   // namespace senscord

/**
 * @def Macro for the new component registration.
 */
#define SENSCORD_REGISTER_COMPONENT(FACTORY_CLASS_NAME) \
  extern "C" void* CreateComponent() {    \
    return new senscord::StandardComponent(new FACTORY_CLASS_NAME());      \
  }     \
  extern "C" void DestroyComponent(void* component) {   \
    delete reinterpret_cast<senscord::StandardComponent*>(component);    \
  }

#endif    // SENSCORD_DEVELOP_STANDARD_COMPONENT_H_
