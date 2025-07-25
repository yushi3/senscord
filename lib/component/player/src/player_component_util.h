/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_COMPONENT_PLAYER_SRC_PLAYER_COMPONENT_UTIL_H_
#define LIB_COMPONENT_PLAYER_SRC_PLAYER_COMPONENT_UTIL_H_

#include <inttypes.h>
#include <stdint.h>
#include <map>
#include <string>
#include <vector>

#include "./player_component_types.h"
#include "senscord/develop/component.h"
#include "senscord/develop/recorder_common.h"
#include "senscord/osal.h"
#include "senscord/osal_inttypes.h"
#include "senscord/property_types.h"

namespace player {
/**
 * @brief Open file
 * @param[in] (file_path) file name path.
 * @param[out] (file) The file pointer of opened file.
 * @param[out] (file_size) The size of opend file.
 * @return Status object.
 */
senscord::Status OpenFile(
    const std::string& file_path, senscord::osal::OSFile** file,
    size_t* file_size);

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
    const size_t read_size, const size_t read_offset);

/**
 * @brief read all file data
 * @param[in] (file_path) file name path.
 * @param[out] (buffer) A buffer to store file data
 * @return Status object.
 */
senscord::Status FileReadAllData(
    const char* file_path, std::vector<uint8_t>* buffer);

/**
 * @brief get file size
 * @param[in] (file_path) file name path.
 * @param[out] (out_len) file size in bytes
 * @return Status object.
 */
senscord::Status FileGetSize(const char* file_path, size_t* out_len);

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
                             senscord::Memory** memory);

/**
 * @brief get attribute data (uint32_t)
 * @param[in] (parser) pointer to the parser.
 * @param[in] (name) pointer to attribute string.
 * @param[out] (num) Pointer to the variable that receives from attribute value
 * @return Status object.
 */
senscord::Status GetAttributeUInt32(
    senscord::osal::OSXmlParser* parser,
    const std::string& name, uint32_t* num);

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
                                    std::string* p_out_str);

/**
 * @brief Clear PlayProperty data
 * @param[out] (prop) prop data is cleared
 * @return none
 */
void ClearPlayProperty(senscord::PlayProperty* prop);

senscord::Status OpenPortParseArg(const std::string& port_type, int32_t port_id,
                                  const senscord::ComponentPortArgument& args,
                                  senscord::PlayProperty* prop);
}  // namespace player

#endif  // LIB_COMPONENT_PLAYER_SRC_PLAYER_COMPONENT_UTIL_H_
