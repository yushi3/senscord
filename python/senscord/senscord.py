# SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

"""
User interface of SensCord SDK.
"""

from __future__ import absolute_import

import os
import platform
import threading
import ctypes

from senscord import _c_api
from senscord import errors
from senscord import senscord_types as _types
from senscord import serialize


class Core(object):
    """The core class of managing streams."""
    _loaded = False
    _lock = threading.Lock()

    def __init__(self):
        self._core_handle = 0

    @staticmethod
    def _load_library(path):
        """Load the SensCord library.

        :raise: InitializationError: Failed to load library.
        """
        with Core._lock:
            if Core._loaded:
                return

            if platform.system() == 'Windows':
                dll_name = 'senscord.dll'
                path_delimiter = ';'
            elif platform.system() == 'Darwin':
                dll_name = 'senscord.bundle'
                path_delimiter = ':'
            else:
                dll_name = 'libsenscord.so'
                path_delimiter = ':'

            if path is None:
                path = os.getenv('SENSCORD_FILE_PATH')
                if path is None:
                    raise errors.InitializationError(
                        'SENSCORD_FILE_PATH is none')
            paths = path.split(path_delimiter)
            found = False
            exceptions = []
            paths = [os.path.normpath(os.path.abspath(d)) for d in paths]
            for directory in paths:
                if not os.path.isdir(directory):
                    exceptions.append((directory, 'Directory does not exist'))
                    continue
                dll_path = os.path.join(directory, dll_name)
                if not os.path.isfile(dll_path):
                    exceptions.append((dll_path, 'File does not exist'))
                    continue
                _c_api.SensCordC.load_dll(dll_path)
                found = True
                break

            if not found:
                directories = '\n'.join(
                    '  %s: %s' % (directory, ex) for directory, ex in exceptions)
                raise errors.InitializationError(
                    'SensCord could not be loaded:\n%s' % directories)

            Core._loaded = True

    def init(self, path=None):
        """Load the SensCord library and initialize Core.

        :param path: File search path of the same format as SENSCORD_FILE_PATH.
            If path is specified, it takes precedence over SENSCORD_FILE_PATH.
        :type path: str
        :raise: InitializationError: Failed to load library.
        :raise: OperationError:
        :raise: ApiError: Failure of core_init() function.
        """
        if self._core_handle:
            raise errors.OperationError(
                'SensCord has already been initialized.')

        Core._load_library(path)

        if path is not None:
            _c_api.SensCordC.set_file_search_path(path)

        self._core_handle = _c_api.SensCordC.core_init()

    def exit(self):
        """Finalize Core and close all opened streams.

        :raise: ApiError: Failure of core_exit() function.
        """
        _c_api.SensCordC.core_exit(self._core_handle)
        self._core_handle = 0

    def get_stream_list(self):
        """Get supported streams list.

        :return: List of stream info.
        :rtype: tuple of ((b'key', b'type'), (b'key', b'type'), ...)
        :raise: ApiError: Failure of core_get_stream_count() or
                          core_get_stream_info() function.
        """
        count = _c_api.SensCordC.core_get_stream_count(self._core_handle)
        stream_list = []
        for index in range(count):
            stream_info = _c_api.SensCordC.core_get_stream_info(
                self._core_handle, index)
            stream_list.append((stream_info.key, stream_info.type, stream_info.id))
        return tuple(stream_list)

    def get_opened_stream_count(self, stream_key):
        """Get count of opened stream.

        :param stream_key: Stream key.
        :type stream_key: bytes or str
        :return: Count of opened stream.
        :rtype: int
        :raise: ApiError: Failure of core_get_opened_stream_count() function.
        """
        count = _c_api.SensCordC.core_get_opened_stream_count(
            self._core_handle, stream_key)
        return count

    def get_version(self):
        """Get the version of this library.

        :return: Version.
        :rtype: SensCordVersion
        :raise: ApiError: Failure of core_get_version() function.
        """
        version = _c_api.SensCordC.core_get_version(self._core_handle)
        return version

    def get_file_search_path(self):
        """Get the file search paths.

        :return: File path.
        :rtype: str
        :raise: ApiError: Failure of senscord_get_file_search_path() function.
        """
        path = _c_api.SensCordC.get_file_search_path()
        return path

    def open_stream(self, stream_key, setting=None):
        """Open the new stream from key.

        :param stream_key: Stream key.
        :type stream_key: bytes or str
        :param setting: Config to open stream.
        :type setting: OpenStreamSetting
        :return: Stream class.
        :rtype: Stream
        :raise: ApiError: Failure of core_open_stream() or
                          core_open_stream_with_setting() function.
        """
        if setting is None:
            stream_handle = _c_api.SensCordC.core_open_stream(
                self._core_handle, stream_key)
        else:
            stream_handle = _c_api.SensCordC.core_open_stream_with_setting(
                self._core_handle, stream_key, setting)
        return Stream(stream_handle)

    def close_stream(self, stream):
        """Close the opened stream.

        :param stream: Stream class.
        :type stream: Stream
        :raise: ApiError: Failure of core_close_stream() function.
        """
        if not isinstance(stream, Stream):
            raise TypeError("Invalid instance of 'stream'")
        # pylint: disable=protected-access
        stream_handle = stream._get_handle()
        stream._release()
        # pylint: enable=protected-access
        try:
            _c_api.SensCordC.core_close_stream(self._core_handle,
                                               stream_handle)
        except Exception:
            # pylint: disable=protected-access
            stream._set_handle(stream_handle)
            # pylint: enable=protected-access
            raise

    def __repr__(self):
        return '%s(handle=%r)' % (self.__class__.__name__, self._core_handle)


class PropertyLockResource(object):
    """Property lock resource class."""

    def __init__(self, stream, lock_resource):
        self._stream = stream
        self.lock_resource = lock_resource

    def __enter__(self):
        return self

    def __exit__(self, exception_type, exception_value, traceback):
        self._stream.unlock_property(self)


class Stream(object):
    """Stream interface class."""

    class CallbackData(object):
        """Data of frame and event callback."""
        def __init__(self, stream, callback_func, private_data):
            self.stream = stream
            self.func = callback_func
            self.private_data = private_data

    def __init__(self, stream_handle):
        self._stream_handle = stream_handle

        self._frame_callback_lock = threading.Lock()
        self._frame_callback_func = _types.FRAME_CALLBACK_FUNC(
            Stream._on_frame_callback)
        self._frame_callback_data = None

        self._event_callback_lock = threading.Lock()
        self._event_callback_func = _types.EVENT_CALLBACK_FUNC(
            Stream._on_event_callback)
        self._event_callback_data = {}

    def _set_handle(self, stream_handle):
        """Set stream handle."""
        self._stream_handle = stream_handle

    def _get_handle(self):
        """Get stream handle."""
        return self._stream_handle

    def _release(self):
        """Release stream."""
        self._stream_handle = 0

    def start(self):
        """Start this stream.

        :raise: ApiError: Failure of stream_start() function.
        """
        _c_api.SensCordC.stream_start(self._stream_handle)

    def stop(self):
        """Stop this stream.

        :raise: ApiError: Failure of stream_stop() function.
        """
        _c_api.SensCordC.stream_stop(self._stream_handle)

    def get_frame(self, timeout_msec):
        """Get the received frame.

        :param timeout_msec: Time of wait msec if no received.
                             0 is polling, minus is forever.
        :type timeout_msec: int
        :return: Frame class.
        :rtype: Frame
        :raise: ApiError: Failure of stream_get_frame() function.
        """
        frame_handle = _c_api.SensCordC.stream_get_frame(
            self._stream_handle, timeout_msec)
        return Frame(frame_handle)

    def release_frame(self, frame):
        """Release the gotten frame.

        :param frame: Received frame by get_frame().
        :type frame: Frame
        :raise: ApiError: Failure of stream_release_frame() function.
        """
        if not isinstance(frame, Frame):
            raise TypeError("Invalid instance of 'frame'")
        # pylint: disable=protected-access
        frame_handle = frame._get_handle()
        # pylint: enable=protected-access
        _c_api.SensCordC.stream_release_frame(self._stream_handle,
                                              frame_handle)
        # pylint: disable=protected-access
        frame._release()
        # pylint: enable=protected-access

    def release_frame_unused(self, frame):
        """Release the gotten frame.

        Use this function if you do not refer to the raw data of the channel.

        :param frame: Received frame by get_frame().
        :type frame: Frame
        :raise: ApiError: Failure of stream_release_frame() function.
        """
        if not isinstance(frame, Frame):
            raise TypeError("Invalid instance of 'frame'")
        # pylint: disable=protected-access
        frame_handle = frame._get_handle()
        # pylint: enable=protected-access
        _c_api.SensCordC.stream_release_frame_unused(
            self._stream_handle, frame_handle)
        # pylint: disable=protected-access
        frame._release()
        # pylint: enable=protected-access

    def clear_frames(self):
        """Clear frames have not gotten.

        :return: Number of cleared frames.
        :rtype: int
        :raise: ApiError: Failure of stream_clear_frames() function.
        """
        clear_count = _c_api.SensCordC.stream_clear_frames(self._stream_handle)
        return clear_count

    def get_property(self, property_key, property_value):
        """Get the property.

        :param property_key: Property key.
        :type property_key: bytes or str
        :param property_value: Property value.
        :type property_value: ctypes.Structure or bytearray
        :raise: ApiError: Failure of stream_get_property() function.
        :raise: TypeError: Unsupported type.
        """
        _c_api.SensCordC.stream_get_property(
            self._stream_handle, property_key, property_value)

    def set_property(self, property_key, property_value):
        """Set the property with key.

        :param property_key: Property key.
        :type property_key: bytes or str
        :param property_value: Property value.
        :type property_value: ctypes.Structure or bytearray
        :raise: ApiError: Failure of stream_set_property() function.
        :raise: TypeError: Unsupported type.
        """
        _c_api.SensCordC.stream_set_property(
            self._stream_handle, property_key, property_value)

    def get_userdata_property(self, property_value):
        """Get the user data property.

        :param property_value: Property value.
        :type property_value: ctypes.Structure or bytearray
        :raise: ApiError: Failure of stream_get_userdata_property() function.
        :raise: TypeError: Unsupported type.
        """
        _c_api.SensCordC.stream_get_userdata_property(
            self._stream_handle, property_value)

    def set_userdata_property(self, property_value):
        """Set the user data property.

        :param property_value: Property value.
        :type property_value: ctypes.Structure or bytearray
        :raise: ApiError: Failure of stream_set_userdata_property() function.
        :raise: TypeError: Unsupported type.
        """
        _c_api.SensCordC.stream_set_userdata_property(
            self._stream_handle, property_value)

    def get_property_list(self):
        """Get the supported property key list on this stream.

        :return: List of property key.
        :rtype: tuple of (b'key',)
        :raise: ApiError: Failure of stream_get_property_count() or
                          stream_get_property_key() function.
        """
        count = _c_api.SensCordC.stream_get_property_count(self._stream_handle)
        property_list = []
        for index in range(count):
            property_key = _c_api.SensCordC.stream_get_property_key(
                self._stream_handle, index)
            property_list.append(property_key)
        return tuple(property_list)

    def lock_property(self, timeout_msec, keys=None):
        """Lock to access properties.

        :param timeout_msec: Time of wait msec if locked already.
        :type timeout_msec: int
        :param keys: List of property key to lock.
        :type keys: list of [b'key',]
        :return: Property lock resource.
        :rtype: PropertyLockResource
        :raise: ApiError: Failure of stream_lock_property() function.
        """
        if keys is not None:
            count = len(keys)
            access_handle = _c_api.SensCordC.stream_lock_property_with_key(
                self._stream_handle, keys, count, timeout_msec)
            return PropertyLockResource(self, access_handle)
        _c_api.SensCordC.stream_lock_property(
            self._stream_handle, timeout_msec)
        return None

    def unlock_property(self, lock_resource=None):
        """Unlock to access properties.

        :param lock_resource: Lock resource.
        :type lock_resource: PropertyLockResource
        :raise: ApiError: Failure of stream_unlock_property() function.
        """
        if lock_resource is None:
            _c_api.SensCordC.stream_unlock_property(self._stream_handle)
        else:
            _c_api.SensCordC.stream_unlock_property_by_handle(
                self._stream_handle, lock_resource.lock_resource)

    @staticmethod
    def _on_frame_callback(_stream_handle, private_data):
        """Frame callback function."""
        callback_data_ptr = ctypes.cast(private_data,
                                        ctypes.POINTER(ctypes.py_object))
        callback_data = callback_data_ptr.contents.value
        if callback_data is None:
            return
        callback_data.func(callback_data.stream, callback_data.private_data)

    def register_frame_callback(self, callback, private_data):
        """Register the callback for frame reached.

        :param callback: Callback function.
        :type callback: function(stream, private_data)
        :param private_data: Private data with callback.
        :type private_data: any object
        :raise: ApiError: Failure of stream_register_frame_callback() function.
        """
        if not callable(callback):
            raise TypeError("'callback' is not callable.")
        callback_data = Stream.CallbackData(self, callback, private_data)
        callback_data_ptr = ctypes.pointer(ctypes.py_object(callback_data))
        with self._frame_callback_lock:
            _c_api.SensCordC.stream_register_frame_callback(
                self._stream_handle, self._frame_callback_func,
                callback_data_ptr)
            # Keep parameters and avoid GC.
            self._frame_callback_data = callback_data_ptr

    def unregister_frame_callback(self):
        """Unregister the callback for frame reached.

        :raise: ApiError: Failure of stream_unregister_frame_callback()
                          function.
        """
        with self._frame_callback_lock:
            _c_api.SensCordC.stream_unregister_frame_callback(
                self._stream_handle)
            self._frame_callback_data = None

    @staticmethod
    def _on_event_callback(_stream_handle, event, args, private_data):
        """Event callback function."""
        callback_data_ptr = ctypes.cast(private_data,
                                        ctypes.POINTER(ctypes.py_object))
        callback_data = callback_data_ptr.contents.value
        if callback_data is None:
            return
        # args to dict(str, any)
        params = {}
        arg_count = _c_api.SensCordC.event_argument_get_element_count(args)
        for index in range(arg_count):
            key = _c_api.SensCordC.event_argument_get_key(args, index)
            val = _c_api.SensCordC.event_argument_get_serialized_binary(
                args, key)
            params[key] = serialize.decode(val)
        callback_data.func(callback_data.stream, event, params,
                           callback_data.private_data)

    def register_event_callback(self, event_type, callback, private_data):
        """Register the callback for event receiving.

        :param event_type: Event type to receive.
        :type event_type: bytes or str
        :param callback: Callback function.
        :type callback: function(stream, event, args, private_data)
        :param private_data: Private data with callback.
        :type private_data: any object
        :raise: ApiError: Failure of stream_register_event_callback() function.
        """
        if not callable(callback):
            raise TypeError("'callback' is not callable.")
        callback_data = Stream.CallbackData(self, callback, private_data)
        callback_data_ptr = ctypes.pointer(ctypes.py_object(callback_data))
        with self._event_callback_lock:
            _c_api.SensCordC.stream_register_event_callback(
                self._stream_handle, event_type, self._event_callback_func,
                callback_data_ptr)
            # Keep parameters and avoid GC.
            self._event_callback_data[event_type] = callback_data_ptr

    def unregister_event_callback(self, event_type):
        """Unregister the event callback.

        :param event_type: Event type to receive.
        :type event_type: bytes or str
        :raise: ApiError: Failure of stream_unregister_event_callback()
                          function.
        """
        with self._event_callback_lock:
            _c_api.SensCordC.stream_unregister_event_callback(
                self._stream_handle, event_type)
            del self._event_callback_data[event_type]

    def __repr__(self):
        return '%s(handle=%r)' % (self.__class__.__name__, self._stream_handle)


class Frame(object):
    """Frame interface class."""
    def __init__(self, frame_handle):
        """Create a Frame object.

        :param frame_handle: Frame handle.
        :raise: ApiError: Failure of frame_get_channel_count() or
                          frame_get_channel() function.
        """
        self._frame_handle = frame_handle

        # get channel list.
        count = _c_api.SensCordC.frame_get_channel_count(
            self._frame_handle)
        channel_list = []
        for index in range(count):
            channel_handle = _c_api.SensCordC.frame_get_channel(
                self._frame_handle, index)
            channel_list.append(Channel(channel_handle))
        self._channel_list = tuple(channel_list)

    def _get_handle(self):
        """Get frame handle."""
        return self._frame_handle

    def _release(self):
        """Release frame and channel."""
        self._frame_handle = 0
        for channel in self._channel_list:
            # pylint: disable=protected-access
            channel._release()
            # pylint: enable=protected-access

    def get_sequence_number(self):
        """Get the sequential number of frame.

        :return: Number of frame.
        :rtype: long
        :raise: ApiError: Failure of frame_get_sequence_number() function.
        """
        number = _c_api.SensCordC.frame_get_sequence_number(self._frame_handle)
        return number

    def get_type(self):
        """Get type of frame.

        :return: Type of frame.
        :rtype: bytes(str)
        :raise: ApiError: Failure of frame_get_type() function.
        """
        frame_type = _c_api.SensCordC.frame_get_type(self._frame_handle)
        return frame_type

    def get_channel_list(self):
        """Get list of channel.

        :return: List of channel.
        :rtype: tuple of (Channel,)
        """
        return self._channel_list

    def get_channel(self, channel_id):
        """Get channel data.

        :param channel_id: Channel ID.
        :type channel_id: int
        :return: Channel class.
        :rtype: Channel
        :raise: ApiError: Failure of channel_get_channel_id() function.
        """
        for channel in self._channel_list:
            get_id = channel.get_channel_id()
            if get_id == channel_id:
                return channel
        return None

    def get_user_data(self):
        """Get the user data.

        :return: User data.
        :rtype: UserData
        :raise: ApiError: Failure of frame_get_user_data() function.
        """
        user_data = _c_api.SensCordC.frame_get_user_data(self._frame_handle)
        return user_data

    def __repr__(self):
        return '%s(handle=%r, channels=%r)' % (
            self.__class__.__name__, self._frame_handle, self._channel_list)


class Channel(object):
    """Channel of frame interface class."""
    def __init__(self, channel_handle):
        self._channel_handle = channel_handle

    def _release(self):
        """Release channel."""
        self._channel_handle = 0

    def get_channel_id(self):
        """Get the channel ID.

        :return: Channel ID.
        :rtype: int
        :raise: ApiError: Failure of channel_get_channel_id() function.
        """
        channel_id = _c_api.SensCordC.channel_get_channel_id(
            self._channel_handle)
        return channel_id

    def get_raw_data(self):
        """Get the raw data.

        :return: Raw data.
        :rtype: RawData
        :raise: ApiError: Failure of channel_get_raw_data() function.
        """
        raw_data = _c_api.SensCordC.channel_get_raw_data(
            self._channel_handle)
        return raw_data

    def get_property(self, property_key, property_value):
        """Get the property related to this raw data.

        :param property_key: Property key.
        :type property_key: bytes or str
        :param property_value: Property value.
        :type property_value: ctypes.Structure or bytearray
        :raise: ApiError: Failure of channel_get_property() function.
        :raise: TypeError: Unsupported type.
        """
        _c_api.SensCordC.channel_get_property(
            self._channel_handle, property_key, property_value)

    def get_property_list(self):
        """Get the count of stored property key on this channel.

        :return: Stored property key list.
        :rtype: tuple of (b'key',)
        :raise: ApiError: Failure of channel_get_property_count()
                          or channel_get_property_key() function.
        """
        count = _c_api.SensCordC.channel_get_property_count(
            self._channel_handle)
        property_list = []
        for index in range(count):
            key = _c_api.SensCordC.channel_get_property_key(
                self._channel_handle, index)
            property_list.append(key)
        return tuple(property_list)

    def get_updated_property_list(self):
        """Get the count of updated property key on this channel.

        :return: Stored property key list.
        :rtype: tuple of (b'key',)
        :raise: ApiError: Failure of channel_get_updated_property_count()
                          or channel_get_updated_property_key() function.
        """
        count = _c_api.SensCordC.channel_get_updated_property_count(
            self._channel_handle)
        property_list = []
        for index in range(count):
            key = _c_api.SensCordC.channel_get_updated_property_key(
                self._channel_handle, index)
            property_list.append(key)
        return tuple(property_list)

    def __repr__(self):
        return '%s(handle=%r)' % (
            self.__class__.__name__, self._channel_handle)
