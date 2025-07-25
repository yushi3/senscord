/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <libxml/xmlwriter.h>
#include <libxml/encoding.h>
#include <string>

#include "senscord/osal.h"
#include "common/osal_error.h"
#include "linux/osal_xml_control.h"

namespace senscord {
namespace osal {

const char kEncoding[] = "UTF-8";
const int32_t kIndent = 1;  // enabled
const xmlChar kIndentString[] = "  ";


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

  XmlControl::GetInstance()->GetControl();

  xmlTextWriterPtr* writer_temporary = reinterpret_cast<xmlTextWriterPtr*>(
      malloc(sizeof(xmlTextWriterPtr)));
  if (writer_temporary == NULL) {
    XmlControl::GetInstance()->ReleaseControl();
    return OSMakeErrorCode(kFuncId, kErrorOutOfMemory);
  }

  *writer_temporary = xmlNewTextWriterFilename(file_name.c_str(), 0);
  if (*writer_temporary == NULL) {
    free(writer_temporary);
    XmlControl::GetInstance()->ReleaseControl();
    return OSMakeErrorCode(kFuncId, kErrorNoData);
  }

  int32_t result = xmlTextWriterSetIndent(*writer_temporary, kIndent);
  if (result < 0) {
    xmlFreeTextWriter(*writer_temporary);
    free(writer_temporary);
    XmlControl::GetInstance()->ReleaseControl();
    return OSMakeErrorCode(kFuncId, kErrorInvalidOperation);
  }

  result = xmlTextWriterSetIndentString(*writer_temporary, kIndentString);
  if (result < 0) {
    xmlFreeTextWriter(*writer_temporary);
    free(writer_temporary);
    XmlControl::GetInstance()->ReleaseControl();
    return OSMakeErrorCode(kFuncId, kErrorInvalidOperation);
  }

  result = xmlTextWriterStartDocument(*writer_temporary, NULL, kEncoding, NULL);
  if (result < 0) {
    xmlFreeTextWriter(*writer_temporary);
    free(writer_temporary);
    XmlControl::GetInstance()->ReleaseControl();
    return OSMakeErrorCode(kFuncId, kErrorNoData);
  }

  writer_ = reinterpret_cast<OSXmlWriter*>(writer_temporary);

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

  int32_t result = xmlTextWriterEndDocument(
    *(reinterpret_cast<xmlTextWriterPtr*>(writer_)));
  if (result < 0) {
    XmlControl::GetInstance()->ReleaseControl();
    return OSMakeErrorCode(kFuncId, kErrorNoData);
  }

  xmlFreeTextWriter(*(reinterpret_cast<xmlTextWriterPtr*>(writer_)));

  free(writer_);
  writer_ = NULL;

  XmlControl::GetInstance()->ReleaseControl();

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

  int32_t result = xmlTextWriterWriteComment(
    *(reinterpret_cast<xmlTextWriterPtr*>(writer_)), BAD_CAST comment.c_str());
  if (result < 0) {
    return OSMakeErrorCode(kFuncId, kErrorNoData);
  }

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
    return OSMakeErrorCode(kFuncId, kErrorInvalidArgument);
  }

  int32_t result = xmlTextWriterStartElement(
    *(reinterpret_cast<xmlTextWriterPtr*>(writer_)), BAD_CAST name.c_str());
  if (result < 0) {
    return OSMakeErrorCode(kFuncId, kErrorNoData);
  }

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

  int32_t result = xmlTextWriterEndElement(
    *(reinterpret_cast<xmlTextWriterPtr*>(writer_)));
  if (result < 0) {
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

  int32_t result = xmlTextWriterWriteAttribute(
    *(reinterpret_cast<xmlTextWriterPtr*>(writer_)),
    BAD_CAST name.c_str(), BAD_CAST attribute.c_str());
  if (result < 0) {
    return OSMakeErrorCode(kFuncId, kErrorNoData);
  }

  return 0;
}

}  // namespace osal
}  // namespace senscord
