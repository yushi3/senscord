/*
 * SPDX-FileCopyrightText: 2017-2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_OSAL_NUTTX_OSAL_ERROR_H_
#define LIB_OSAL_NUTTX_OSAL_ERROR_H_

#include <stdint.h>
#include "senscord/osal_error.h"

namespace senscord {
namespace osal {

const int32_t kFunctionIdMask = 0xFFFF;
const int32_t kFunctionIdShiftBit = 8;
const int32_t kErrorCauseMask = 0xFF;

/**
 * @brief OSAL function identifier.
 */
enum OSFunctionId {
  kIdNone                           = 0x0000,

  kIdStdio                          = 0x0100,
  kIdOSPrintf,                      // 0x0101
  kIdOSVprintf,                     // 0x0102
  kIdOSVsnprintf,                   // 0x0103

  kIdFile                           = 0x0200,
  kIdOSFopen,                       // 0x0201
  kIdOSFclose,                      // 0x0202
  kIdOSRemove,                      // 0x0203
  kIdOSFwrite,                      // 0x0204
  kIdOSFread,                       // 0x0205
  kIdOSFseek,                       // 0x0206
  kIdOSFtell,                       // 0x0207
  kIdOSFerror,                      // 0x0208
  kIdOSFeof,                        // 0x0209
  kIdOSFclearError,                 // 0x020A
  kIdOSFflush,                      // 0x020B
  kIdOSGetFileSize,                 // 0x020C

  kIdThread                         = 0x0300,
  kIdOSCreateThread,                // 0x0301
  kIdOSDetachThread,                // 0x0302
  kIdOSJoinThread,                  // 0x0303
  kIdOSTimedJoinThread,             // 0x0304
  kIdOSRelativeTimedJoinThread,     // 0x0305
  kIdOSSetThreadPriority,           // 0x0306
  kIdOSGetThreadPriority,           // 0x0307
  kIdOSGetCurrentThread,            // 0x0308

  kIdMutex                          = 0x0400,
  kIdOSCreateMutex,                 // 0x0401
  kIdOSDestroyMutex,                // 0x0402
  kIdOSLockMutex,                   // 0x0403
  kIdOSTimedLockMutex,              // 0x0404
  kIdOSRelativeTimedLockMutex,      // 0x0405
  kIdOSTryLockMutex,                // 0x0406
  kIdOSUnlockMutex,                 // 0x0407

  kIdCond                           = 0x0500,
  kIdOSCreateCond,                  // 0x0501
  kIdOSDestroyCond,                 // 0x0502
  kIdOSWaitCond,                    // 0x0503
  kIdOSTimedWaitCond,               // 0x0504
  kIdOSRelativeTimedWaitCond,       // 0x0505
  kIdOSSignalCond,                  // 0x0506
  kIdOSBroadcastCond,               // 0x0507

  kIdSocket                         = 0x0600,
  kIdOSCreateSocket,                // 0x0601
  kIdOSShutdownSocket,              // 0x0602
  kIdOSDestroySocket,               // 0x0603
  kIdOSBindSocket,                  // 0x0604
  kIdOSListenSocket,                // 0x0605
  kIdOSAcceptSocket,                // 0x0606
  kIdOSConnectSocket,               // 0x0607
  kIdOSSendSocket,                  // 0x0608
  kIdOSSendToSocket,                // 0x0609
  kIdOSRecvSocket,                  // 0x060A
  kIdOSRecvFromSocket,              // 0x060B
  kIdOSSelectSocket,                // 0x060C
  kIdOSRelativeTimedSelectSocket,   // 0x060D
  kIdOSTimedSelectSocket,           // 0x060E
  kIdOSHtonl,                       // 0x060F
  kIdOSHtons,                       // 0x0610
  kIdOSNtohl,                       // 0x0611
  kIdOSNtohs,                       // 0x0612
  kIdOSInetAton,                    // 0x0613
  kIdOSInetNtoa,                    // 0x0614
  kIdOSSetSocketSendBufferSize,     // 0x0615
  kIdOSGetSocketSendBufferSize,     // 0x0616
  kIdOSSetSocketRecvBufferSize,     // 0x0617
  kIdOSGetSocketRecvBufferSize,     // 0x0618
  kIdOSSetSocketReuseAddr,          // 0x0619
  kIdOSSendMsgSocket,               // 0x061A
  kIdOSSetSocketTcpNoDelay,         // 0x061B
  kIdOSGetInetAddressList,          // 0x061C

  kIdMemory                         = 0x0700,
  kIdOSMemchr,                      // 0x0701
  kIdOSMemcmp,                      // 0x0702
  kIdOSMemcpy,                      // 0x0703
  kIdOSMemmove,                     // 0x0704
  kIdOSMemset,                      // 0x0705
  kIdOSMalloc,                      // 0x0706
  kIdOSFree,                        // 0x0707

  kIdMath                           = 0x0800,
  kIdOSFabs,                        // 0x0801

  kIdTime                           = 0x0900,
  kIdOSGetTime,                     // 0x0901
  kIdOSGetLocalTime,                // 0x0902

  kIdSleep                          = 0x0A00,
  kIdOSSleep,                       // 0x0A01

  kIdRandom                         = 0x0B00,
  kIdOSRand,                        // 0x0B01

  kIdTimer                          = 0x0C00,
  kIdOSTimerStartTimer,             // 0x0C01
  kIdOSTimerStopTimer,              // 0x0C02

  kIdDlLoad                         = 0x0D00,
  kIdOSDlLoad,                      // 0x0D01
  kIdOSDlGetFuncPtr,                // 0x0D02
  kIdOSDlFree,                      // 0x0D03

  kIdXmlParser                      = 0x0E00,
  kIdOSXmlParserOpen,               // 0x0E01
  kIdOSXmlParserClose,              // 0x0E02
  kIdOSXmlParserParse,              // 0x0E03
  kIdOSXmlParserGetAttribute,       // 0x0E04
  kIdOSXmlParserGetElement,         // 0x0E05

  kIdString                         = 0x0F00,
  kIdOSStrtoll,                     // 0x0F01
  kIdOSStrtoull,                    // 0x0F02
  kIdOSBasename,                    // 0x0F03

  kIdDirectory                      = 0x1000,
  kIdOSGetRegularFileList,          // 0x1001
  kIdOSGetEnvironment,              // 0x1002
  kIdOSMakeDirectory,               // 0x1003
  kIdOSRemoveDirectory,             // 0x1004

  kIdXmlCreator                         = 0x1100,
  kIdOSXmlCreatorOpen,                  // 0x1101
  kIdOSXmlCreatorClose,                 // 0x1102
  kIdOSXmlCreatorWriteComment,          // 0x1103
  kIdOSXmlCreatorWriteStartElemnt,      // 0x1104
  kIdOSXmlCreatorWriteEndElement,       // 0x1105
  kIdOSXmlCreatorWriteAttribute         // 0x1106
};

/**
 * @brief Make an error code.
 * @param[in] func_id  OSAL function identifier.
 * @param[in] cause    OSAL error cause.
 * @return Error code.
 *         If the error cause is kErrorNone, it returns success value (0).
 */
int32_t OSMakeErrorCode(OSFunctionId func_id, OSErrorCause cause);

}  //  namespace osal
}  //  namespace senscord

#endif  // LIB_OSAL_NUTTX_OSAL_ERROR_H_
