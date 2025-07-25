# SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

"""
SensCord C API accessor module.
"""

from __future__ import absolute_import

import ctypes
import os
import platform

from senscord import errors
from senscord import serialize
from senscord import _utils
from senscord import senscord_types as _types


# Length of the version string.
VERSION_LENGTH = 256
# Length of the user data.
USER_DATA_LENGTH = 256
# Length of the version string.
STREAM_KEY_LENGTH = 64
# Length of the stream arguments.
STREAM_ARGUMENT_LENGTH = 32
# Length of the stream argument name string.
STREAM_ARGUMENT_NAME_LENGTH = 32
# Length of the stream argument value string.
STREAM_ARGUMENT_VALUE_LENGTH = 256


class SensCordC(object):
    """SensCord C API."""
    _senscord_dll = None
    _osal = None
    _senscord_py_dll = None

    @staticmethod
    def _to_bytes(value):
        if isinstance(value, str):
            return value.encode()
        return value

    @staticmethod
    def _convert_senscord_version(src):
        """Convert senscord version.

        :param src: Source senscord version.
        :type src: SensCordVersion
        :return: Converted version.
        :rtype: SensCordVersion
        """
        version = _types.SensCordVersion()
        version.senscord_version = SensCordC._convert_version(src.senscord_version)
        version.project_version = SensCordC._convert_version(src.project_version)
        for i in range(src.stream_count):
            src_stream = src.stream_versions[i]
            stream = SensCordC._convert_stream_version(src_stream)
            version.stream_versions[src_stream.stream_key] = stream
        for i in range(src.server_count):
            src_server = src.server_versions[i]
            server = SensCordC._convert_senscord_version(src_server)
            version.server_versions[src.destination_id] = server
        return version

    @staticmethod
    def _convert_stream_version(src):
        """Convert stream version.

        :param src: Source stream version.
        :type src: StreamVersion
        :return: Converted version.
        :rtype: StreamVersion
        """
        version = _types.StreamVersion()
        version.stream_version = SensCordC._convert_version(src.stream_version)
        for index in range(src.linkage_count):
            element = SensCordC._convert_version(src.linkage_versions[index])
            version.linkage_versions.append(element)
        version.destination_id = src.destination_id
        return version

    @staticmethod
    def _convert_version(src):
        """Convert version.

        :param src: Source version.
        :type src: Version
        :return: Converted version.
        :rtype: Version
        """
        version = _types.Version()
        version.name = src.name
        version.major = src.major
        version.minor = src.minor
        version.patch = src.patch
        version.description = src.description
        return version

    @staticmethod
    def _convert_open_stream_setting(src):
        """Convert OpenStreamSetting(Py to C).

        :param src: Source strucrure.
        :type src: OpenStreamSetting
        :return: Converted OpenStreamSetting.
        :rtype: OpenStreamSetting
        """
        setting = OpenStreamSetting()
        setting.frame_buffering = src.frame_buffering
        if len(src.arguments) > STREAM_ARGUMENT_LENGTH:
            raise errors.OperationError('count of arguments exceeded.')
        setting.arguents_count = 0
        for key, value in src.arguments.items():
            setting.arguents[setting.arguents_count].name = SensCordC._to_bytes(key)
            setting.arguents[setting.arguents_count].value = SensCordC._to_bytes(value)
            setting.arguents_count += 1
        return setting

    @staticmethod
    def _make_link_lib_path(senscord_path, lib_name):
        abs_path = os.path.abspath(
            os.path.join(senscord_path, os.pardir, lib_name))
        if not os.path.isfile(abs_path):
            raise errors.OperationError(abs_path + ' is not found.')
        return abs_path

    @staticmethod
    def load_dll(path):
        """Load SensCord library.

        :param path: Path of SensCord library.
        :type path: str
        """
        if SensCordC._senscord_dll is not None:
            return

        if platform.system() == 'Windows':
            # senscord_osal
            SensCordC._osal = ctypes.CDLL(
                SensCordC._make_link_lib_path(path, 'senscord_osal.dll'))
            # senscord
            SensCordC._senscord_dll = ctypes.CDLL(path)
            # senscord_python
            SensCordC._senscord_py_dll = ctypes.CDLL(
                SensCordC._make_link_lib_path(path, 'senscord_python.dll'))
        elif platform.system() == 'Darwin':
            # senscord
            SensCordC._senscord_dll = ctypes.CDLL(path)
            # senscord_python
            SensCordC._senscord_py_dll = ctypes.CDLL(
                SensCordC._make_link_lib_path(path, 'senscord_python.bundle'))
        else:
            # senscord
            SensCordC._senscord_dll = ctypes.CDLL(path)
            # senscord_python
            SensCordC._senscord_py_dll = ctypes.CDLL(
                SensCordC._make_link_lib_path(path, 'libsenscord_python.so'))

    @staticmethod
    def core_init():
        """Initialize Core, called at once.

        :return: Core handle.
        :rtype: long
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of core_init() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_core_init
        func.restype = ctypes.c_int32
        func.argtypes = [ctypes.POINTER(ctypes.c_uint64)]

        core_handle = ctypes.c_uint64()
        ret = func(ctypes.byref(core_handle))
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())
        return core_handle.value

    @staticmethod
    def core_exit(core_handle):
        """Finalize Core and close all opened streams.

        :param core_handle: Core handle.
        :type core_handle: long
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of core_exit() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_core_exit
        func.restype = ctypes.c_int32
        func.argtypes = [ctypes.c_uint64]

        ret = func(core_handle)
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())

    @staticmethod
    def core_get_stream_count(core_handle):
        """Get count of supported streams list.

        :param core_handle: Core handle.
        :type core_handle: long
        :return: Count of stream list.
        :rtype: int
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of core_get_stream_count() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_core_get_stream_count
        func.restype = ctypes.c_int32
        func.argtypes = [ctypes.c_uint64, ctypes.POINTER(ctypes.c_uint32)]

        count = ctypes.c_uint32()
        ret = func(core_handle, ctypes.byref(count))
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())
        return count.value

    @staticmethod
    def core_get_stream_info(core_handle, index):
        """Get supported stream information.

        :param core_handle: Core handle.
        :type core_handle: long
        :param index: Index of stream list.
        :type index: int
        :return: Stream information.
        :rtype: StreamTypeInfo
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of core_get_stream_info() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_core_get_stream_info
        func.restype = ctypes.c_int32
        func.argtypes = [
            ctypes.c_uint64,
            ctypes.c_uint32,
            ctypes.POINTER(StreamTypeInfo)
        ]

        stream_info = StreamTypeInfo()
        ret = func(core_handle, index, ctypes.byref(stream_info))
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())
        return stream_info

    @staticmethod
    def core_get_opened_stream_count(core_handle, stream_key):
        """Get count of opened stream.

        :param core_handle: Core handle.
        :type core_handle: long
        :param stream_key: Stream key.
        :type stream_key: bytes or str
        :return: Count of opened stream.
        :rtype: int
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of core_get_opened_stream_count() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_core_get_opened_stream_count
        func.restype = ctypes.c_int32
        func.argtypes = [
            ctypes.c_uint64,
            ctypes.c_char_p,
            ctypes.POINTER(ctypes.c_uint32)
        ]

        stream_key = SensCordC._to_bytes(stream_key)
        count = ctypes.c_uint32()
        ret = func(core_handle, stream_key, ctypes.byref(count))
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())
        return count.value

    @staticmethod
    def core_get_version(core_handle):
        """Get the version of this core library.

        :param core_handle: Core handle.
        :type core_handle: long
        :return: Version of this core library.
        :rtype: VersionProperty
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of core_get_version() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_core_get_version
        func.restype = ctypes.c_int32
        func.argtypes = [
            ctypes.c_uint64,
            ctypes.POINTER(SensCordVersion),
        ]

        tmp_version = SensCordVersion()
        ret = func(core_handle, ctypes.byref(tmp_version))
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())
        version = SensCordC._convert_senscord_version(tmp_version)
        return version

    @staticmethod
    def core_open_stream(core_handle, stream_key):
        """Open the new stream from key.

        :param core_handle: Core handle.
        :type core_handle: long
        :param stream_key: Stream key.
        :type stream_key: bytes or str
        :return: Stream handle.
        :rtype: long
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of core_open_stream() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_core_open_stream
        func.restype = ctypes.c_int32
        func.argtypes = [
            ctypes.c_uint64,
            ctypes.c_char_p,
            ctypes.POINTER(ctypes.c_uint64)
        ]

        stream_key = SensCordC._to_bytes(stream_key)
        stream_handle = ctypes.c_uint64()
        ret = func(core_handle, stream_key, ctypes.byref(stream_handle))
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())
        return stream_handle.value

    @staticmethod
    def core_open_stream_with_setting(core_handle, stream_key, setting):
        """Open the new stream from key and specified config.

        :param core_handle: Core handle.
        :type core_handle: long
        :param stream_key: Stream key.
        :type stream_key: bytes or str
        :param setting: Config to open stream.
        :type setting: OpenStreamSetting
        :return: Stream handle.
        :rtype: long
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of core_open_stream_with_setting() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_core_open_stream_with_setting
        func.restype = ctypes.c_int32
        func.argtypes = [
            ctypes.c_uint64,
            ctypes.c_char_p,
            ctypes.POINTER(OpenStreamSetting),
            ctypes.POINTER(ctypes.c_uint64)
        ]

        tmp_setting = SensCordC._convert_open_stream_setting(setting)
        stream_key = SensCordC._to_bytes(stream_key)
        stream_handle = ctypes.c_uint64()
        ret = func(core_handle, stream_key, tmp_setting,
                   ctypes.byref(stream_handle))
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())
        return stream_handle.value

    @staticmethod
    def core_close_stream(core_handle, stream_handle):
        """Close the opened stream.

        :param core_handle: Core handle.
        :type core_handle: long
        :param stream_handle: Stream handle.
        :type stream_handle: long
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of core_close_stream() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_core_close_stream
        func.restype = ctypes.c_int32
        func.argtypes = [ctypes.c_uint64, ctypes.c_uint64]

        ret = func(core_handle, stream_handle)
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())

    @staticmethod
    def stream_start(stream_handle):
        """Start stream.

        :param stream_handle: Stream handle.
        :type stream_handle: long
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of stream_start() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_stream_start
        func.restype = ctypes.c_int32
        func.argtypes = [ctypes.c_uint64]

        ret = func(stream_handle)
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())

    @staticmethod
    def stream_stop(stream_handle):
        """Stop stream.

        :param stream_handle: Stream handle.
        :type stream_handle: long
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of stream_stop() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_stream_stop
        func.restype = ctypes.c_int32
        func.argtypes = [ctypes.c_uint64]

        ret = func(stream_handle)
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())

    @staticmethod
    def stream_get_frame(stream_handle, timeout_msec):
        """Get the received frame.

        :param stream_handle: Stream handle.
        :type stream_handle: long
        :param timeout_msec: Time of wait msec if no received.
        :type timeout_msec: int
        :return: Frame handle.
        :rtype: long
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of stream_get_frame() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_stream_get_frame
        func.restype = ctypes.c_int32
        func.argtypes = [
            ctypes.c_uint64,
            ctypes.POINTER(ctypes.c_uint64),
            ctypes.c_int
        ]

        frame_handle = ctypes.c_uint64()
        ret = func(stream_handle, ctypes.byref(frame_handle), timeout_msec)
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())
        return frame_handle.value

    @staticmethod
    def stream_release_frame(stream_handle, frame_handle):
        """Release the gotten frame.

        :param stream_handle: Stream handle.
        :type stream_handle: long
        :param frame_handle: Frame handle.
        :type frame_handle: long
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of stream_release_frame() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_stream_release_frame
        func.restype = ctypes.c_int32
        func.argtypes = [ctypes.c_uint64, ctypes.c_uint64]

        ret = func(stream_handle, frame_handle)
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())

    @staticmethod
    def stream_release_frame_unused(stream_handle, frame_handle):
        """Release the gotten frame.

        Use this function if you do not refer to the raw data of the channel.

        :param stream_handle: Stream handle.
        :type stream_handle: long
        :param frame_handle: Frame handle.
        :type frame_handle: long
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of stream_release_frame() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_stream_release_frame_unused
        func.restype = ctypes.c_int32
        func.argtypes = [ctypes.c_uint64, ctypes.c_uint64]

        ret = func(stream_handle, frame_handle)
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())

    @staticmethod
    def stream_clear_frames(stream_handle):
        """Clear frames have not gotten.

        :param stream_handle: Stream handle.
        :type stream_handle: long
        :return: Number of cleared frames.
        :rtype: int
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of stream_clear_frames() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_stream_clear_frames
        func.restype = ctypes.c_int32
        func.argtypes = [ctypes.c_uint64, ctypes.POINTER(ctypes.c_int32)]

        clear_count = ctypes.c_int32()
        ret = func(stream_handle, ctypes.byref(clear_count))
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())
        return clear_count.value

    @staticmethod
    def stream_get_property(stream_handle, property_key, property_value):
        """Get the property.

        :param stream_handle: Stream handle.
        :type stream_handle: long
        :param property_key: Property key.
        :type property_key: bytes or str
        :param property_value: Property value.
        :type property_value: ctypes.Structure or bytearray
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of stream_get_property() function.
        :raise: TypeError: Unsupported type.
        """
        if SensCordC._senscord_py_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_py_dll.senscord_py_stream_get_serialized_property
        func.restype = ctypes.c_int32
        func.argtypes = [
            ctypes.c_uint64,
            ctypes.c_char_p,
            ctypes.py_object
        ]

        property_key = SensCordC._to_bytes(property_key)

        if isinstance(property_value, serialize.Serializable):
            value = bytearray(property_value.to_bytes())
        elif isinstance(property_value, bytearray):
            value = property_value
        else:
            raise TypeError('Unsupported type. (property_value)')

        ret = func(stream_handle, property_key, value)
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())

        if isinstance(property_value, serialize.Serializable):
            decoder = serialize.Decoder(value)
            property_value.decode(decoder)

    @staticmethod
    def stream_set_property(stream_handle, property_key, property_value):
        """Set the property with key.

        :param stream_handle: Stream handle.
        :type stream_handle: long
        :param property_key: Property key.
        :type property_key: bytes or str
        :param property_value: Property value.
        :type property_value: ctypes.Structure or bytearray
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of stream_set_property() function.
        :raise: TypeError: Unsupported type.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_stream_set_serialized_property
        func.restype = ctypes.c_int32
        func.argtypes = [
            ctypes.c_uint64,
            ctypes.c_char_p,
            ctypes.c_void_p,
            ctypes.c_size_t
        ]

        property_key = SensCordC._to_bytes(property_key)
        if property_value is None:
            null_ptr = ctypes.c_void_p()
            ret = func(stream_handle, property_key, null_ptr, 0)
            if ret != 0:
                raise errors.ApiError(SensCordC.get_last_error())
            return

        if isinstance(property_value, serialize.Serializable):
            tmp = property_value.to_bytes()
            value_size = len(tmp)
            value = (ctypes.c_ubyte * value_size).from_buffer_copy(tmp)
        elif isinstance(property_value, bytearray):
            value_size = len(property_value)
            value = (ctypes.c_ubyte * value_size).from_buffer(property_value)
        else:
            raise TypeError('Unsupported type. (property_value)')

        ret = func(stream_handle, property_key, ctypes.byref(value),
                   value_size)
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())

    @staticmethod
    def stream_get_userdata_property(stream_handle, property_value):
        """Get the user data property.

        :param stream_handle: Stream handle.
        :type stream_handle: long
        :param property_value: Property value.
        :type property_value: ctypes.Structure or bytearray
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of stream_get_userdata_property() function.
        :raise: TypeError: Unsupported type.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_stream_get_userdata_property
        func.restype = ctypes.c_int32
        func.argtypes = [ctypes.c_uint64, ctypes.c_void_p, ctypes.c_size_t]

        if isinstance(property_value, serialize.Serializable):
            value_size = USER_DATA_LENGTH
            value = (ctypes.c_ubyte * value_size)()
        elif isinstance(property_value, bytearray):
            value_size = len(property_value)
            value = (ctypes.c_ubyte * value_size).from_buffer(property_value)
        elif isinstance(property_value, (ctypes.Structure, ctypes.Array)):
            value_size = ctypes.sizeof(property_value)
            value = property_value
        else:
            raise TypeError('Unsupported type. (property_value)')

        ret = func(stream_handle, ctypes.byref(value), value_size)
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())

        if isinstance(property_value, serialize.Serializable):
            decoder = serialize.Decoder(bytearray(value))
            property_value.decode(decoder)

    @staticmethod
    def stream_set_userdata_property(stream_handle, property_value):
        """Set the user data property.

        :param stream_handle: Stream handle.
        :type stream_handle: long
        :param property_value: Property value.
        :type property_value: ctypes.Structure or bytearray
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of stream_set_userdata_property() function.
        :raise: TypeError: Unsupported type.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_stream_set_userdata_property
        func.restype = ctypes.c_int32
        func.argtypes = [ctypes.c_uint64, ctypes.c_void_p, ctypes.c_size_t]

        if property_value is None:
            null_ptr = ctypes.c_void_p()
            ret = func(stream_handle, null_ptr, 0)
            if ret != 0:
                raise errors.ApiError(SensCordC.get_last_error())
            return

        if isinstance(property_value, serialize.Serializable):
            tmp = property_value.to_bytes()
            value_size = len(tmp)
            value = (ctypes.c_ubyte * value_size).from_buffer_copy(tmp)
        elif isinstance(property_value, bytearray):
            value_size = len(property_value)
            value = (ctypes.c_ubyte * value_size).from_buffer(property_value)
        elif isinstance(property_value, (ctypes.Structure, ctypes.Array)):
            value_size = ctypes.sizeof(property_value)
            value = property_value
        else:
            raise TypeError('Unsupported type. (property_value)')

        ret = func(stream_handle, ctypes.byref(value), value_size)
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())

    @staticmethod
    def stream_get_property_count(stream_handle):
        """Get the count of supported property key on this stream.

        :param stream_handle: Stream handle.
        :type stream_handle: long
        :return: Count of property list.
        :rtype: int
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of stream_get_property_count() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_stream_get_property_count
        func.restype = ctypes.c_int32
        func.argtypes = [ctypes.c_uint64, ctypes.POINTER(ctypes.c_uint32)]

        count = ctypes.c_uint32()
        ret = func(stream_handle, ctypes.byref(count))
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())
        return count.value

    @staticmethod
    def stream_get_property_key(stream_handle, index):
        """Get the supported property key on this stream.

        :param stream_handle: Stream handle.
        :type stream_handle: long
        :param index: Index of property list.
        :type index: int
        :return: Property key.
        :rtype: bytes(str)
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of stream_get_property_key() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_stream_get_property_key
        func.restype = ctypes.c_int32
        func.argtypes = [
            ctypes.c_uint64,
            ctypes.c_uint32,
            ctypes.POINTER(ctypes.c_char_p)
        ]

        property_key = ctypes.c_char_p()
        ret = func(stream_handle, index, ctypes.byref(property_key))
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())
        return property_key.value

    @staticmethod
    def stream_lock_property(stream_handle, timeout_msec):
        """Lock to access properties.

        :param stream_handle: Stream handle.
        :type stream_handle: long
        :param timeout_msec: Time of wait msec if locked already.
        :type timeout_msec: int
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of stream_lock_property() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_stream_lock_property
        func.restype = ctypes.c_int32
        func.argtypes = [ctypes.c_uint64, ctypes.c_uint32]

        ret = func(stream_handle, timeout_msec)
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())

    @staticmethod
    def stream_lock_property_with_key(stream_handle, keys, count, timeout_msec):
        """Lock to access properties.

        :param stream_handle: Stream handle.
        :type stream_handle: long
        :param keys: Property keys.
        :type keys: list of [b'key',]
        :param count: Count of keys.
        :type count: int
        :param timeout_msec: Time of wait msec if locked already.
        :type timeout_msec: int
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of stream_lock_property() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_stream_lock_property_with_key
        func.restype = ctypes.c_int32
        func.argtypes = [
            ctypes.c_uint64,
            ctypes.POINTER(ctypes.c_char_p),
            ctypes.c_uint32,
            ctypes.c_int32,
            ctypes.POINTER(ctypes.c_uint64)]

        tmp_keys = (ctypes.c_char_p * len(keys))(*(SensCordC._to_bytes(key) for key in keys))
        lock_resource = ctypes.c_uint64()
        ret = func(stream_handle, tmp_keys, count, timeout_msec, ctypes.byref(lock_resource))
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())
        return lock_resource.value

    @staticmethod
    def stream_unlock_property(stream_handle):
        """Unlock to access properties.

        :param stream_handle: Stream handle.
        :type stream_handle: long
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of stream_unlock_property() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_stream_unlock_property
        func.restype = ctypes.c_int32
        func.argtypes = [ctypes.c_uint64]

        ret = func(stream_handle)
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())

    @staticmethod
    def stream_unlock_property_by_handle(stream_handle, access_handle):
        """Unlock to access properties.

        :param stream_handle: Stream handle.
        :type stream_handle: long
        :param access_handle: Access handle.
        :type access_handle: long
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of stream_unlock_property() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_stream_unlock_property_by_resource
        func.restype = ctypes.c_int32
        func.argtypes = [
            ctypes.c_uint64,
            ctypes.c_uint64]

        ret = func(stream_handle, access_handle)
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())

    @staticmethod
    def stream_register_frame_callback(stream_handle, callback, private_data):
        """Register the callback for frame reached.

        :param stream_handle: Stream handle.
        :type stream_handle: long
        :param callback: Callback function.
        :type callback: FRAME_CALLBACK_FUNC
        :param private_data: Private data with callback.
        :type private_data: int
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of stream_register_frame_callback() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_stream_register_frame_callback
        func.restype = ctypes.c_int32
        func.argtypes = [
            ctypes.c_uint64,
            _types.FRAME_CALLBACK_FUNC,
            ctypes.c_void_p
        ]

        ret = func(stream_handle, callback, private_data)
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())

    @staticmethod
    def stream_unregister_frame_callback(stream_handle):
        """Unregister the callback for frame reached.

        :param stream_handle: Stream handle.
        :type stream_handle: long
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of stream_unregister_frame_callback()
                          function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_stream_unregister_frame_callback
        func.restype = ctypes.c_int32
        func.argtypes = [ctypes.c_uint64]

        ret = func(stream_handle)
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())

    @staticmethod
    def stream_register_event_callback(stream_handle, event_type, callback,
                                       private_data):
        """Register the callback for event receiving.

        :param stream_handle: Stream handle.
        :type stream_handle: long
        :param event_type: Event type to receive.
        :type event_type: bytes or str
        :param callback: Callback function.
        :type callback: EVENT_CALLBACK_FUNC
        :param private_data: Private data with callback.
        :type private_data: int
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of stream_register_event_callback() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_stream_register_event_callback2
        func.restype = ctypes.c_int32
        func.argtypes = [
            ctypes.c_uint64,
            ctypes.c_char_p,
            _types.EVENT_CALLBACK_FUNC,
            ctypes.c_void_p
        ]

        event_type = SensCordC._to_bytes(event_type)
        ret = func(stream_handle, event_type, callback, private_data)
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())

    @staticmethod
    def stream_unregister_event_callback(stream_handle, event_type):
        """Unregister the event callback.

        :param stream_handle: Stream handle.
        :type stream_handle: long
        :param event_type: Event type to receive.
        :type event_type: bytes or str
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of stream_unregister_event_callback()
                          function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_stream_unregister_event_callback
        func.restype = ctypes.c_int32
        func.argtypes = [ctypes.c_uint64, ctypes.c_char_p]

        event_type = SensCordC._to_bytes(event_type)
        ret = func(stream_handle, event_type)
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())

    @staticmethod
    def frame_get_sequence_number(frame_handle):
        """Get the sequential number of frame.

        :param frame_handle: Frame handle.
        :type frame_handle: long
        :return: Number of frame.
        :rtype: long
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of frame_get_sequence_number() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_frame_get_sequence_number
        func.restype = ctypes.c_int32
        func.argtypes = [ctypes.c_uint64, ctypes.POINTER(ctypes.c_uint64)]

        number = ctypes.c_uint64()
        ret = func(frame_handle, ctypes.byref(number))
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())
        return number.value

    @staticmethod
    def frame_get_type(frame_handle):
        """Get type of frame.

        :param frame_handle: Frame handle.
        :type frame_handle: long
        :return: Type of frame.
        :rtype: bytes(str)
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of frame_get_type() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_frame_get_type
        func.restype = ctypes.c_int32
        func.argtypes = [ctypes.c_uint64, ctypes.POINTER(ctypes.c_char_p)]

        frame_type = ctypes.c_char_p()
        ret = func(frame_handle, ctypes.byref(frame_type))
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())
        return frame_type.value

    @staticmethod
    def frame_get_channel_count(frame_handle):
        """Get channel count.

        :param frame_handle: Frame handle.
        :type frame_handle: long
        :return: Count of channel list.
        :rtype: int
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of frame_get_channel_count() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_frame_get_channel_count
        func.restype = ctypes.c_int32
        func.argtypes = [ctypes.c_uint64, ctypes.POINTER(ctypes.c_uint32)]

        count = ctypes.c_uint32()
        ret = func(frame_handle, ctypes.byref(count))
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())
        return count.value

    @staticmethod
    def frame_get_channel(frame_handle, index):
        """Get channel data.

        :param frame_handle: Frame handle.
        :type frame_handle: long
        :param index: Index of channel list.
        :type index: int
        :return: Channel handle.
        :rtype: long
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of frame_get_channel() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_frame_get_channel
        func.restype = ctypes.c_int32
        func.argtypes = [
            ctypes.c_uint64,
            ctypes.c_uint32,
            ctypes.POINTER(ctypes.c_uint64)
        ]

        channel_handle = ctypes.c_uint64()
        ret = func(frame_handle, index, ctypes.byref(channel_handle))
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())
        return channel_handle.value

    @staticmethod
    def frame_get_user_data(frame_handle):
        """Get the user data.

        :param frame_handle: Frame handle.
        :type frame_handle: long
        :return: User data.
        :rtype: UserData
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of frame_get_user_data() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_frame_get_user_data
        func.restype = ctypes.c_int32
        func.argtypes = [
            ctypes.c_uint64,
            ctypes.POINTER(_types.UserData)
        ]

        user_data = _types.UserData()
        ret = func(frame_handle, ctypes.byref(user_data))
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())
        return user_data

    @staticmethod
    def channel_get_channel_id(channel_handle):
        """Get the channel ID.

        :param channel_handle: Channel handle.
        :type channel_handle: long
        :return: Channel ID.
        :rtype: int
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of channel_get_channel_id() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_channel_get_channel_id
        func.restype = ctypes.c_int32
        func.argtypes = [
            ctypes.c_uint64,
            ctypes.POINTER(ctypes.c_uint32)
        ]

        channel_id = ctypes.c_uint32()
        ret = func(channel_handle, ctypes.byref(channel_id))
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())
        return channel_id.value

    @staticmethod
    def channel_get_raw_data(channel_handle):
        """Get the raw data.

        :param channel_handle: Channel handle.
        :type channel_handle: long
        :return: Raw data.
        :rtype: RawData
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of channel_get_raw_data() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_channel_get_raw_data
        func.restype = ctypes.c_int32
        func.argtypes = [
            ctypes.c_uint64,
            ctypes.POINTER(_types.RawData)
        ]

        raw_data = _types.RawData()
        ret = func(channel_handle, ctypes.byref(raw_data))
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())
        return raw_data

    @staticmethod
    def channel_get_property(channel_handle,
                             property_key, property_value):
        """Get the property related to this raw data.

        :param channel_handle: Channel handle.
        :type channel_handle: long
        :param property_key: Property key.
        :type property_key: bytes or str
        :param property_value: Property value.
        :type property_value: ctypes.Structure or bytearray
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of channel_get_property() function.
        :raise: TypeError: Unsupported type.
        """
        if SensCordC._senscord_py_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_py_dll.senscord_py_channel_get_serialized_property
        func.restype = ctypes.c_int32
        func.argtypes = [
            ctypes.c_uint64,
            ctypes.c_char_p,
            ctypes.py_object
        ]

        property_key = SensCordC._to_bytes(property_key)

        if isinstance(property_value, serialize.Serializable):
            value = bytearray(property_value.to_bytes())
        elif isinstance(property_value, bytearray):
            value = property_value
        else:
            raise TypeError('Unsupported type. (property_value)')

        ret = func(channel_handle, property_key, value)
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())

        if isinstance(property_value, serialize.Serializable):
            decoder = serialize.Decoder(value)
            property_value.decode(decoder)

    @staticmethod
    def channel_get_property_count(channel_handle):
        """Get the count of stored property key on this channel.

        :param channel_handle: Channel handle.
        :type channel_handle: long
        :return: Count of stored property list.
        :rtype: int
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of channel_get_property_count()
                          function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_channel_get_property_count
        func.restype = ctypes.c_int32
        func.argtypes = [
            ctypes.c_uint64,
            ctypes.POINTER(ctypes.c_uint32)
        ]

        count = ctypes.c_uint32()
        ret = func(channel_handle, ctypes.byref(count))
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())
        return count.value

    @staticmethod
    def channel_get_property_key(channel_handle, index):
        """Get the stored property key on this channel.

        :param channel_handle: Channel handle.
        :type channel_handle: long
        :param index: Index of stored property list.
        :type index: int
        :return: Property key.
        :rtype: bytes(str)
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of channel_get_property_key() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_channel_get_property_key
        func.restype = ctypes.c_int32
        func.argtypes = [
            ctypes.c_uint64,
            ctypes.c_uint32,
            ctypes.POINTER(ctypes.c_char_p)
        ]

        property_key = ctypes.c_char_p()
        ret = func(channel_handle, index, ctypes.byref(property_key))
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())
        return property_key.value

    @staticmethod
    def channel_get_updated_property_count(channel_handle):
        """Get the count of updated property key on this channel.

        :param channel_handle: Channel handle.
        :type channel_handle: long
        :return: Count of updated property list.
        :rtype: int
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of channel_get_updated_property_count()
                          function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_channel_get_updated_property_count
        func.restype = ctypes.c_int32
        func.argtypes = [
            ctypes.c_uint64,
            ctypes.POINTER(ctypes.c_uint32)
        ]

        count = ctypes.c_uint32()
        ret = func(channel_handle, ctypes.byref(count))
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())
        return count.value

    @staticmethod
    def channel_get_updated_property_key(channel_handle, index):
        """Get the updated property key on this channel.

        :param channel_handle: Channel handle.
        :type channel_handle: long
        :param index: Index of updated property list.
        :type index: int
        :return: Property key.
        :rtype: bytes(str)
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of channel_get_updated_property_key()
                          function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_channel_get_updated_property_key
        func.restype = ctypes.c_int32
        func.argtypes = [
            ctypes.c_uint64,
            ctypes.c_uint32,
            ctypes.POINTER(ctypes.c_char_p)
        ]

        property_key = ctypes.c_char_p()
        ret = func(channel_handle, index, ctypes.byref(property_key))
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())
        return property_key.value

    @staticmethod
    def event_argument_get_element_count(event_args):
        """Get the number of elements.

        :param event_args: Event argument handle.
        :return: the number of elements.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_event_argument_get_element_count
        func.restype = ctypes.c_int32
        func.argtypes = [
            ctypes.c_uint64,
            ctypes.POINTER(ctypes.c_uint32)
        ]

        length = ctypes.c_uint32()
        ret = func(event_args, ctypes.byref(length))
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())
        return length.value

    @staticmethod
    def event_argument_get_key(event_args, index):
        """Gets the key at the specified index.

        :param event_args: Event argument handle.
        :param index: Element index. (0 to elements-1)
        :return: Argument key.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_event_argument_get_key
        func.restype = ctypes.c_char_p
        func.argtypes = [
            ctypes.c_uint64,
            ctypes.c_uint32
        ]

        value = func(event_args, index)
        if not value:
            raise errors.ApiError(SensCordC.get_last_error())
        return value

    @staticmethod
    def event_argument_get_serialized_binary(event_args, key):
        """Gets the serialized binary array of the specified key.

        :param event_args: Event argument handle.
        :param key: Argument key.
        :return: Serialized binary.
        :rtype: bytearray
        :raise: OperationError: SensCord library is not loaded.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_event_argument_get_serialized_binary
        func.restype = ctypes.c_int32
        func.argtypes = [
            ctypes.c_uint64,
            ctypes.c_char_p,
            ctypes.c_void_p,
            ctypes.POINTER(ctypes.c_uint32)
        ]
        argument_key = SensCordC._to_bytes(key)
        buffer_size = ctypes.c_uint32()
        # get the required size.
        func(event_args, argument_key, None, ctypes.byref(buffer_size))
        if buffer_size.value == 0:
            raise errors.ApiError(SensCordC.get_last_error())
        # get the serialized binary.
        buffer = (ctypes.c_ubyte * buffer_size.value)()
        ret = func(event_args, argument_key, ctypes.byref(buffer),
                   ctypes.byref(buffer_size))
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())
        return bytearray(buffer)

    @staticmethod
    def get_last_error():
        """Get information on the last error that occurred.

        :return: Status class.
        :rtype: Status
        :raise: OperationError: SensCord library is not loaded.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_get_last_error
        func.restype = Status
        func.argtypes = []
        status = func()
        return status

    @staticmethod
    def set_file_search_path(paths):
        """Set the file search paths.

        Use instead of SENSCORD_FILE_PATH.

        :param paths: The same format as SENSCORD_FILE_PATH.
        :type paths: str
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of senscord_set_file_search_path() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_set_file_search_path
        func.restype = ctypes.c_int32
        func.argtypes = [
            ctypes.c_char_p
        ]
        paths = SensCordC._to_bytes(paths)
        ret = func(paths)
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())

    @staticmethod
    def get_file_search_path():
        """Get the file search paths.

        :return: File path.
        :rtype: str
        :raise: OperationError: SensCord library is not loaded.
        :raise: ApiError: Failure of senscord_get_file_search_path() function.
        """
        if SensCordC._senscord_dll is None:
            raise errors.OperationError('dll is not loaded.')
        func = SensCordC._senscord_dll.senscord_get_file_search_path
        func.restype = ctypes.c_int32
        func.argtypes = [
            ctypes.c_char_p,
            ctypes.POINTER(ctypes.c_uint32)
        ]
        buffer_size = ctypes.c_uint32()
        # get the required size.
        func(None, ctypes.byref(buffer_size))
        if buffer_size.value == 0:
            raise errors.ApiError(SensCordC.get_last_error())
        # get the path string.
        buffer = (ctypes.c_char * buffer_size.value)()
        ret = func(buffer, ctypes.byref(buffer_size))
        if ret != 0:
            raise errors.ApiError(SensCordC.get_last_error())
        return _utils.bytes_to_str(buffer.value)


class Status(ctypes.Structure):
    """Error status."""
    _fields_ = [
        ('level', _types.ErrorLevel),
        ('cause', _types.ErrorCause),
        ('message', ctypes.c_char_p),
        ('block', ctypes.c_char_p),
        ('trace', ctypes.c_char_p),
    ]
    #: :var ErrorLevel: Level of error.
    level = None
    #: :var ErrorCause: Cause of error.
    cause = None
    #: :var c_char_p: Error massage.
    message = None
    #: :var c_char_p: Where the error occurred.
    block = None
    #: :var c_char_p: Trace information.
    trace = None


class StreamTypeInfo(ctypes.Structure):
    """The information of stream key."""
    _fields_ = [
        ('key', ctypes.c_char_p),
        ('type', ctypes.c_char_p),
        ('id', ctypes.c_char_p),
    ]
    #: :var c_char_p: Stream key.
    key = None
    #: :var c_char_p: Stream type.
    type = None
    #: :var c_char_p: Id.
    id = None


class StreamArgument(ctypes.Structure):
    """Stream argument element."""
    _fields_ = [
        ('name', ctypes.c_char * STREAM_ARGUMENT_NAME_LENGTH),
        ('value', ctypes.c_char * STREAM_ARGUMENT_VALUE_LENGTH),
    ]
    #: :var str: Name.
    name = None
    #: :var str: Value.
    value = None


class OpenStreamSetting(ctypes.Structure):
    """Stream argument element."""
    _fields_ = [
        ('frame_buffering', _types.FrameBuffering),
        ('arguents_count', ctypes.c_uint32),
        ('arguents', StreamArgument * STREAM_ARGUMENT_LENGTH),
    ]
    #: :var FrameBuffering: Frame buffering setting.
    frame_buffering = None
    #: :var str: Count of the stream argument array.
    arguents_count = None
    #: :var StreamArgument: Stream arguments.
    arguents = None


class Version(ctypes.Structure):
    """Version information."""
    _fields_ = [
        ('name', ctypes.c_char * VERSION_LENGTH),
        ('major', ctypes.c_uint32),
        ('minor', ctypes.c_uint32),
        ('patch', ctypes.c_uint32),
        ('description', ctypes.c_char * VERSION_LENGTH),
    ]
    #: :var str: Name.
    name = None
    #: :var uint32: Major version.
    major = None
    #: :var uint32: Minor version.
    minor = None
    #: :var uint32: Patch version.
    patch = None
    #: :var str: Version description.
    description = None


class StreamVersion(ctypes.Structure):
    """Stream Version informations."""
    _fields_ = [
        ('stream_key', ctypes.c_char * STREAM_KEY_LENGTH),
        ('stream_version', Version),
        ('linkage_count', ctypes.c_uint32),
        ('linkage_versions', ctypes.POINTER(Version)),
        ('destination_id', ctypes.c_int32),
    ]
    #: :var str: Stream key.
    stream_key = None
    #: :var VersionProperty: Stream version.
    stream_version = None
    #: :var int32: Linkage count.
    linkage_count = None
    #: :var VersionProperty: Stream linkage versions.
    linkage_versions = None
    #: :var int32: Destination ID
    destination_id = None


class SensCordVersion(ctypes.Structure):
    """SensCord version informations."""
    #: :var VersionProperty: Virtual address.
    senscord_version = None
    #: :var VersionProperty: Data size.
    project_version = None
    #: :var uint32: Stream count.
    stream_count = None
    #: :var StreamVersion: Stream versions.
    stream_versions = None
    #: :var int32: Destination ID
    destination_id = None
    #: :var uint32: Server count.
    server_count = None
    #: :var dict: Server versions. (Key=Destination ID)
    server_versions = None


SensCordVersion._fields_ = [
    ('senscord_version', Version),
    ('project_version', Version),
    ('stream_count', ctypes.c_uint32),
    ('stream_versions', ctypes.POINTER(StreamVersion)),
    ('destination_id', ctypes.c_int32),
    ('server_count', ctypes.c_uint32),
    ('server_versions', ctypes.POINTER(SensCordVersion)),
]
