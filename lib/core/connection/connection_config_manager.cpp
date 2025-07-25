/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "connection/connection_config_manager.h"
#include <string>
#include <vector>
#include <utility>  // make_pair
#include "core/internal_types.h"

namespace {

// connection config element/attribute
const char* kElementConnections = "connections";
const char* kElementConnection  = "connection";
const char* kElementArguments   = "arguments";
const char* kElementArgument    = "argument";
const char* kAttributeKey       = "key";
const char* kAttributeLibrary   = "library";
const char* kAttributeName      = "name";
const char* kAttributeValue     = "value";

}  // namespace

namespace senscord {

/**
 * @brief Constructor
 */
ConnectionConfigManager::ConnectionConfigManager()
    : is_read_(false), parsed_node_(osal::kOSXmlUnsupportedNode) {
}

/**
 * @brief Destructor
 */
ConnectionConfigManager::~ConnectionConfigManager() {}

/**
 * @brief Read connections config.
 * @param[in] (filename) Config file path.
 * @return Status object.
 */
Status ConnectionConfigManager::ReadConfig(const std::string& filename) {
  if (is_read_) {
    return Status::OK();
  }

  // open config file.
  if (parser_.Open(filename) != 0) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument,
        "open error(%s)", filename.c_str());
  }

  Status status;
  osal::OSXmlNodeType node_type = osal::kOSXmlUnsupportedNode;

  while (TakeOrParseNode(&node_type)) {
    if (node_type == osal::kOSXmlElementNode) {
      std::string element;
      parser_.GetElement(&element);
      if (element == kElementConnections) {
        // connections tag
        status = ParseConnections();
        if (!status.ok()) {
          SENSCORD_STATUS_TRACE(status);
          break;
        }
      }
    }
  }

  // close
  parser_.Close();
  if (status.ok()) {
    is_read_ = true;
  } else {
    // clear
    connection_list_.clear();
  }
  return status;
}

/**
 * @brief Get the connection library name.
 * @param[in] (key) Connection key.
 * @param[out] (library_name) Library name.
 * @return Status object.
 */
Status ConnectionConfigManager::GetLibraryName(
    const std::string& key, std::string* library_name) const {
  if (library_name == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid argument");
  }
  ConnectionList::const_iterator itr = connection_list_.find(key);
  if (itr == connection_list_.end()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotFound, "unknown key: %s", key.c_str());
  }
  *library_name = itr->second.library_name;
  return Status::OK();
}

/**
 * @brief Get the connection arguments.
 * @param[in] (key) Connection key.
 * @param[out] (arguments) arguments.
 * @return Status object.
 */
Status ConnectionConfigManager::GetArguments(
    const std::string& key,
    std::map<std::string, std::string>* arguments) const {
  if (arguments == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid argument");
  }
  ConnectionList::const_iterator itr = connection_list_.find(key);
  if (itr == connection_list_.end()) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseNotFound, "unknown key: %s", key.c_str());
  }
  *arguments = itr->second.arguments;
  return Status::OK();
}

/**
 * @brief Takes a parsed node, or Parses a new node.
 * @param[out] (node) current node.
 * @return True if obtained, false otherwise.
 */
bool ConnectionConfigManager::TakeOrParseNode(osal::OSXmlNodeType* node) {
  if (parsed_node_ != osal::kOSXmlUnsupportedNode) {
    *node = parsed_node_;
    parsed_node_ = osal::kOSXmlUnsupportedNode;
    return true;
  }
  while (parser_.Parse(node) == 0) {
    if (*node != osal::kOSXmlUnsupportedNode) {
      return true;
    }
  }
  return false;
}

/**
 * @brief Parse connections element of config.
 * @return Status object.
 */
Status ConnectionConfigManager::ParseConnections() {
  Status status;
  osal::OSXmlNodeType node_type = osal::kOSXmlUnsupportedNode;
  while (TakeOrParseNode(&node_type)) {
    if (node_type == osal::kOSXmlElementNode) {
      std::string element;
      parser_.GetElement(&element);
      if (element == kElementConnection) {
        // connection tag
        status = ParseConnection();
        if (!status.ok()) {
          SENSCORD_STATUS_TRACE(status);
          break;
        }
      } else {
        SENSCORD_LOG_WARNING("unknown \"%s\" element, ignored",
            element.c_str());
      }
    } else if (node_type == osal::kOSXmlElementEnd) {
      std::string element;
      parser_.GetElement(&element);
      if (element == kElementConnections) {
        break;
      } else {
        SENSCORD_LOG_WARNING("unknown \"/%s\" element, ignored",
            element.c_str());
      }
    }
  }
  return status;
}

/**
 * @brief Parse connection element of config.
 * @return Status object.
 */
Status ConnectionConfigManager::ParseConnection() {
  Status status;

  std::string key;
  status = ParseAttribute(kAttributeKey, &key);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  ConnectionInformation info = {};
  status = ParseAttribute(kAttributeLibrary, &info.library_name);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }

  // child node of <connection>
  osal::OSXmlNodeType node_type = osal::kOSXmlUnsupportedNode;
  while (TakeOrParseNode(&node_type)) {
    if (node_type == osal::kOSXmlElementNode) {
      std::string element;
      parser_.GetElement(&element);
      if (element == kElementArguments) {
        status = ParseArguments(&info.arguments);
        if (!status.ok()) {
          SENSCORD_STATUS_TRACE(status);
          break;
        }
      } else if (element == kElementConnection) {
        parsed_node_ = node_type;
        break;
      }
    } else if (node_type == osal::kOSXmlElementEnd) {
      std::string element;
      parser_.GetElement(&element);
      if (element == kElementConnection) {
        break;
      }
      if (element == kElementConnections) {
        parsed_node_ = node_type;
        break;
      }
    }
  }

  connection_list_.insert(std::make_pair(key, info));
  return status;
}

/**
 * @brief Parse attribute of config.
 * @param[in] (attr_name) attribute name.
 * @param[out] (value) type attribute value.
 * @return Status object.
 */
Status ConnectionConfigManager::ParseAttribute(
    const char* attr_name, std::string* value) {
  if ((parser_.GetAttribute(attr_name, value)) != 0) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore, Status::kCauseAborted,
        "parse attribute \"%s\" failed", attr_name);
  }
  return Status::OK();
}

/**
 * @brief Parse arguments element of config.
 * @param[out] (arguments) arguments.
 * @return Status object.
 */
Status ConnectionConfigManager::ParseArguments(
    std::map<std::string, std::string>* arguments) {
  Status status;
  osal::OSXmlNodeType node_type = osal::kOSXmlUnsupportedNode;
  while (TakeOrParseNode(&node_type)) {
    if (node_type == osal::kOSXmlElementNode) {
      std::string element;
      parser_.GetElement(&element);
      if (element == kElementArgument) {
        status = ParseArgument(arguments);
        if (!status.ok()) {
          SENSCORD_STATUS_TRACE(status);
          break;
        }
      }
    } else if (node_type == osal::kOSXmlElementEnd) {
      std::string element;
      parser_.GetElement(&element);
      if (element == kElementArguments) {
        break;
      }
    }
  }
  return status;
}

/**
 * @brief Parse argument element of config.
 * @param[out] (arguments) arguments.
 * @return Status object.
 */
Status ConnectionConfigManager::ParseArgument(
    std::map<std::string, std::string>* arguments) {
  Status status;
  std::string name;
  status = ParseAttribute(kAttributeName, &name);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  std::string value;
  status = ParseAttribute(kAttributeValue, &value);
  if (!status.ok()) {
    return SENSCORD_STATUS_TRACE(status);
  }
  (*arguments)[name] = value;
  return Status::OK();
}

}    // namespace senscord
