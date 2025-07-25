/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_OSAL_H_
#define SENSCORD_OSAL_H_

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <vector>
#include <string>

#include "senscord/config.h"
#include "senscord/osal_error.h"
#include "senscord/osal_inttypes.h"
#include "senscord/noncopyable.h"

namespace senscord {
namespace osal {

/* Standard IO */
/**
 * @brief Print formatted output to the standard output stream.
 * @param[in] format  Format string.
 * @param[in] ...     Optional arguments.
 * @return On success, the total number of written characters is returned.
 *         Negative value is fail. (error code)
 */
int32_t OSPrintf(const char* format, ...);

/**
 * @brief Print formatted output specified by va_list to the standard output
 *        stream.
 * @param[in] format  Format string.
 * @param[in] args    List of arguments.
 * @return On success, the total number of written characters is returned.
 *         Negative value is fail. (error code)
 */
int32_t OSVprintf(const char* format, va_list args);

/**
 * @brief Outputs the converted character string to the buffer.
 *
 * For the following parameters, an error occurs.
 *   buffer == NULL : kErrorInvalidArgument
 *   format == NULL : kErrorInvalidArgument
 *   size   == 0    : kErrorInvalidArgument
 *
 * If the length of the converted string is greater than the buffer size,
 * the string is truncated and the return value is the total number of
 * written characters.
 *
 * @param[out] buffer  Pointer to the buffer that stores the converted
 *                     character string.
 * @param[in]  size    Size of buffer.
 * @param[in]  format  Format string.
 * @param[in]  args    List of arguments.
 * @return On success, the total number of written characters is returned.
 *         Negative value is fail. (error code)
 */
int32_t OSVsnprintf(char* buffer, size_t size,
                    const char* format, va_list args);

/* String */
/**
 * @brief Radix auto.
 */
const uint8_t kOSRadixAuto = 0;
/**
 * @brief Radix max.
 * Character is converted to number. ('A' represents 10, 'Z' represents 35)
 */
const uint8_t kOSRadixMax  = 36;
/**
 * @brief Radix min.
 * It means binary number.
 */
const uint8_t kOSRadixMin  = 2;

/**
 * @brief Convert a string to a 64-bit signed integer.
 * @param[in]  target_string  String to convert.
 * @param[out] end_string     Pointer to the variable that receives the stop
 *                            position.
 * @param[in]  radix          Radix to use.
 * @param[out] convert_value  Pointer to the variable that receives the
 *                            converted number.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSStrtoll(const char* target_string,
                  char** end_string, uint8_t radix, int64_t* convert_value);

/**
 * @brief Convert a string to a 64-bit unsigned integer.
 * @param[in]  target_string  String to convert.
 * @param[out] end_string     Pointer to the variable that receives the stop
 *                            position.
 * @param[in]  radix          Radix to use.
 * @param[out] convert_value  Pointer to the variable that receives the
 *                            converted number.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSStrtoull(const char* target_string,
                   char** end_string, uint8_t radix, uint64_t* convert_value);

/**
 * @brief Extract file name from file path.
 * @param[in] path  File path.
 * @return Pointer of extracted file name, or NULL.
 */
const char* OSBasename(const char* path);

/* File */
/**
 * @brief File object.
 */
typedef struct OSFile OSFile;

/**
 * @brief File seek origin.
 * @see OSFseek
 */
enum OSFileSeekOrigin {
  kSeekSet,  /**< Beginning of file. */
  kSeekCur,  /**< Current position of the file pointer. */
  kSeekEnd,  /**< End of file. */
};

/**
 * @brief Open a file.
 * @param[in]  file_path Path of the file to be opened.
 * @param[in]  mode      String containing the file access mode.
 * @param[out] file      Pointer to the variable that receives the file object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSFopen(const char* file_path, const char* mode, OSFile** file);

/**
 * @brief Close a file.
 * @param[in] file  File object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSFclose(OSFile* file);

/**
 * @brief Remove a file.
 * @param[in] path_name  Path of the file to delete.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSRemove(const char* path_name);

/**
 * @brief Output of the binary stream.
 * @param[in]  buffer      Pointer to the array of elements to be written.
 * @param[in]  member_size Size in bytes of each element to be written.
 * @param[in]  member_num  Number of elements, each one with size of
 *                         member_size bytes.
 * @param[in]  file        File object.
 * @param[out] written_num Total number of elements successfully written.
 *                         (optional)
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSFwrite(const void* buffer, size_t member_size, size_t member_num,
                 OSFile* file, size_t* written_num);

/**
 * @brief Input of the binary stream.
 * @param[out] buffer      Pointer to the buffer that stores the read data.
 * @param[in]  member_size Size in bytes of each element to be read.
 * @param[in]  member_num  Number of elements, each one with size of
 *                         member_size bytes.
 * @param[in]  file        File object.
 * @param[out] read_num    Total number of elements successfully read.
 *                         (optional)
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSFread(void* buffer, size_t member_size, size_t member_num,
                OSFile* file, size_t* read_num);

/**
 * @brief Sets the current position of the file.
 * @param[in] file    File object.
 * @param[in] offset  Binary files: Number of bytes to offset from seek_origin.
 *                    Text files: Either zero, or a value returned by OSFtell.
 * @param[in] seek_origin Position used as reference for the offset.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSFseek(OSFile* file, int64_t offset, OSFileSeekOrigin seek_origin);

/**
 * @brief Gets the current position of the file.
 * @param[in]  file    File object.
 * @param[out] offset  Pointer to the variable that receives the current
 *                     position.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSFtell(OSFile* file, int64_t* offset);

/**
 * @brief Return error status of the stream.
 * @param[in] file  File object.
 * @retval >0  IO error.
 * @retval 0   No error.
 * @retval <0  Fail. (error code)
 */
int32_t OSFerror(OSFile* file);

/**
 * @brief Return EOF status of the stream.
 * @param[in] file  File object.
 * @retval >0  EOF.
 * @retval 0   Not EOF.
 * @retval <0  Fail. (error code)
 */
int32_t OSFeof(OSFile* file);

/**
 * @brief Reset the status of the stream.
 * @param[in] file  File object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSFclearError(OSFile* file);

/**
 * @brief file flush of the stream.
 * @param[in] file  File object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSFflush(OSFile* file);

/**
 * @brief get binary size of the stream.
 * @param[in]  file File object.
 * @param[out] size File size.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSGetBinaryFileSize(OSFile* file, size_t* size);

/* Directory */
/**
 * @brief Directory path delimiter.
 */
#ifdef _WIN32
const char kDirectoryDelimiter = '\\';
#else
const char kDirectoryDelimiter = '/';
#endif

/**
 * @brief Make the directory
 * @param[in] (directory_path) Directory path.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSMakeDirectory(const char* directory_path);

/**
 * @brief Remove the directory
 * @param[in] (directory_path) Directory path.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSRemoveDirectory(const char* directory_path);

/**
 * @brief Get a list of regular files in the specified directory.
 * @param[in]  directory_path Path of the directory to scan.
 * @param[out] file_list      Pointer to the variable length array that receive
 *                            a list of file paths.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSGetRegularFileList(const std::string& directory_path,
                             std::vector<std::string>* file_list);

/**
 * @brief Get the value of the specified environment variable.
 * @param[in]  name        Name of the environment variable.
 * @param[out] environment Pointer to the variable that receives the value of
 *                         the environment variable.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSGetEnvironment(const std::string& name, std::string* environment);

/**
 * @brief Get the file name of the dynamic library.
 * @param[in] base  Base file name.
 * @param[out] name  Dynamic library file name.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSGetDynamicLibraryFileName(
    const std::string& base, std::string* name);

/* Thread */
/**
 * @brief Thread object.
 */
typedef struct OSThread OSThread;

/**
 * @brief Thread end result.
 */
typedef uintptr_t OSThreadResult;

/**
 * @brief Thread function pointer.
 */
typedef OSThreadResult (*OSThreadFunc)(void* argument);

/**
 * @brief Detached state of thread.
 */
enum OSThreadDetachState {
  kOSThreadJoinable = 0,  /**< Joinable state. (default) */
  kOSThreadDetached,      /**< Detached state. */
};

/**
 * @brief Priority of thread.
 */
enum OSThreadPriority {
  kOSThreadPriorityDefault = 0,  /**< Default priority. (Same as Normal) */
  kOSThreadPriorityIdle,         /**< Lower priority than Lowest. */
  kOSThreadPriorityLowest,       /**< Lowest priority. */
  kOSThreadPriorityBelowNormal,  /**< Priority between Lowest and Normal. */
  kOSThreadPriorityNormal,       /**< Normal priority. */
  kOSThreadPriorityAboveNormal,  /**< Priority between Highest and Normal. */
  kOSThreadPriorityHighest,      /**< Highest priority. */
};

/**
 * @brief Thread attributes.
 */
struct OSThreadAttribute {
  OSThreadDetachState detach_state;  /**< Detached state of thread */
  OSThreadPriority    priority;      /**< Priority of thread */
};

/**
 * @brief Create a new thread.
 * @param[out] thread          Pointer to the variable that receives the
 *                             thread object.
 * @param[in]  thread_func     Functions to be executed in new thread.
 * @param[in]  thread_argument Argument to be passed to a new thread.
 * @param[in]  thread_attr     Attributes for a new thread.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSCreateThread(OSThread** thread,
                       OSThreadFunc thread_func,
                       void* thread_argument,
                       const OSThreadAttribute* thread_attr);

/**
 * @brief Detach a thread.
 * @param[in] thread  Thread object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSDetachThread(OSThread* thread);

/**
 * @brief Join with a terminated thread.
 * @param[in]  thread  Thread object.
 * @param[out] result  Pointer to the variable that receives the thread end
 *                     result. (optional)
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSJoinThread(OSThread* thread, OSThreadResult* result);

/**
 * @brief Join with a terminated thread.
 * @param[in]  thread        Thread object.
 * @param[in]  nano_seconds  Timeout absolute time, in nanoseconds.
 * @param[out] result        Pointer to the variable that receives the thread
 *                           end result. (optional)
 * @return 0 is success. Negative value is fail. (error code)
 * @retval kErrorTimedOut When the low-order 1 byte is kErrorTimedOut,
 *                        it means timeout.
 */
int32_t OSTimedJoinThread(OSThread* thread,
                          uint64_t nano_seconds,
                          OSThreadResult* result);

/**
 * @brief Join with a terminated thread.
 * @param[in]  thread        Thread object.
 * @param[in]  nano_seconds  Timeout relative time, in nanoseconds.
 * @param[out] result        Pointer to the variable that receives the thread
 *                           end result. (optional)
 * @return 0 is success. Negative value is fail. (error code)
 * @retval kErrorTimedOut When the low-order 1 byte is kErrorTimedOut,
 *                        it means timeout.
 */
int32_t OSRelativeTimedJoinThread(OSThread* thread,
                                  uint64_t nano_seconds,
                                  OSThreadResult* result);

/**
 * @brief Set priority of a thread.
 * @param[in] thread    Thread object.
 * @param[in] priority  Thread priority.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSSetThreadPriority(OSThread* thread, OSThreadPriority priority);

/**
 * @brief Get priority of a thread.
 * @param[in]  thread    Thread object.
 * @param[out] priority  Thread priority.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSGetThreadPriority(OSThread* thread, OSThreadPriority* priority);

/**
 * @brief Get the current thread.
 * @param[out] thread  Pointer to the variable that receives the thread object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSGetCurrentThread(OSThread** thread);


/* Mutex exclusion */
/**
 * @brief Mutex(Mutual exclusion) object.
 */
typedef struct OSMutex OSMutex;

/**
 * @brief Create a mutex object.
 * @param[out] mutex  Pointer to the variable that receives the mutex object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSCreateMutex(OSMutex** mutex);

/**
 * @brief Destroy a mutex object.
 * @param[in] mutex  Mutex object to destroy.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSDestroyMutex(OSMutex* mutex);

/**
 * @brief Lock a mutex.
 * @param[in] mutex  Mutex object to lock.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSLockMutex(OSMutex* mutex);

/**
 * @brief Lock a mutex. (specify absolute time)
 * @param[in] mutex         Mutex object to lock.
 * @param[in] nano_seconds  Timeout absolute time, in nanoseconds.
 * @return 0 is success. Negative value is fail. (error code)
 * @retval kErrorTimedOut When the low-order 1 byte is kErrorTimedOut,
 *                        it means timeout.
 */
int32_t OSTimedLockMutex(OSMutex* mutex, uint64_t nano_seconds);

/**
 * @brief Lock a mutex. (specify relative time)
 * @param[in] mutex         Mutex object to lock.
 * @param[in] nano_seconds  Timeout relative time, in nanoseconds.
 * @return 0 is success. Negative value is fail. (error code)
 * @retval kErrorTimedOut When the low-order 1 byte is kErrorTimedOut,
 *                        it means timeout.
 */
int32_t OSRelativeTimedLockMutex(OSMutex* mutex, uint64_t nano_seconds);

/**
 * @brief Try to lock a mutex.
 * @param[in] mutex  Mutex object to lock.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSTryLockMutex(OSMutex* mutex);

/**
 * @brief Unlock a mutex.
 * @param[in] mutex  Mutex object to unlock.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSUnlockMutex(OSMutex* mutex);

/* Condition Variable */
/**
 * @brief Condition Variable object.
 */
typedef struct OSCond OSCond;

/**
 * @brief Create a condition variable.
 * @param[out] cond  Pointer to the variable that receives the condition
 *                   variable.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSCreateCond(OSCond** cond);

/**
 * @brief Destroy a condition variable.
 * @param[in] cond  Condition variable object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSDestroyCond(OSCond* cond);

/**
 * @brief Wait until notified.
 * @param[in] cond   Condition variable object.
 * @param[in] mutex  Mutex object that is currently locked by this thread.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSWaitCond(OSCond* cond, OSMutex* mutex);

/**
 * @brief Wait for timeout or until notified. (specify absolute time)
 * @param[in] cond   Condition variable object.
 * @param[in] mutex  Mutex object that is currently locked by this thread.
 * @param[in] nano_seconds  Timeout absolute time, in nanoseconds.
 * @return 0 is success. Negative value is fail. (error code)
 * @retval kErrorTimedOut When the low-order 1 byte is kErrorTimedOut,
 *                        it means timeout.
 */
int32_t OSTimedWaitCond(OSCond* cond,
                        OSMutex* mutex,
                        uint64_t nano_seconds);

/**
 * @brief Wait for timeout or until notified. (specify relative time)
 * @param[in] cond   Condition variable object.
 * @param[in] mutex  Mutex object that is currently locked by this thread.
 * @param[in] nano_seconds  Timeout relative time, in nanoseconds.
 * @return 0 is success. Negative value is fail. (error code)
 * @retval kErrorTimedOut When the low-order 1 byte is kErrorTimedOut,
 *                        it means timeout.
 */
int32_t OSRelativeTimedWaitCond(OSCond* cond,
                                OSMutex* mutex,
                                uint64_t nano_seconds);

/**
 * @brief Unblocks one of the threads waiting for the condition variable.
 * @param[in] cond  Condition variable object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSSignalCond(OSCond* cond);

/**
 * @brief Unblocks all threads waiting for the condition variable.
 * @param[in] cond  Condition variable object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSBroadcastCond(OSCond* cond);

/* Thread Sleep */
/**
 * @brief Sleep for the specified time.
 * @param[in] nano_seconds  Sleep time, in nanoseconds.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSSleep(uint64_t nano_seconds);


/* Socket */
/**
 * @brief Socket type.
 */
enum OSSocketType {
  kSocketTypeInetUdp,  /**< IPv4 UDP socket. */
  kSocketTypeInetTcp,  /**< IPv4 TCP socket. */
};

/**
 * @brief Socket shutdown option.
 * @see OSShutdownSocket
 */
enum OSShutdownOption {
  kShutdownReceive,  /**< Shutdown of receive operation. */
  kShutdownSend,     /**< Shutdown of send operation. */
  kShutdownBoth,     /**< Shutdown of send/receive operation. */
};

/**
 * @brief Socket object.
 */
typedef struct OSSocket OSSocket;

/**
 * @brief IPv4 address structure.
 */
struct OSSocketAddressInet {
  uint16_t port;     /**< Port number */
  uint32_t address;  /**< IP address */
};

/**
 * @brief Message structure for OSSendMsgSocket.
 */
struct OSSocketMessage {
  void* buffer;        /**< Pointer to the buffer. */
  size_t buffer_size;  /**< Size of the buffer. */
};

/**
 * @brief Corresponds to 0.0.0.0 of the IPv4 address. (INADDR_ANY)
 * Used in OSBindSocket function.
 */
const uint32_t kOSInAddrAny = 0x00000000;
/**
 * @brief Corresponds to 127.0.0.1 of the IPv4 address. (INADDR_LOOPBACK)
 */
const uint32_t kOSInAddrLoopback = 0x7f000001;

/**
 * @brief Create a socket.
 * @param[in]  socket_type Type of socket.
 * @param[out] socket      Pointer to the variable that receives the socket
 *                         object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSCreateSocket(OSSocketType socket_type, OSSocket** socket);

/**
 * @brief Disables send, receive, or both on a socket.
 * @param[in] socket  Socket object.
 * @param[in] option  Shutdown option.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSShutdownSocket(OSSocket* socket, OSShutdownOption option);

/**
 * @brief Destroy a socket.
 * @param[in] socket  Socket object.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSDestroySocket(OSSocket* socket);

/**
 * @brief Bind a name to a socket.
 * @param[in] socket  Socket object.
 * @param[in] address IP address to assign to the bound socket.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSBindSocket(OSSocket* socket, const OSSocketAddressInet& address);

/**
 * @brief Listen for connections on a socket.
 * @param[in] socket  Socket object.
 * @param[in] backlog The maximum length of the queue of pending connections.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSListenSocket(OSSocket* socket, int32_t backlog);

/**
 * @brief Accept a connection on a socket.
 * @param[in]  socket         Socket object.
 * @param[out] accept_socket  Pointer to the variable that receives the
 *                            accepted socket object.
 * @param[out] accept_address Pointer to the variable that receives the
 *                            address of the connection destination. (optional)
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSAcceptSocket(OSSocket* socket,
                       OSSocket** accept_socket,
                       OSSocketAddressInet* accept_address);

/**
 * @brief Initiate a connection on a socket.
 * @param[in] socket  Socket object.
 * @param[in] address IP address of the connection destination.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSConnectSocket(OSSocket* socket,
                        const OSSocketAddressInet& address);

/**
 * @brief Initiate a connection on a socket. (with timeout)
 * @param[in] socket  Socket object.
 * @param[in] address IP address of the connection destination.
 * @param[in] relative_timeout Timeout relative time, in nanoseconds.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSConnectSocket(OSSocket* socket,
                        const OSSocketAddressInet& address,
                        uint64_t relative_timeout);

/**
 * @brief Send a message on a socket.
 * @param[in]  socket      Socket object.
 * @param[in]  buffer      Pointer to the buffer.
 * @param[in]  buffer_size Buffer length in bytes.
 * @param[out] sent_size   Pointer to the variable that receives the size
 *                         sent. (optional)
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSSendSocket(OSSocket* socket,
                     const void* buffer,
                     size_t buffer_size,
                     size_t* sent_size);

/**
 * @brief Send a message on a socket.
 * @param[in]  socket       Socket object.
 * @param[in]  buffer       Pointer to the buffer.
 * @param[in]  buffer_size  Buffer length in bytes.
 * @param[in]  dest_address IP address of the destination. (optional)
 * @param[out] sent_size    Pointer to the variable that receives the size
 *                          sent. (optional)
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSSendToSocket(OSSocket* socket,
                       const void* buffer,
                       size_t buffer_size,
                       const OSSocketAddressInet* dest_address,
                       size_t* sent_size);

/**
 * @brief Concatenate multiple messages and send with socket.
 *
 * For unconnected DGRAM socket, specify dest_address.
 *
 * @param[in]  socket       Socket object.
 * @param[in]  messages     List of messages to concatenate.
 * @param[in]  dest_address IP address of the destination. (optional)
 * @param[out] sent_size    Pointer to the variable that receives the size
 *                          sent. (optional)
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSSendMsgSocket(OSSocket* socket,
                        const std::vector<OSSocketMessage>& messages,
                        const OSSocketAddressInet* dest_address,
                        size_t* sent_size);

/**
 * @brief Receive a message from a socket.
 * @param[in]  socket        Socket object.
 * @param[out] buffer        Pointer to the buffer to receive the incoming
 *                           data.
 * @param[in]  buffer_size   Buffer length in bytes.
 * @param[out] received_size Pointer to the variable that receives the size
 *                           received. (optional)
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSRecvSocket(OSSocket* socket,
                     void* buffer,
                     size_t buffer_size,
                     size_t* received_size);

/**
 * @brief Receive a message from a socket.
 * @param[in]  socket         Socket object.
 * @param[out] buffer         Pointer to the buffer to receive the incoming
 *                            data.
 * @param[in]  buffer_size    Buffer length in bytes.
 * @param[out] source_address Pointer to the variable that receives the
 *                            IP address of the source. (optional)
 * @param[out] received_size  Pointer to the variable that receives the size
 *                            received. (optional)
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSRecvFromSocket(OSSocket* socket,
                         void* buffer,
                         size_t buffer_size,
                         OSSocketAddressInet* source_address,
                         size_t* received_size);

/**
 * @brief Determine the state of one or more sockets and perform
 *        synchronous I/O.
 * @param[in,out] read_sockets   Pointer to list of sockets to be checked for
 *                               readability. (optional)
 * @param[in,out] write_sockets  Pointer to list of sockets to be checked for
 *                               writability. (optional)
 * @param[in,out] except_sockets Pointer to list of sockets to be checked for
 *                               errors. (optional)
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSSelectSocket(
    std::vector<OSSocket*>* read_sockets,
    std::vector<OSSocket*>* write_sockets,
    std::vector<OSSocket*>* except_sockets);

/**
 * @brief Determine the state of one or more sockets and perform
 *        synchronous I/O.
 * @param[in,out] read_sockets   Pointer to list of sockets to be checked for
 *                               readability. (optional)
 * @param[in,out] write_sockets  Pointer to list of sockets to be checked for
 *                               writability. (optional)
 * @param[in,out] except_sockets Pointer to list of sockets to be checked for
 *                               errors. (optional)
 * @param[in]     nano_seconds   Timeout relative time, in nanoseconds.
 * @return 0 is success. Negative value is fail. (error code)
 * @retval kErrorTimedOut When the low-order 1 byte is kErrorTimedOut,
 *                        it means timeout.
 */
int32_t OSRelativeTimedSelectSocket(
    std::vector<OSSocket*>* read_sockets,
    std::vector<OSSocket*>* write_sockets,
    std::vector<OSSocket*>* except_sockets,
    uint64_t nano_seconds);

/**
 * @brief Determine the state of one or more sockets and perform
 *        synchronous I/O.
 * @param[in,out] read_sockets   Pointer to list of sockets to be checked for
 *                               readability. (optional)
 * @param[in,out] write_sockets  Pointer to list of sockets to be checked for
 *                               writability. (optional)
 * @param[in,out] except_sockets Pointer to list of sockets to be checked for
 *                               errors. (optional)
 * @param[in]     nano_seconds   Timeout absolute time, in nanoseconds.
 * @return 0 is success. Negative value is fail. (error code)
 * @retval kErrorTimedOut When the low-order 1 byte is kErrorTimedOut,
 *                        it means timeout.
 */
int32_t OSTimedSelectSocket(
    std::vector<OSSocket*>* read_sockets,
    std::vector<OSSocket*>* write_sockets,
    std::vector<OSSocket*>* except_sockets,
    uint64_t nano_seconds);

/**
 * @brief Convert uint32_t from host to network byte order. (which is
 *        big-endian)
 * @param[in] hostlong  32-bit number in host byte order.
 * @return 32-bit number in network byte order.
 */
uint32_t OSHtonl(uint32_t hostlong);

/**
 * @brief Convert uint16_t from host to network byte order. (which is
 *        big-endian).
 * @param[in] hostshort  16-bit number in host byte order.
 * @return 16-bit number in network byte order.
 */
uint16_t OSHtons(uint16_t hostshort);

/**
 * @brief Convert uint32_t from network to host byte order.
 * @param[in] netlong  32-bit number in network byte order.
 * @return 32-bit number in host byte order.
 */
uint32_t OSNtohl(uint32_t netlong);

/**
 * @brief Convert uint16_t from network to host byte order.
 * @param[in] netshort  16-bit number in network byte order.
 * @return 16-bit number in host byte order.
 */
uint16_t OSNtohs(uint16_t netshort);

/**
 * @brief Convert a string IPv4 address to binary data in network byte order.
 * @param[in]  source_address      String IPv4 address.
 * @param[out] destination_address IPv4 address in network byte order.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSInetAton(const char* source_address,
                   uint32_t* destination_address);

/**
 * @brief Convert the binary data given in network byte order, to a string
 *        IPv4 address.
 * @param[in]  source_address      IPv4 address in network byte order.
 * @param[out] destination_address Pointer to the buffer to receive the string
 *                                 IPv4 address.
 * @param[in]  destination_size    Buffer length in bytes.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSInetNtoa(uint32_t source_address,
                   char* destination_address,
                   size_t destination_size);

/**
 * @brief Set the send buffer size.
 * @param[in] socket      Socket object.
 * @param[in] buffer_size Buffer size in bytes.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSSetSocketSendBufferSize(OSSocket* socket, uint32_t buffer_size);

/**
 * @brief Get the send buffer size.
 * @param[in]  socket      Socket object.
 * @param[out] buffer_size Pointer to the variable that receives the buffer
 *                         size in bytes.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSGetSocketSendBufferSize(OSSocket* socket, uint32_t* buffer_size);

/**
 * @brief Set the receive buffer size.
 * @param[in] socket      Socket object.
 * @param[in] buffer_size Buffer size in bytes.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSSetSocketRecvBufferSize(OSSocket* socket, uint32_t buffer_size);

/**
 * @brief Get the receive buffer size.
 * @param[in]  socket      Socket object.
 * @param[out] buffer_size Pointer to the variable that receives the buffer
 *                         size in bytes.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSGetSocketRecvBufferSize(OSSocket* socket, uint32_t* buffer_size);

/**
 * @brief Set rules for reuse of bind address.
 *
 * It must be called before OSBindSocket.
 *
 * @param[in] socket  Socket object.
 * @param[in] flag    true: Reuse is enabled.
 *                    false: Reuse is disabled.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSSetSocketReuseAddr(OSSocket* socket, bool flag);

/**
 * @brief Set the socket option for TCP_NODELAY.
 *
 * @param[in] socket  Socket object.
 * @param[in] enabled true: Enabling TCP_NODELAY.
 *                    false: Disabling TCP_NODELAY.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSSetSocketTcpNoDelay(OSSocket* socket, bool enabled);

/**
 * @brief Get a list of IPv4 addresses of the terminal.
 *
 * @param[out] addr_list  Pointer to a variable that
 *                        stores a list of IPv4 addresses.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSGetInetAddressList(std::vector<OSSocketAddressInet>* addr_list);

/* Memory */
/**
 * @brief Search character in block of memory.
 * @param[in] source  Pointer to the block of memory where the search is
 *                    performed.
 * @param[in] character  Character to be searched.
 * @param[in] length  Length of bytes to be analyzed.
 * @return A pointer to the first occurrence of value in the block of memory
 *         pointed by source.
 *         If the character is not found, the function returns a null pointer.
 */
const void* OSMemchr(const void* source, int32_t character, size_t length);
void* OSMemchr(void* source, int32_t character, size_t length);

/**
 * @brief Compare two blocks of memory.
 * @param[in] source1  Pointer to block of memory.
 * @param[in] source2  Pointer to block of memory.
 * @param[in] length   Length of bytes to compare.
 * @retval >0  source1 larger than source2.
 * @retval 0   source1 identical to source2.
 * @retval <0  source1 smaller than source2.
 */
int32_t OSMemcmp(const void* source1, const void* source2, size_t length);

/**
 * @brief Copy block of memory.
 * @param[out] dest      Pointer to the destination array where the content is
 *                       to be copied.
 * @param[in]  dest_size Size of the destination array.
 * @param[in]  source    Pointer to the source of data to be copied.
 * @param[in]  count     Number of bytes to copy.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSMemcpy(void* dest, size_t dest_size,
                 const void* source, size_t count);

/**
 * @brief Move block of memory.
 * @param[out] dest      Pointer to the destination array where the content is
 *                       to be copied.
 * @param[in]  dest_size Size of the destination array.
 * @param[in]  source    Pointer to the source of data to be copied.
 * @param[in]  count     Number of bytes to copy.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSMemmove(void* dest, size_t dest_size,
                  const void* source, size_t count);

/**
 * @brief Fill block of memory.
 * @param[out] buffer    Pointer to the block of memory to fill.
 * @param[in]  character Value to be set.
 * @param[in]  length    Number of bytes to be set.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSMemset(void* buffer, int32_t character, size_t length);

/**
 * @brief Allocate memory block.
 * @param[in] length  Length of the memory block, in bytes.
 * @return On success, it returns a pointer to the memory block.
 *         If it failed, it returns a null pointer.
 */
void* OSMalloc(size_t length);

/**
 * @brief Deallocate memory block.
 * @param[in] ptr Pointer to a memory block previously allocated with OSMalloc.
 * @return Always returns zero.
 */
int32_t OSFree(void* ptr);

/* dlloader */
/**
 * @brief Handle of dynamic library.
 */
typedef struct OSDlHandle OSDlHandle;

/**
 * @brief Load a dynamic library.
 * @param[in]  library_name  Library path name to load.
 * @param[out] dlhandle      Pointer to the variable that receives the handle
 *                           of dynamic library.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSDlLoad(const char* library_name, OSDlHandle** dlhandle);

/**
 * @brief Load a dynamic library. (with error message)
 * @param[in]  library_name  Library path name to load.
 * @param[out] dlhandle      Pointer to the variable that receives the handle
 *                           of dynamic library.
 * @param[out] error_msg     Error message.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSDlLoad(const char* library_name,
                 OSDlHandle** dlhandle, std::string* error_msg);

/**
 * @brief Get a function pointer from a dynamic library.
 * @param[in]  handle         Handle of dynamic library.
 * @param[in]  function_name  Function name.
 * @param[out] func_ptr       Pointer to the variable that receives the
 *                            function pointer.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSDlGetFuncPtr(OSDlHandle* handle, const char* function_name,
                       void** func_ptr);

/**
 * @brief Get a function pointer from a dynamic library.
 *        (with error message)
 * @param[in]  handle         Handle of dynamic library.
 * @param[in]  function_name  Function name.
 * @param[out] func_ptr       Pointer to the variable that receives the
 *                            function pointer.
 * @param[out] error_msg      Error message.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSDlGetFuncPtr(OSDlHandle* handle, const char* function_name,
                       void** func_ptr, std::string* error_msg);

/**
 * @brief Unload a dynamic library.
 * @param[in] handle  Handle of dynamic library.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSDlFree(OSDlHandle* handle);

/**
 * @brief Unload a dynamic library. (with error message)
 * @param[in]  handle       Handle of dynamic library.
 * @param[out] error_msg    Error message.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSDlFree(OSDlHandle* handle, std::string* error_msg);

/* Math */
/**
 * @brief Calculates the absolute value of a floating-point number.
 * @param[in] num  Floating-point number.
 * @return The absolute value. There is no error return.
 */
double OSFabs(double num);

/* Time*/
/**
 * @brief Get current time.
 * Time in nanoseconds since the epoch time(1970-1-1 00:00:00 UTC).
 * @param[out] nano_seconds  Pointer to the variable that receives the current
 *                           time. (in nanoseconds)
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSGetTime(uint64_t* nano_seconds);

/**
 * @brief Time structure.
 */
typedef struct {
  uint16_t year;          /**< Years */
  uint8_t  month;         /**< Months (1-12) */
  uint8_t  day_of_week;   /**< Day of the week (0-6: 0=Sunday, 6=Saturday) */
  uint8_t  day;           /**< Day of the month (1-31) */
  uint8_t  hour;          /**< Hours (0-23) */
  uint8_t  minute;        /**< Minutes (0-59) */
  uint8_t  second;        /**< Seconds (0-60, Generally 0-59) */
  uint16_t milli_second;  /**< Milliseconds (0-999) */
} OSSystemTime;

/**
 * @brief Get current time.
 * The function corrects for the timezone.
 * @param[out] current_time  Pointer to the variable that receives the current
 *                           time. (OSSystemTime)
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSGetLocalTime(OSSystemTime* current_time);

/* Rand */
const uint16_t kOSRandMax = 0x7FFF;  /**< @brief Random maximum value */
const uint16_t kOSRandMin = 0x0;     /**< @brief Random minimum value */

/**
 * @brief Generate a random number.
 * Returns a random value between kOSRandMin and kOSRandMax.
 * @param[out] random_val  Pointer to the variable that receives the random
 *                         value.
 * @return 0 is success. Negative value is fail. (error code)
 */
int32_t OSRand(uint16_t* random_val);

/* Timer */
/**
 * @brief Timer identifier.
 */
typedef struct OSTimerId OSTimerId;

/**
 * @brief Timer class.
 */
class OSTimer {
 public:
  /**
   * @brief OSTimer destructor.
   */
  virtual ~OSTimer();

  /**
   * @brief Function executed when the timer expires.
   * Override and implement.
   */
  virtual void TimerHandler() = 0;

  /**
   * @brief Start the timer.
   * @param[in] first_milli_seconds    First interval, in milliseconds.
   * @param[in] interval_milli_seconds Second and subsequent intervals,
   *                                   in milliseconds.
   * @return 0 is success. Negative value is fail. (error code)
   */
  int32_t StartTimer(uint64_t first_milli_seconds,
                     uint64_t interval_milli_seconds);

  /**
   * @brief Stop the timer.
   * @return 0 is success. Negative value is fail. (error code)
   */
  int32_t StopTimer();

 protected:
  /**
   * @brief OSTimer constructor.
   */
  OSTimer();

 private:
  OSTimerId* timer_id_;  /**< Timer identifier */

  OSMutex* mutex_;
};

/* XML Parser */
/**
 * @brief XML node type.
 */
enum OSXmlNodeType{
  kOSXmlUnsupportedNode = 0,  /**< Unsupported node */
  kOSXmlElementNode,          /**< Element node */
  kOSXmlElementEnd,           /**< End of element */
};

/**
 * @brief XML reader to use with parser.
 */
typedef struct OSXmlReader OSXmlReader;

/**
 * @brief Xml parser class.
 */
class OSXmlParser {
 public:
  /**
   * @brief OSXmlParser constructor.
   */
  OSXmlParser();

  /**
   * @brief OSXmlParser destructor.
   */
  ~OSXmlParser();

  /**
   * @brief Open a XML file.
   * Other files can not be opened until closed.
   * @param[in] file_name  Path of the file to open.
   * @return 0 is success. Negative value is fail. (error code)
   */
  int32_t Open(const std::string& file_name);

  /**
   * @brief Close a XML file.
   * @return 0 is success. Negative value is fail. (error code)
   */
  int32_t Close();

  /**
   * @brief Parse the XML file one line, and get the node type.
   * @param[out] type  Pointer to the variable that receives the node type.
   * @return 0 is success. Negative value is fail. (error code)
   */
  int32_t Parse(OSXmlNodeType* type);

  /**
   * @brief Get a attribute from the current node.
   * @param[in]  name      Attribute name.
   * @param[out] attribute Pointer to the string that receives the attribute
   *                       value.
   * @return 0 is success. Negative value is fail. (error code)
   */
  int32_t GetAttribute(const std::string& name, std::string* attribute);


  /**
   * @brief Get the element name from the current node.
   * @param[out] element Pointer to the string that receives the element name.
   * @return 0 is success. Negative value is fail. (error code)
   */
  int32_t GetElement(std::string* element);

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
  int32_t GetDepth(uint32_t* depth);

 private:
  OSXmlReader* reader_;
};

typedef struct OSXmlWriter OSXmlWriter;

class OSXmlCreator {
 public:
   /**
    * @brief OSXmlCreator constructor.
    */
  OSXmlCreator();

  /**
   * @brief OSXmlCreator destructor.
   */
  ~OSXmlCreator();

  /**
   * @brief Open a XML file.
   * @param[in] filename Path of the file to open.
   * @return 0 is success. Negative value is fail. (error code)
   */
  int32_t Open(const std::string& file_name);

  /**
   * @brief Close a XML file.
   * @return 0 is success. Negative value is fail. (error code)
   */
  int32_t Close();

  /**
   * @brief Writing comment.
   * @param[in] comment Contents of the Comment to be written.
   * @return 0 is success. Negative value is fail. (error code)
   */
  int32_t WriteComment(const std::string& comment);

  /**
   * @brief Start writing element.
   * @param[in] name Name of the element to be written.
   * @return 0 is success. Negative value is fail. (error code)
   */
  int32_t WriteStartElement(const std::string& name);

  /**
   * @brief End the element currently being written.
   * @return 0 is success. Negative value is fail. (error code)
   */
  int32_t WriteEndElement();

  /**
   * @brief Add attributes to the target tag.
   * @param[in] name Name of the attribute to be written.
   * @param[in] attribute The value of the attribute to be written.
   * @return 0 is success. Negative value is fail. (error code)
   */
  int32_t WriteAttribute(const std::string& name, const std::string& attribute);

 private:
  OSXmlWriter* writer_;
};

/**
 * @brief Fast exclusive lock class.
 */
class OSExclusiveLock {
 public:
  OSExclusiveLock();
  ~OSExclusiveLock();

  /**
   * @brief Exclusive lock.
   */
  void Lock();
  /**
   * @brief Exclusive unlock.
   */
  void Unlock();

 private:
  OSExclusiveLock(const OSExclusiveLock&);  // = delete;
  OSExclusiveLock& operator =(const OSExclusiveLock&);  // = delete;

 private:
  void* lock_object_;
};

}  //  namespace osal
}  //  namespace senscord

#endif  // SENSCORD_OSAL_H_
