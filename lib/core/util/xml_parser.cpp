/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "util/xml_parser.h"

#include <inttypes.h>

#include <sstream>

namespace senscord {
namespace util {

/**
 * @brief Get the element name.
 * @return Element name.
 */
std::string XmlElement::GetName() const {
  std::string name;
  if (!xpath.empty()) {
    name = *xpath.rbegin();
  }
  return name;
}

/**
 * @brief Get the element depth.
 *
 * depth=0 (xpath=/sdk)
 * depth=1 (xpath=/sdk/streams)
 * depth=2 (xpath=/sdk/streams/stream)
 *
 * @return Element depth. (-1: error)
 */
int32_t XmlElement::GetDepth() const {
  return static_cast<int32_t>(xpath.size()) - 1;
}

/**
 * @brief Get the XPath string.
 * @return XPath string.
 */
std::string XmlElement::GetXPath() const {
  std::ostringstream buf;
  for (std::vector<std::string>::const_iterator itr = xpath.begin(),
      end = xpath.end(); itr != end; ++itr) {
    buf << '/' << *itr;
  }
  return buf.str();
}

struct XmlParser::Impl {
  osal::OSXmlParser parser;
  XmlElement current_element;
  XmlElement* next_element;
};

/**
 * @brief Constructor.
 */
XmlParser::XmlParser() : pimpl_(new Impl()) {
  pimpl_->next_element = NULL;
}

/**
 * @brief Destructor.
 */
XmlParser::~XmlParser() {
  delete pimpl_;
}

/**
 * @brief Open XML file.
 * @param[in] (file_path) File path to open.
 * @return Status object.
 */
Status XmlParser::Open(const std::string& file_path) {
  if (pimpl_->parser.Open(file_path) != 0) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseInvalidArgument,
        "Failed to open: file_path=%s", file_path.c_str());
  }
  return Status::OK();
}

/**
 * @brief Close XML file.
 */
void XmlParser::Close() {
  pimpl_->parser.Close();
}

/**
 * @brief Get the next element.
 * @param[out] (element) Next element.
 * @return True if the element is obtained.
 */
bool XmlParser::NextElement(XmlElement* element) {
  if (pimpl_->next_element != NULL) {
    *element = *pimpl_->next_element;
    pimpl_->next_element = NULL;
    return true;
  }
  osal::OSXmlNodeType type = osal::kOSXmlUnsupportedNode;
  while (pimpl_->parser.Parse(&type) == 0) {
    if (type == osal::kOSXmlElementNode) {
      std::string name;
      pimpl_->parser.GetElement(&name);
      uint32_t depth = 0;
      pimpl_->parser.GetDepth(&depth);
      pimpl_->current_element.xpath.resize(depth);
      pimpl_->current_element.xpath.push_back(name);
      *element = pimpl_->current_element;
#if 0
      osal::OSPrintf(
          "[%" PRId32 "] %s (name=%s)\n",
          element->GetDepth(), element->GetXPath().c_str(),
          element->GetName().c_str());
#endif
      return true;
    }
  }
  return false;
}

/**
 * @brief Undo the `NextElement()`.
 */
void XmlParser::UndoElement() {
  pimpl_->next_element = &pimpl_->current_element;
}

/**
 * @brief Get the attribute.
 * @param[in] (name) Attribute name.
 * @param[out] (value) Attribute value.
 * @return Status object.
 */
Status XmlParser::GetAttribute(const std::string& name, std::string* value) {
  int32_t ret = pimpl_->parser.GetAttribute(name, value);
  if (ret != 0) {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseNotFound,
        "Failed to get attribute `%s` (%s)",
        name.c_str(), pimpl_->current_element.GetXPath().c_str());
  }
  return Status::OK();
}

/**
 * @brief Get the attribute string.
 * @param[in] (name) Attribute name.
 * @param[in] (default_value) Default value. (default: empty string)
 * @return Attribute value.
 */
std::string XmlParser::GetAttributeString(
    const std::string& name,
    const std::string& default_value) {
  std::string value = default_value;
  pimpl_->parser.GetAttribute(name, &value);
  return value;
}

}  // namespace util
}  // namespace senscord
