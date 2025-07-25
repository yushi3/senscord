/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_UTIL_XML_PARSER_H_
#define LIB_CORE_UTIL_XML_PARSER_H_

#include <stdint.h>

#include <string>
#include <vector>

#include "senscord/osal.h"
#include "senscord/status.h"

namespace senscord {
namespace util {

/**
 * @brief XML Element.
 */
struct XmlElement {
  std::vector<std::string> xpath;

  /**
   * @brief Get the element name.
   * @return Element name.
   */
  std::string GetName() const;

  /**
   * @brief Get the element depth.
   *
   * depth=0 (xpath=/sdk)
   * depth=1 (xpath=/sdk/streams)
   * depth=2 (xpath=/sdk/streams/stream)
   *
   * @return Element depth. (-1: error)
   */
  int32_t GetDepth() const;

  /**
   * @brief Get the XPath string.
   * @return XPath string.
   */
  std::string GetXPath() const;
};

/**
 * @brief XML Parser.
 */
class XmlParser {
 public:
  /**
   * @brief Constructor.
   */
  XmlParser();

  /**
   * @brief Destructor.
   */
  ~XmlParser();

  /**
   * @brief Open XML file.
   * @param[in] (file_path) File path to open.
   * @return Status object.
   */
  Status Open(const std::string& file_path);

  /**
   * @brief Close XML file.
   */
  void Close();

  /**
   * @brief Get the next element.
   * @param[out] (element) Next element.
   * @return True if the element is obtained.
   */
  bool NextElement(XmlElement* element);

  /**
   * @brief Undo the `NextElement()`.
   */
  void UndoElement();

  /**
   * @brief Get the attribute.
   * @param[in] (name) Attribute name.
   * @param[out] (value) Attribute value.
   * @return Status object.
   */
  Status GetAttribute(const std::string& name, std::string* value);

  /**
   * @brief Get the attribute string.
   * @param[in] (name) Attribute name.
   * @param[in] (default_value) Default value. (default: empty string)
   * @return Attribute value.
   */
  std::string GetAttributeString(
      const std::string& name,
      const std::string& default_value = std::string());

 private:
  struct Impl;
  Impl* pimpl_;
};

}  // namespace util
}  // namespace senscord

#endif  // LIB_CORE_UTIL_XML_PARSER_H_
