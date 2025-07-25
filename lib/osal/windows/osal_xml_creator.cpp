/*
 * SPDX-FileCopyrightText: 2017-2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <Shlwapi.h>
#include <xmllite.h>

#include <stdio.h>
#include <string>

#include "senscord/osal.h"
#include "common/osal_error.h"

namespace senscord {
namespace osal {

/**
* @brief XML reader structure.
*/
struct XmlWriter {
  /** XML writer interface created with CreateXmlWriter. */
  IXmlWriter* xml_writer;
  /** Stream interface opened with SHCreateStreamOnFile. */
  IStream* stream;
  /** Flag indicating whether an element has been written to xml. */
  bool is_written_element;
};

/**
* @brief Convert std::string to wchar.
*/
static void ConvertStringToWchar(const std::string& source,
                                 LPWSTR* destination) {
  *destination = new WCHAR[source.length() + 1];
  mbstowcs_s(NULL, *destination, source.length() + 1,
    source.c_str(), _TRUNCATE);
}

/**
 * @brief OSXmlCreator constructor.
 */
OSXmlCreator::OSXmlCreator() {
  writer_ = NULL;
}

/**
 * @brief OSXmlCreator destructor.
 */
OSXmlCreator::~OSXmlCreator() {
  Close();
}

/**
 * @brief Open a XML file.
 * @param[in] filename Path of the file to open.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSXmlCreator::Open(const std::string& file_name) {
  static const OSFunctionId kFuncId = kIdOSXmlCreatorOpen;
  if (writer_ != NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidOperation);
  }
  if (file_name.empty()) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  XmlWriter* temporary_writer = new XmlWriter();
  if (FAILED(SHCreateStreamOnFile(file_name.c_str(),
      STGM_CREATE | STGM_WRITE, &temporary_writer->stream))) {
    delete temporary_writer;
    return OSMakeErrorCode(kFuncId, kErrorNoData);
  }

  if (FAILED(CreateXmlWriter(__uuidof(IXmlWriter),
      reinterpret_cast<void**>(&temporary_writer->xml_writer), 0))) {
    temporary_writer->stream->Release();
    delete temporary_writer;
    return OSMakeErrorCode(kFuncId, kErrorNoData);
  }

  if (FAILED(temporary_writer->xml_writer->SetOutput(
      temporary_writer->stream))) {
    temporary_writer->xml_writer->Release();
    temporary_writer->stream->Release();
    delete temporary_writer;
    return OSMakeErrorCode(kFuncId, kErrorNoData);
  }

  if (FAILED(temporary_writer->xml_writer->SetProperty(
      XmlWriterProperty_Indent, TRUE))) {
    temporary_writer->xml_writer->Release();
    temporary_writer->stream->Release();
    delete temporary_writer;
    return OSMakeErrorCode(kFuncId, kErrorNoData);
  }

  if (FAILED(temporary_writer->xml_writer->WriteStartDocument(
    XmlStandalone_Omit))) {
    temporary_writer->xml_writer->Release();
    temporary_writer->stream->Release();
    delete temporary_writer;
    return OSMakeErrorCode(kFuncId, kErrorNoData);
  }

  temporary_writer->is_written_element = false;

  writer_ = reinterpret_cast<OSXmlWriter*>(temporary_writer);

  return 0;
}

/**
 * @brief Close a XML file.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSXmlCreator::Close() {
  static const OSFunctionId kFuncId = kIdOSXmlCreatorClose;
  if (writer_ == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidOperation);
  }

  XmlWriter* temporary_writer = reinterpret_cast<XmlWriter*>(writer_);

  if (temporary_writer->is_written_element == true) {
    if (FAILED(temporary_writer->xml_writer->WriteEndDocument())) {
      return OSMakeErrorCode(kFuncId, kErrorNoData);
    }
  }

  temporary_writer->xml_writer->Flush();
  temporary_writer->xml_writer->Release();
  temporary_writer->stream->Release();

  delete temporary_writer;
  writer_ = NULL;

  return 0;
}

/**
 * @brief Writing comment.
 * @param[in] comment Contents of the Comment to be written.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSXmlCreator::WriteComment(const std::string& comment) {
  static const OSFunctionId kFuncId = kIdOSXmlCreatorWriteComment;
  if (writer_ == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidOperation);
  }

  XmlWriter* temporary_writer = reinterpret_cast<XmlWriter*>(writer_);
  LPWSTR temporary_comment;

  ConvertStringToWchar(comment, &temporary_comment);

  temporary_writer->xml_writer->WriteComment(temporary_comment);

  delete[] temporary_comment;

  return 0;
}

/**
 * @brief Start writing element.
 * @param[in] name Name of the element to be written.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSXmlCreator::WriteStartElement(const std::string& name) {
  static const OSFunctionId kFuncId = kIdOSXmlCreatorWriteStartElemnt;
  if (writer_ == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidOperation);
  }
  if (name.empty()) {
    return OSMakeErrorCode(kFuncId, kErrorNoData);
  }

  XmlWriter* temporary_writer = reinterpret_cast<XmlWriter*>(writer_);
  LPWSTR temporary_name;

  ConvertStringToWchar(name, &temporary_name);

  if (FAILED(temporary_writer->xml_writer->WriteStartElement(
      NULL, temporary_name, NULL))) {
    delete[] temporary_name;
    return OSMakeErrorCode(kFuncId, kErrorInvalidOperation);
  }

  delete[] temporary_name;

  temporary_writer->is_written_element = true;

  return 0;
}

/**
 * @brief End the element currently being written.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSXmlCreator::WriteEndElement() {
  static const OSFunctionId kFuncId = kIdOSXmlCreatorWriteEndElement;
  if (writer_ == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidOperation);
  }

  XmlWriter* temporary_writer = reinterpret_cast<XmlWriter*>(writer_);

  if (FAILED(temporary_writer->xml_writer->WriteEndElement())) {
    return OSMakeErrorCode(kFuncId, kErrorNoData);
  }

  return 0;
}

/**
 * @brief Add attributes to the target tag.
 * @param[in] name Name of the attribute to be written.
 * @param[in] attribute The value of the attribute to be written.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSXmlCreator::WriteAttribute(const std::string& name,
                                     const std::string& attribute) {
  static const OSFunctionId kFuncId = kIdOSXmlCreatorWriteAttribute;
  if (writer_ == NULL) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidOperation);
  }
  if (name.empty()) {
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  XmlWriter* temporary_writer = reinterpret_cast<XmlWriter*>(writer_);
  LPWSTR temporary_name;
  LPWSTR temporary_attribute;

  ConvertStringToWchar(name, &temporary_name);
  ConvertStringToWchar(attribute, &temporary_attribute);

  if (FAILED(temporary_writer->xml_writer->WriteAttributeString(
    NULL, temporary_name, NULL, temporary_attribute))) {
    delete[] temporary_name;
    delete[] temporary_attribute;
    return OSMakeErrorCode(kFuncId, kErrorNoData);
  }

  delete[] temporary_name;
  delete[] temporary_attribute;

  return 0;
}

}  // namespace osal
}  // namespace senscord
