/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef APPLICATION_MULTI_SERVER_SRC_CLIENT_ADAPTER_H_
#define APPLICATION_MULTI_SERVER_SRC_CLIENT_ADAPTER_H_

#include <map>
#include <string>
#include "senscord/osal.h"
#include "senscord/senscord.h"
#include "senscord/connection.h"
#include "senscord/serialize.h"
#include "server_log.h"
#include "client_adapter_manager.h"
#include "stream_adapter.h"
#include "config_manager.h"
#include "publisher_adapter.h"

namespace senscord {
namespace server {

class PublisherAdapter;

/**
 * @brief The abstract adapter class for the client connection.
 */
class ClientAdapterBase : private util::Noncopyable {
 public:
  /**
   * @brief Constructor.
   * @param[in] (manager) The client adapter manager.
   * @param[in] (connection) The connection interface.
   */
  explicit ClientAdapterBase(
      ClientAdapterManager* manager, Connection* connection);

  /**
   * @brief Destructor.
   */
  virtual ~ClientAdapterBase();

  /**
   * @brief Start to receive the message to the client.
   * @return Status object.
   */
  Status Start();

  /**
   * @brief Stop to receive the message.
   * @return Status object.
   */
  Status Stop();

  /**
   * @brief Set the secondary client adapter.
   * @param[in] (stream_id) Identifier of server stream.
   * @param[in] (client) Secondary client adapter. (If NULL, reset)
   * @return true if it is set.
   */
  virtual bool SetSecondaryAdapter(
      uint64_t stream_id, ClientAdapterBase* client);

  /**
   * @brief Send the reply message to client.
   * @param[in] (request_msg) The request message.
   * @param[in] (resource) The address for resource.
   * @param[in] (reply_data) The reply message.
   * @return Status object.
   */
  template <typename T>
  Status SendReply(
      const Message& request_msg,
      void* resource,
      const T& reply_data) {
    Status status = SendMessage(resource, request_msg.header.request_id,
        kMessageTypeReply, request_msg.header.data_type, reply_data);
    return SENSCORD_STATUS_TRACE(status);
  }

  /**
   * @brief Send the message to client.
   * @param[in] (resource) The address for resource.
   * @param[in] (request_id) The request ID of message.
   * @param[in] (type) The type of message.
   * @param[in] (data_type) The data type of message.
   * @param[in] (msg_data) The data of message.
   * @return Status object.
   */
  template <typename T>
  Status SendMessage(
      void* resource,
      uint64_t request_id,
      MessageType type,
      MessageDataType data_type,
      const T& msg_data) {
    Message msg = {};
    msg.header.server_stream_id = reinterpret_cast<uint64_t>(resource);
    msg.header.request_id = request_id;
    msg.header.type = type;
    msg.header.data_type = data_type;
    msg.data = const_cast<T*>(&msg_data);

    Status status = SendMessageToClient(msg);
    return SENSCORD_STATUS_TRACE(status);
  }

  /**
   * @brief Send the message to client.
   * @param[in] (msg) The message to send.
   * @return Status object.
   **/
  virtual Status SendMessageToClient(const Message& msg);

  /**
   * @brief Release the message.
   * @param[in] (msg) The message to release.
   */
  void ReleaseMessage(const Message* msg) const;

 protected:
  /**
   * @brief The subroutine to receive new message.
   * @return Status object.
   */
  virtual Status RecvMessage() = 0;

  /**
   * @brief This function is called when monitoring is finished.
   */
  virtual void OnMonitoringFinished();

 private:
  /**
   * @brief Working thread for receiving.
   * @param[in] (arg) The instance of client adapter.
   * @return Always returns normal.
   */
  static osal::OSThreadResult ThreadProc(void* arg);

  /**
   * @brief The method of the client receiving thread.
   */
  void Monitoring();

 protected:
  // adapter manager.
  ClientAdapterManager* manager_;

  // connection interface.
  Connection* connection_;

 private:
  // the thread for receiving messages.
  osal::OSThread* thread_;
  bool end_flag_;
};

/**
 * @brief The receiving adapter class for the client connection.
 */
class ClientAdapter : public ClientAdapterBase {
 public:
  /**
   * @brief Constructor.
   * @param[in] (manager) The client adapter manager.
   * @param[in] (connection) The connection interface.
   * @param[in] (core) The SDK Core instance.
   * @param[in] (config_manager) The config manager.
   * @param[in] (connection_key) The connection key.
   */
  explicit ClientAdapter(
      ClientAdapterManager* manager, Connection* connection,
      Core* core, const ConfigManager& config_manager,
      const std::string& connection_key);

  /**
   * @brief Destructor.
   */
  ~ClientAdapter();

  /**
   * @brief Get raw data from Connection class.
   * @param[in] (channel) Channel object.
   * @param[out] (rawdata) Information of the raw data to send.
   * @return Status object.
   */
  Status GetChannelRawData(const Channel* channel,
                           ChannelRawDataInfo* rawdata) const;

  /**
   * @brief Set the secondary client adapter.
   * @param[in] (stream_id) Identifier of server stream.
   * @param[in] (client) Secondary client adapter. (If NULL, reset)
   * @return true if it is set.
   */
  bool SetSecondaryAdapter(uint64_t stream_id, ClientAdapterBase* client);

  /**
   * @brief Send the message to client.
   * @param[in] (msg) The message to send.
   * @return Status object.
   **/
  Status SendMessageToClient(const Message& msg);

 protected:
  /**
   * @brief The subroutine to receive new message.
   * @return Status object.
   */
  Status RecvMessage();

  /**
   * @brief This function is called when monitoring is finished.
   */
  void OnMonitoringFinished();

 private:
  /**
   * @brief The subroutine to receive new message.
   * @param[in] (msg) New received message.
   * @return Status object.
   */
  Status Acception(Message* msg);

  /**
   * @brief Get the version.
   * @param[in] (msg) The received message.
   * @return Status object.
   */
  Status GetVersion(const Message& msg);

  /**
   * @brief Get the stream list.
   * @param[in] (msg) The received message.
   * @return Status object.
   */
  Status GetStreamList(const Message& msg);

#ifdef SENSCORD_SERVER_SETTING
  /**
   * @brief Get the config.
   * @param[in] (msg) The received message.
   * @return Status object.
   */
  Status GetConfig(const Message& msg);
#endif  // SENSCORD_SERVER_SETTING

  /**
   * @brief Open new stream.
   * @param[in] (msg) The received message.
   * @return Status object.
   */
  Status OpenStream(const Message& msg);

  /**
   * @brief Close the stream.
   * @param[in] (msg) The received message.
   * @param[in] (adapter) The removing stream adapter.
   * @return Status object.
   */
  Status CloseStream(const Message& msg, ResourceAdapter* adapter);

  /**
   * @brief Create the new stream adapter. Need to lock.
   * @param[in] (stream) The pointer of new stream.
   * @return Status object.
   */
  Status CreateStreamAdapter(Stream* stream);

  /**
   * @brief Create the new publisher adapter. Need to lock.
   * @param[in] (request) The request message.
   * @param[out] (adapter) The pointer of new publisher adapter.
   * @return Status object.
   */
  Status CreatePublisherAdapter(
      const MessageDataOpenPublisherRequest& request,
      PublisherAdapter** adapter);

  /**
   * @brief Delete the stream adapter. Need to lock.
   * @param[in] (adapter) The stream adapter.
   * @return Status object.
   */
  Status DeleteResourceAdapter(ResourceAdapter* adapter);

  /**
   * @brief Get the stream adapter pointer. Need to lock.
   * @param[in] (stream_id) Identifier of server stream.
   * @return The pointer of stream adapter. NULL means failed.
   */
  ResourceAdapter* GetAdapter(uint64_t stream_id) const;

  /**
   * @brief Open new publisher.
   * @param[in] (msg) The received message.
   * @return Status object.
   */
  Status OpenPublisher(const Message& msg);

  /**
   * @brief Close the publisher.
   * @param[in] (msg) The received message.
   * @param[in] (adapter) The removing publisher adapter.
   * @return Status object.
   */
  Status ClosePublisher(const Message& msg, ResourceAdapter* adapter);

  /**
   * @brief Close all resource.
   */
  void CloseAllResource();

 private:
  // SDK Core.
  Core* core_;

  // Config manager.
  const ConfigManager& config_manager_;

  // the list of opened resources.
  typedef std::map<uint64_t, ResourceAdapter*> ResourceAdapterList;
  ResourceAdapterList resource_adapters_;
  osal::OSMutex* resource_adapters_mutex_;

  ClientAdapterBase* secondary_adapter_;
  osal::OSMutex* secondary_adapter_mutex_;

  // connection key
  std::string connection_key_;
};

}  // namespace server
}   // namespace senscord

#endif  // APPLICATION_MULTI_SERVER_SRC_CLIENT_ADAPTER_H_
