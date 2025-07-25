/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <libxml/xmlreader.h>
#include <string>

#include "senscord/osal.h"
#include "common/osal_error.h"
#include "linux/osal_xml_control.h"

namespace senscord {
namespace osal {

/**
 * @brief OSXmlParser constructor.
 */
OSXmlParser::OSXmlParser() {
  reader_ = NULL;
}

/**
 * @brief OSXmlParser destructor.
 */
OSXmlParser::~OSXmlParser() {
  Close();
}

/**
 * @brief Open a XML file.
 * Other files can not be opened until closed.
 * @param[in] file_name  Path of the file to open.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSXmlParser::Open(const std::string& file_name) {
  static const OSFunctionId kFuncId = kIdOSXmlParserOpen;
  if (reader_ != NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidOperation);
  }

  XmlControl::GetInstance()->GetControl();

  xmlTextReaderPtr* reader_temporary = reinterpret_cast<xmlTextReaderPtr*>(
      malloc(sizeof(xmlTextReaderPtr)));
  if (reader_temporary == NULL) {
    XmlControl::GetInstance()->ReleaseControl();
    return OSMakeErrorCode(kFuncId, kErrorOutOfMemory);
  }

  *reader_temporary = xmlReaderForFile(file_name.c_str(), NULL, 0);
  if (*reader_temporary != NULL) {
    reader_ = reinterpret_cast<OSXmlReader*>(reader_temporary);
  } else {
    free(reader_temporary);
    XmlControl::GetInstance()->ReleaseControl();
    return OSMakeErrorCode(kFuncId, kErrorNoData);
  }

  return 0;
}

/**
 * @brief Close a XML file.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSXmlParser::Close() {
  static const OSFunctionId kFuncId = kIdOSXmlParserClose;
  if (reader_ == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidOperation);
  }

  xmlFreeTextReader(*(reinterpret_cast<xmlTextReaderPtr*>(reader_)));

  xmlCleanupCharEncodingHandlers();

  xmlDictCleanup();

  free(reader_);
  reader_ = NULL;

  XmlControl::GetInstance()->ReleaseControl();

  return 0;
}

/**
 * @brief Parse the XML file one line, and get the node type.
 * @param[out] type  Pointer to the variable that receives the node type.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSXmlParser::Parse(OSXmlNodeType* type) {
  static const OSFunctionId kFuncId = kIdOSXmlParserParse;
  if (reader_ == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidOperation);
  }
  if (type == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  int32_t result = xmlTextReaderRead(
      *(reinterpret_cast<xmlTextReaderPtr*>(reader_)));
  if (result != 1) {
    return OSMakeErrorCode(kFuncId, kErrorNoData);
  }

  int type_temporary = xmlTextReaderNodeType(
      *(reinterpret_cast<xmlTextReaderPtr*>(reader_)));

  switch (type_temporary) {
    case XML_READER_TYPE_ELEMENT:
      *type = kOSXmlElementNode;
      break;
    case XML_READER_TYPE_END_ELEMENT:
      *type = kOSXmlElementEnd;
      break;
    default:
      *type = kOSXmlUnsupportedNode;
      break;
  }

  return 0;
}

/**
 * @brief Get a attribute from the current node.
 * @param[in]  name      Attribute name.
 * @param[out] attribute Pointer to the string that receives the attribute
 *                       value.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSXmlParser::GetAttribute(const std::string& name,
                                  std::string* attribute) {
  static const OSFunctionId kFuncId = kIdOSXmlParserGetAttribute;
  if (reader_ == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidOperation);
  }
  if (attribute == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  int32_t result = xmlTextReaderHasAttributes(
      *(reinterpret_cast<xmlTextReaderPtr*>(reader_)));
  if (result == 0) {
    return OSMakeErrorCode(kFuncId, kErrorNoData);
  }

  xmlChar* attribute_temporary = xmlTextReaderGetAttribute(
      (*(reinterpret_cast<xmlTextReaderPtr*>(reader_))),
      reinterpret_cast<const xmlChar*>(name.c_str()));
  if (attribute_temporary == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorNoData);
  }

  *attribute = reinterpret_cast<const char*>(attribute_temporary);
  xmlFree(attribute_temporary);

  return 0;
}

/**
 * @brief Get the element name from the current node.
 * @param[out] element Pointer to the string that receives the element name.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSXmlParser::GetElement(std::string* element) {
  static const OSFunctionId kFuncId = kIdOSXmlParserGetElement;
  if (reader_ == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidOperation);
  }
  if (element == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  const xmlChar* element_temporary = xmlTextReaderConstName(
      (*(reinterpret_cast<xmlTextReaderPtr*>(reader_))));
  if (element_temporary == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorNoData);
  }

  *element = reinterpret_cast<const char*>(element_temporary);

  return 0;
}

/**
 * @brief Get the depth from the current node.
 *
 * Example:
 * <parent>    depth = 0
 *   <child>   depth = 1
 *   </child>  depth = 1
 * </parent>   depth = 0
 *
 * @param[out] depth Pointer to a variable that receives the depth.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSXmlParser::GetDepth(uint32_t* depth) {
  static const OSFunctionId kFuncId = kIdOSXmlParserGetDepth;
  if (reader_ == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidOperation);
  }
  if (depth == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  int32_t value = xmlTextReaderDepth(
      (*(reinterpret_cast<xmlTextReaderPtr*>(reader_))));
  if (value < 0) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }
  *depth = static_cast<uint32_t>(value);
  return 0;
}

}  // namespace osal
}  // namespace senscord
