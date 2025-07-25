/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdint.h>
#include <Shlwapi.h>
#include <xmllite.h>

#include <string>
#include <map>

#include "senscord/osal.h"
#include "common/osal_error.h"
#include "windows/osal_xml_control.h"

namespace senscord {
namespace osal {

/**
 * @brief XML reader structure.
 */
struct XmlReader{
  /** XML reader interface created with CreateXmlReader. */
  IXmlReader* xml_reader;
  /** Stream interface opened with SHCreateStreamOnFile. */
  IStream* stream;
  /** Element name. */
  std::string element;
  /** List of attribute names and values. */
  std::map<std::string, std::string> attributes;
  /** Depth. */
  uint32_t depth;
};

/**
 * @brief Convert wchar to std::string.
 */
static void ConvertWcharToString(const LPCWSTR source,
                                 std::string* destination) {
  size_t length = wcslen(source);
  char* temporary = new char[length + 1];
  wcstombs_s(NULL, temporary, length + 1, source, _TRUNCATE);
  *destination = temporary;
  delete[] temporary;
}

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

  XmlReader* temporary_reader = new XmlReader();
  CreateXmlReader(IID_PPV_ARGS(&temporary_reader->xml_reader), NULL);

  if (FAILED(SHCreateStreamOnFile(file_name.c_str(), STGM_READ,
      &temporary_reader->stream))) {
    temporary_reader->xml_reader->Release();
    delete temporary_reader;
    XmlControl::GetInstance()->ReleaseControl();
    return OSMakeErrorCode(kFuncId, kErrorNoData);
  }

  if (FAILED(temporary_reader->xml_reader->SetInput(
      temporary_reader->stream))) {
    temporary_reader->xml_reader->Release();
    temporary_reader->stream->Release();
    delete temporary_reader;
    XmlControl::GetInstance()->ReleaseControl();
    return OSMakeErrorCode(kFuncId, kErrorNoData);
  }

  reader_ = reinterpret_cast<OSXmlReader*>(temporary_reader);

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

  XmlReader* temporary_reader = reinterpret_cast<XmlReader*>(reader_);
  temporary_reader->xml_reader->Release();
  temporary_reader->stream->Release();
  delete temporary_reader;
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

  XmlNodeType temporary_type;
  XmlReader* temporary_reader = reinterpret_cast<XmlReader*>(reader_);
  HRESULT result = temporary_reader->xml_reader->Read(&temporary_type);
  if (result != S_OK) {
    return OSMakeErrorCode(kFuncId, kErrorNoData);
  }

  temporary_reader->element = "";
  temporary_reader->attributes.clear();
  temporary_reader->depth = 0;

  if (temporary_type == XmlNodeType_Element) {
    *type = kOSXmlElementNode;

    // element
    LPCWSTR temporary_element;
    if (FAILED(temporary_reader->xml_reader->GetLocalName(
        &temporary_element, NULL))) {
      return OSMakeErrorCode(kFuncId, kErrorNoData);
    }
    ConvertWcharToString(temporary_element, &temporary_reader->element);

    // depth
    if (FAILED(temporary_reader->xml_reader->GetDepth(
        &temporary_reader->depth))) {
      return OSMakeErrorCode(kFuncId, kErrorNoData);
    }

    // attribute
    while (S_OK == temporary_reader->xml_reader->MoveToNextAttribute()) {
      LPCWSTR temporary_name;
      LPCWSTR temporary_value;
      if (FAILED(temporary_reader->xml_reader->GetLocalName(
          &temporary_name, NULL))) {
        return OSMakeErrorCode(kFuncId, kErrorNoData);
      }
      if (FAILED(temporary_reader->xml_reader->GetValue(
          &temporary_value, NULL))) {
        return OSMakeErrorCode(kFuncId, kErrorNoData);
      }

      std::string key;
      std::string value;
      ConvertWcharToString(temporary_name, &key);
      ConvertWcharToString(temporary_value, &value);

      temporary_reader->attributes[key] = value;
    }
  } else if (temporary_type == XmlNodeType_EndElement) {
    *type = kOSXmlElementEnd;

    // element
    LPCWSTR temporary_element;
    if (FAILED(temporary_reader->xml_reader->GetLocalName(
        &temporary_element, NULL))) {
      return OSMakeErrorCode(kFuncId, kErrorNoData);
    }
    ConvertWcharToString(temporary_element, &temporary_reader->element);

    // depth
    if (FAILED(temporary_reader->xml_reader->GetDepth(
        &temporary_reader->depth))) {
      return OSMakeErrorCode(kFuncId, kErrorNoData);
    }
    --temporary_reader->depth;
  } else {
    *type = kOSXmlUnsupportedNode;
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

  XmlReader* temporary_reader = reinterpret_cast<XmlReader*>(reader_);
  if (temporary_reader->attributes.count(name) == 0) {
    return OSMakeErrorCode(kFuncId, kErrorNoData);
  }
  *attribute = temporary_reader->attributes[name];

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

  XmlReader* temporary_reader = reinterpret_cast<XmlReader*>(reader_);
  if (temporary_reader->element.empty()) {
    return OSMakeErrorCode(kFuncId, kErrorNoData);
  }
  *element = temporary_reader->element;

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
  XmlReader* temporary_reader = reinterpret_cast<XmlReader*>(reader_);
  *depth = temporary_reader->depth;
  return 0;
}

}  // namespace osal
}  // namespace senscord
