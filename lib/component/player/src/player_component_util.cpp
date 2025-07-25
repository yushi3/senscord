/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "player_component_util.h"

#include <inttypes.h>
#include <stdint.h>

#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <limits>  // std::numeric_limits

#include "senscord/logger.h"
#include "./player_property_accessor.h"

static const char* kModuleName = "player_component_util";

namespace player {
/**
 * @brief Open file
 * @param[in] (file_path) file name path.
 * @param[out] (file) The file pointer of opened file.
 * @param[out] (file_size) The size of opened file.
 * @return Status object.
 */
senscord::Status OpenFile(
    const std::string& file_path, senscord::osal::OSFile** file,
    size_t* file_size) {
  // open
  senscord::osal::OSFile* fp = NULL;
  int32_t ret = senscord::osal::OSFopen(file_path.c_str(), "rb", &fp);
  if (ret != 0) {
    return SENSCORD_STATUS_FAIL(
        kModuleName, senscord::Status::kCauseAborted,
        "fail to open file: 0x%" PRIx32 ":%s", ret, file_path.c_str());
  }

  // get file size
  size_t size = 0;
  ret = senscord::osal::OSGetBinaryFileSize(fp, &size);
  if (ret != 0) {
    senscord::osal::OSFclose(fp);
    return SENSCORD_STATUS_FAIL(
        kModuleName, senscord::Status::kCauseAborted,
        "fail to get size: 0x%" PRIx32 ":%s", ret, file_path.c_str());
  }

  *file = fp;
  *file_size = size;

  return senscord::Status::OK();
}

/**
 * @brief Read file
 * @param[in] (file) The file pointer of read file.
 * @param[out] (read_buffer) The buffer of read data.
 * @param[in] (read_size) The size of read data.
 * @param[in] (read_offset) The offset of read data.
 * @return Status object.
 */
senscord::Status ReadFile(
    senscord::osal::OSFile* file, void* read_buffer,
    const size_t read_size, const size_t read_offset) {
  int32_t ret = 0;
  // seek
  ret = senscord::osal::OSFseek(
      file, read_offset, senscord::osal::kSeekSet);
  if (ret != 0) {
    return SENSCORD_STATUS_FAIL(
        kModuleName, senscord::Status::kCauseAborted,
        "fail to seek raw_index.dat: 0x%" PRIx32, ret);
  }

  // read
  size_t read_num = 0;
  ret = senscord::osal::OSFread(
      read_buffer, 1, read_size, file, &read_num);
  if (ret != 0) {
    return SENSCORD_STATUS_FAIL(
        kModuleName, senscord::Status::kCauseAborted,
        "fail to read raw_index.dat: 0x%" PRIx32, ret);
  }

  return senscord::Status::OK();
}

/**
 * @brief read all file data
 * @param[in] (file_path) file name path.
 * @param[out] (buffer) A buffer to store file data
 * @return Status object.
 */
senscord::Status FileReadAllData(
    const char* file_path, std::vector<uint8_t>* buffer) {
  senscord::osal::OSFile* file = NULL;
  int32_t ret;

  if (file_path == NULL) {
    return SENSCORD_STATUS_FAIL(kModuleName,
                                senscord::Status::kCauseInvalidArgument,
                                "file_path is NULL");
  }
  if (buffer == NULL) {
    return SENSCORD_STATUS_FAIL(
        kModuleName, senscord::Status::kCauseInvalidArgument, "buffer is NULL");
  }

  size_t length = 0;
  senscord::Status status = FileGetSize(file_path, &length);
  if (!status.ok()) {
    SENSCORD_LOG_WARNING("fail FileGetSize(): %s", file_path);
    return SENSCORD_STATUS_TRACE(status);
  }

  buffer->resize(length);

  ret = senscord::osal::OSFopen(file_path, "rb", &file);
  if (ret != 0) {
    return SENSCORD_STATUS_FAIL(kModuleName, senscord::Status::kCauseAborted,
                                "fail to open file: 0x%" PRIx32 ":%s", ret,
                                file_path);
  }

  size_t read_num = 0;
  ret = OSFread(buffer->data(), 1, length, file, &read_num);
  if (ret != 0) {
    OSFclose(file);
    return SENSCORD_STATUS_FAIL(kModuleName, senscord::Status::kCauseAborted,
                                "fail to read file: 0x%" PRIx32 ":%s", ret,
                                file_path);
  }
  if (length != read_num) {
    OSFclose(file);
    return SENSCORD_STATUS_FAIL(kModuleName, senscord::Status::kCauseAborted,
                                "invalid file size, file_size=%" PRIu64
                                " read_len=%" PRIu64 ":%s",
                                length, read_num, file_path);
  }
  OSFclose(file);
  return senscord::Status::OK();
}

/**
 * @brief get file size
 * @param[in] (file_path) file name path.
 * @param[out] (out_len) file size in bytes
 * @return Status object.
 */
senscord::Status FileGetSize(const char* file_path, size_t* out_len) {
  senscord::osal::OSFile* file = NULL;
  int32_t ret;

  if (file_path == NULL) {
    return SENSCORD_STATUS_FAIL(kModuleName,
                                senscord::Status::kCauseInvalidArgument,
                                "file_path is NULL");
  }
  if (out_len == NULL) {
    return SENSCORD_STATUS_FAIL(kModuleName,
                                senscord::Status::kCauseInvalidArgument,
                                "out_len is NULL");
  }

  ret = senscord::osal::OSFopen(file_path, "rb", &file);
  if (ret != 0) {
    return SENSCORD_STATUS_FAIL(kModuleName, senscord::Status::kCauseNotFound,
                                "fail to open file: 0x%" PRIx32 " %s", ret,
                                file_path);
  }
  ret = senscord::osal::OSGetBinaryFileSize(file, out_len);
  if (ret != 0) {
    OSFclose(file);
    return SENSCORD_STATUS_FAIL(kModuleName, senscord::Status::kCauseNotFound,
                                "fail to get file size: 0x%" PRIx32 " %s", ret,
                                file_path);
  }
  OSFclose(file);
  if (*out_len == 0) {  // regard file size 0 as kCauseNotFound
    return SENSCORD_STATUS_FAIL(kModuleName, senscord::Status::kCauseNotFound,
                                "fail to open file: 0x%" PRIx32 " %s", ret,
                                file_path);
  }
  return senscord::Status::OK();
}

/**
 * @brief get attribute data(uint64_t)
 * @param[in] (parser) pointer to the parser.
 * @param[in] (name) name of attribute.
 * @param[out] (num) Pointer to the variable that receives from attribute value
 * @return Status object.
 */
senscord::Status GetAttributeUInt32(
    senscord::osal::OSXmlParser* parser,
    const std::string& name, uint32_t* num) {
  if (parser == NULL) {
    return SENSCORD_STATUS_FAIL(
        kModuleName, senscord::Status::kCauseAborted, "parser is NULL");
  }
  if (num == NULL) {
    return SENSCORD_STATUS_FAIL(
        kModuleName, senscord::Status::kCauseAborted, "num is NULL");
  }

  // Get attributes as a string
  std::string str_value;
  int32_t result = parser->GetAttribute(name, &str_value);
  if (result != 0) {
    return SENSCORD_STATUS_FAIL(
        kModuleName, senscord::Status::kCauseInvalidArgument,
        "GetAttribute failed(%s): ret=%" PRId32, name.c_str(), result);
  }

  // Convert attributes to Signed 64bit numbers
  int64_t signed_value = 0;
  result = senscord::osal::OSStrtoll(str_value.c_str(), NULL, 0, &signed_value);
  if (result != 0) {
    return SENSCORD_STATUS_FAIL(
        kModuleName, senscord::Status::kCauseInvalidArgument,
        "OSStrtoll failed(%s): ret=%" PRId32, name.c_str(), result);
  }

  // Check whether a value can be converted to uint32_t type
  if ((signed_value > std::numeric_limits<uint32_t>::max()) ||
      (signed_value < 0)) {
    return SENSCORD_STATUS_FAIL(
        kModuleName, senscord::Status::kCauseInvalidArgument,
        "Number is out of range of uint32 type: %" PRId64, signed_value);
  }

  // apply value
  *num = static_cast<uint32_t>(signed_value);

  return senscord::Status::OK();
}
/**
 * @brief get attribute string(std::string)
 * @param[in] (parser) pointer to the parser.
 * @param[in] (attribute_name) pointer to attribute string.
 * @param[out] (p_out_str) Pointer to the string that receives from attribute
 * value
 * @return Status object.
 */
senscord::Status GetAttributeString(senscord::osal::OSXmlParser* parser,
                                    const char* attribute_name,
                                    std::string* p_out_str) {
  if (attribute_name == NULL) {
    return SENSCORD_STATUS_FAIL(kModuleName,
                                senscord::Status::kCauseInvalidArgument,
                                "attribute_name is NULL");
  }
  if (p_out_str == NULL) {
    return SENSCORD_STATUS_FAIL(kModuleName,
                                senscord::Status::kCauseInvalidArgument,
                                "p_out_id is NULL");
  }
  int result = parser->GetAttribute(attribute_name, p_out_str);
  if (result != 0) {
    return SENSCORD_STATUS_FAIL(kModuleName,
                                senscord::Status::kCauseInvalidArgument,
                                "GetAttribute failed. \"%s\"", attribute_name);
  }
  return senscord::Status::OK();
}

/**
 * @brief allocate memory in this API and read file data in this memory
 * @param[in] (allocator) allocator
 * @param[in] (target_path) directory where playing files are placed on.
 * @param[in] (channel_number) channel number
 * @param[in] (sequence_number) sequence number
 * @param[out] (memory) allocated memory
 * @return Status object.
 */
senscord::Status ReadRawFile(senscord::MemoryAllocator* allocator,
                             const std::string& target_path,
                             uint32_t channel_number, uint64_t sequence_number,
                             senscord::Memory** memory) {
  std::string full_path;
  std::string path1;
  std::string path2;
  senscord::Status status;
  senscord::osal::OSFile* file = NULL;
  int32_t ret;

  if (allocator == NULL) {
    return SENSCORD_STATUS_FAIL(kModuleName,
                                senscord::Status::kCauseInvalidArgument,
                                "allocator is NULL");
  }

  if (memory == NULL) {
    return SENSCORD_STATUS_FAIL(
        kModuleName, senscord::Status::kCauseInvalidArgument, "memory is NULL");
  }

  senscord::RecordUtility::GetChannelDirectoryName(channel_number, &path1);
  senscord::RecordUtility::GetRawDataFileName(sequence_number, &path2);
  full_path = target_path + senscord::osal::kDirectoryDelimiter + path1 +
              senscord::osal::kDirectoryDelimiter + path2;

  size_t file_size = 0;
  status = player::FileGetSize(full_path.c_str(), &file_size);
  if (!status.ok()) {
    SENSCORD_LOG_WARNING("fail player::FileGetSize(): %s", full_path.c_str());
    return SENSCORD_STATUS_TRACE(status);
  }

  // open
  ret = senscord::osal::OSFopen(full_path.c_str(), "rb", &file);
  if (ret != 0) {
    return SENSCORD_STATUS_FAIL(kModuleName, senscord::Status::kCauseAborted,
                                "failed to open recording file: 0x%" PRIx32,
                                ret);
  }

  // allocate memory
  status = allocator->Allocate(file_size, memory);
  if (!status.ok()) {
    const char* err_format =
        "fail to alloc memory for raw file: size=%" PRIu64 " %s";
    SENSCORD_STATUS_TRACE(status);
    SENSCORD_LOG_WARNING(err_format, file_size, full_path.c_str());
    senscord::osal::OSFclose(file);
    return SENSCORD_STATUS_FAIL(kModuleName,
                                senscord::Status::kCauseResourceExhausted,
                                err_format, file_size, full_path.c_str());
  }

  // read
  size_t read_len = 0;
  ret = senscord::osal::OSFread(
      reinterpret_cast<void*>((*memory)->GetAddress()),
      sizeof(uint8_t), file_size, file, &read_len);
  // close
  senscord::osal::OSFclose(file);
  if (ret != 0) {
    allocator->Free(*memory);
    *memory = NULL;
    return SENSCORD_STATUS_FAIL(kModuleName, senscord::Status::kCauseAborted,
                                "failed to read recording file: 0x%" PRIx32,
                                ret);
  }
  if (file_size != read_len) {
    allocator->Free(*memory);
    *memory = NULL;
    return SENSCORD_STATUS_FAIL(kModuleName, senscord::Status::kCauseAborted,
                                "invalid file size, file_size=%" PRIu64
                                " read_len=%" PRIu64,
                                file_size, read_len);
  }

  return senscord::Status::OK();
}

/**
 * @brief Clear PlayProperty data
 * @param[out] (prop) prop data is cleared
 * @return none
 */
void ClearPlayProperty(senscord::PlayProperty* prop) {
  if (prop != NULL) {
    prop->target_path = std::string("");
    prop->start_offset = 0;
    prop->count = 0;
    prop->speed = senscord::kPlaySpeedBasedOnFramerate;
    prop->mode.repeat = false;
  }
}

/**
 * @brief Parse argment for openport.
 * @param[in] (port_type) The type of port.
 * @param[in] (port_id) The ID of port type.
 * @param[in] (args) Arguments of port starting.
 * @param[out] (prop) Reference to PlayProperty.
 * @return Status object.
 */
senscord::Status OpenPortParseArg(const std::string& port_type, int32_t port_id,
                                  const senscord::ComponentPortArgument& args,
                                  senscord::PlayProperty* prop) {
  int result;
  uint64_t num;

  if (prop == NULL) {
    return SENSCORD_STATUS_FAIL(
        kModuleName, senscord::Status::kCauseInvalidArgument, "prop is NULL");
  }

  prop->speed = senscord::kPlaySpeedBasedOnFramerate;

  std::map<std::string, std::string>::const_iterator itr;
  for (itr = args.arguments.begin(); itr != args.arguments.end(); ++itr) {
    std::string name = itr->first;
    std::string value = itr->second;
    SENSCORD_LOG_DEBUG("OpenPort args: name=%s, value=%s", itr->first.c_str(),
                       itr->second.c_str());
    if (name == "target_path") {
      prop->target_path = value;
    } else if (name == "repeat") {
      if (value == "true") {
        prop->mode.repeat = true;
      } else if (value == "false") {
        prop->mode.repeat = false;
      } else {
        return SENSCORD_STATUS_FAIL(kModuleName,
            senscord::Status::kCauseInvalidArgument,
            "fail in parse args \"repeat=%s\"", value.c_str());
      }
    } else if (name == "start_offset") {
      result = senscord::osal::OSStrtoull(value.c_str(), NULL, 0, &num);

      if ((result == 0) && (value.find("-") == std::string::npos)) {
        prop->start_offset = static_cast<uint32_t>(num);
      } else {
        return SENSCORD_STATUS_FAIL(kModuleName,
            senscord::Status::kCauseInvalidArgument,
            "fail in parse args \"start_offset=%s\"", value.c_str());
      }
    } else if (name == "count") {
      if (value == "all") {
        prop->count = senscord::kPlayCountAll;
      } else {
        result = senscord::osal::OSStrtoull(value.c_str(), NULL, 0, &num);
        if ((result == 0) && (value.find("-") == std::string::npos)) {
          prop->count = static_cast<uint32_t>(num);
        } else {
          return SENSCORD_STATUS_FAIL(kModuleName,
              senscord::Status::kCauseInvalidArgument,
              "fail in parse args \"count=%s\"", value.c_str());
        }
      }
    }
  }
  return senscord::Status::OK();
}
}  // namespace player
