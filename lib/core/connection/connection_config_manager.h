/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_CONNECTION_CONNECTION_CONFIG_MANAGER_H_
#define LIB_CORE_CONNECTION_CONNECTION_CONFIG_MANAGER_H_

#include <string>
#include <vector>
#include <map>
#include "senscord/noncopyable.h"
#include "senscord/status.h"
#include "senscord/osal.h"

namespace senscord {

/**
 * @brief Config manager for connection.
 */
class ConnectionConfigManager : private util::Noncopyable {
 public:
  /**
   * @brief Constructor
   */
  ConnectionConfigManager();

  /**
   * @brief Destructor
   */
  ~ConnectionConfigManager();

  /**
   * @brief Read connections config.
   * @param[in] (filename) Config file path.
   * @return Status object.
   */
  Status ReadConfig(const std::string& filename);

  /**
   * @brief Get the connection library name.
   * @param[in] (key) Connection key.
   * @param[out] (library_name) Library name.
   * @return Status object.
   */
  Status GetLibraryName(
      const std::string& key, std::string* library_name) const;

  /**
   * @brief Get the connection arguments.
   * @param[in] (key) Connection key.
   * @param[out] (arguments) arguments.
   * @return Status object.
   */
  Status GetArguments(
      const std::string& key,
      std::map<std::string, std::string>* arguments) const;

 private:
  /**
   * @brief Takes a parsed node, or Parses a new node.
   * @param[out] (node) current node.
   * @return True if obtained, false otherwise.
   */
  bool TakeOrParseNode(osal::OSXmlNodeType* node);

  /**
   * @brief Parse connections element of config.
   * @return Status object.
   */
  Status ParseConnections();

  /**
   * @brief Parse connection element of config.
   * @return Status object.
   */
  Status ParseConnection();

  /**
   * @brief Parse attribute of config.
   * @param[in] (attr_name) attribute name.
   * @param[out] (value) type attribute value.
   * @return Status object.
   */
  Status ParseAttribute(const char* attr_name, std::string* value);

  /**
   * @brief Parse arguments element of config.
   * @param[out] (arguments) arguments.
   * @return Status object.
   */
  Status ParseArguments(std::map<std::string, std::string>* arguments);

  /**
   * @brief Parse argument element of config.
   * @param[out] (arguments) arguments.
   * @return Status object.
   */
  Status ParseArgument(std::map<std::string, std::string>* arguments);

 private:
  bool is_read_;

  struct ConnectionInformation {
    std::string library_name;
    std::map<std::string, std::string> arguments;
  };

  // config informations <key, information>
  typedef std::map<std::string, ConnectionInformation> ConnectionList;
  ConnectionList connection_list_;

  // XML Parser Class.
  osal::OSXmlParser parser_;
  osal::OSXmlNodeType parsed_node_;
};

}  // namespace senscord

#endif  // LIB_CORE_CONNECTION_CONNECTION_CONFIG_MANAGER_H_
