# SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

"""
Type definition of SensCord SDK.
"""

from __future__ import absolute_import

import ctypes

from senscord import _utils
from senscord import serialize
from senscord import _temporal_contrast_data

# Frame received callback function.
FRAME_CALLBACK_FUNC = ctypes.CFUNCTYPE(
    None, ctypes.c_uint64, ctypes.c_void_p)

# Event received callback function.
EVENT_CALLBACK_FUNC = ctypes.CFUNCTYPE(
    None, ctypes.c_uint64, ctypes.c_char_p, ctypes.c_uint64, ctypes.c_void_p)


class ErrorLevel(_utils.CEnum):
    """Level of error."""
    _names_ = {
        'UNDEFINED': 0,
        'FAIL': 1,
        'FATAL': 2,
    }
    _values_ = {
        0: 'UNDEFINED',
        1: 'FAIL',
        2: 'FATAL',
    }
    UNDEFINED = 0
    FAIL = 1
    FATAL = 2


class ErrorCause(_utils.CEnum):
    """Cause of error."""
    _names_ = {
        'NONE': 0,
        'NOT_FOUND': 1,
        'INVALID_ARGUMENT': 2,
        'RESOURCE_EXHAUSTED': 3,
        'PERMISSION_DENIED': 4,
        'BUSY': 5,
        'TIMEOUT': 6,
        'CANCELLED': 7,
        'ABORTED': 8,
        'ALREADY_EXISTS': 9,
        'INVALID_OPERATION': 10,
        'OUT_OF_RANGE': 11,
        'DATA_LOSS': 12,
        'HARDWARE_ERROR': 13,
        'NOT_SUPPORTED': 14,
        'UNKNOWN': 15,
    }
    _values_ = {
        0: 'NONE',
        1: 'NOT_FOUND',
        2: 'INVALID_ARGUMENT',
        3: 'RESOURCE_EXHAUSTED',
        4: 'PERMISSION_DENIED',
        5: 'BUSY',
        6: 'TIMEOUT',
        7: 'CANCELLED',
        8: 'ABORTED',
        9: 'ALREADY_EXISTS',
        10: 'INVALID_OPERATION',
        11: 'OUT_OF_RANGE',
        12: 'DATA_LOSS',
        13: 'HARDWARE_ERROR',
        14: 'NOT_SUPPORTED',
        15: 'UNKNOWN',
    }
    NONE = 0
    NOT_FOUND = 1
    INVALID_ARGUMENT = 2
    RESOURCE_EXHAUSTED = 3
    PERMISSION_DENIED = 4
    BUSY = 5
    TIMEOUT = 6
    CANCELLED = 7
    ABORTED = 8
    ALREADY_EXISTS = 9
    INVALID_OPERATION = 10
    OUT_OF_RANGE = 11
    DATA_LOSS = 12
    HARDWARE_ERROR = 13
    NOT_SUPPORTED = 14
    UNKNOWN = 15


class Vector2f(serialize.Structure):
    """Vector2(float)."""
    _fields_ = [
        ('x', serialize.Float),
        ('y', serialize.Float),
    ]
    #: :var float: x value.
    x = None
    #: :var float: y value.
    y = None

    def __repr__(self):
        return '%s(%r, %r)' % (
            self.__class__.__name__, self.x, self.y)


class Vector2u32(serialize.Structure):
    """Vector2(uint32_t)."""
    _fields_ = [
        ('x', serialize.Uint32),
        ('y', serialize.Uint32),
    ]
    #: :var uint32: x value.
    x = None
    #: :var uint32: y value.
    y = None

    def __repr__(self):
        return '%s(%r, %r)' % (
            self.__class__.__name__, self.x, self.y)


class Vector3f(serialize.Structure):
    """Vector3(float)."""
    _fields_ = [
        ('x', serialize.Float),
        ('y', serialize.Float),
        ('z', serialize.Float),
    ]
    #: :var float: x value.
    x = None
    #: :var float: y value.
    y = None
    #: :var float: z value.
    z = None

    def __repr__(self):
        return '%s(%r, %r, %r)' % (
            self.__class__.__name__, self.x, self.y, self.z)


class Vector3u32(serialize.Structure):
    """Vector3(uint32_t)."""
    _fields_ = [
        ('x', serialize.Uint32),
        ('y', serialize.Uint32),
        ('z', serialize.Uint32),
    ]
    #: :var uint32: x value.
    x = None
    #: :var uint32: y value.
    y = None
    #: :var uint32: z value.
    z = None

    def __repr__(self):
        return '%s(%r, %r, %r)' % (
            self.__class__.__name__, self.x, self.y, self.z)


class Vector4f(serialize.Structure):
    """Vector4(float)."""
    _fields_ = [
        ('x', serialize.Float),
        ('y', serialize.Float),
        ('z', serialize.Float),
        ('a', serialize.Float),
    ]
    #: :var float: x value.
    x = None
    #: :var float: y value.
    y = None
    #: :var float: z value.
    z = None
    #: :var float: a value.
    a = None

    def __repr__(self):
        return '%s(%r, %r, %r, %r)' % (
            self.__class__.__name__, self.x, self.y, self.z, self.a)


class Quaternionf(serialize.Structure):
    """Quaternion(float)."""
    _fields_ = [
        ('x', serialize.Float),
        ('y', serialize.Float),
        ('z', serialize.Float),
        ('w', serialize.Float),
    ]
    #: :var float: x value.
    x = None
    #: :var float: y value.
    y = None
    #: :var float: z value.
    z = None
    #: :var float: w value.
    w = None

    def __repr__(self):
        return '%s(%r, %r, %r, %r)' % (
            self.__class__.__name__, self.x, self.y, self.z, self.w)


class Matrix3x3f(serialize.Structure):
    """Matrix(float)."""
    # [C++] float element[3][3];
    _fields_ = [
        ('element', serialize.Array(serialize.Float, 3, 3)),
    ]
    #: :var list[list[float*3]*3]: float array with 3x3 elements.
    element = None

    def __repr__(self):
        return '%s(%r)' % (
            self.__class__.__name__, self.element)


class Matrix3x4f(serialize.Structure):
    """Matrix(float)."""
    # [C++] float element[3][4];
    _fields_ = [
        ('element', serialize.Array(serialize.Float, 3, 4)),
    ]
    #: :var list[list[float*4]*3]: float array with 3x4 elements.
    element = None

    def __repr__(self):
        return '%s(%r)' % (
            self.__class__.__name__, self.element)


class ScalarU32(serialize.Structure):
    """Scalar(uint32_t)."""
    _fields_ = [
        ('value', serialize.Uint32),
    ]
    #: :var uint32: value.
    value = None

    def __repr__(self):
        return '%s(%r)' % (
            self.__class__.__name__, self.value)


class ScalarF(serialize.Structure):
    """Scalar(float)."""
    _fields_ = [
        ('value', serialize.Float),
    ]
    #: :var float: value.
    value = None

    def __repr__(self):
        return '%s(%r)' % (
            self.__class__.__name__, self.value)


class RangeU32(serialize.Structure):
    """Range expressed by the min max. (uint32_t)"""

    _fields_ = [
        ('min', serialize.Uint32),
        ('max', serialize.Uint32),
    ]
    #: :var uint32: min value.
    min = None
    #: :var uint32: max value.
    max = None

    def __repr__(self):
        return '%s(min=%r, max=%r)' % (
            self.__class__.__name__, self.min, self.max)


# Stream types
STREAM_TYPE_IMAGE = b'image'
STREAM_TYPE_DEPTH = b'depth'
STREAM_TYPE_IMU = b'imu'
STREAM_TYPE_SLAM = b'slam'
STREAM_TYPE_OBJECT_DETECTION = b'object_detection'
STREAM_TYPE_KEY_POINT = b'key_point'
STREAM_TYPE_TEMPORAL_CONTRAST = b'pixel_polarity'
STREAM_TYPE_OBJECT_TRACKING = b'object_tracking'
STREAM_TYPE_PIXEL_POLARITY = b'pixel_polarity'
STREAM_TYPE_AUDIO = b'audio'


class Buffering(_utils.CEnum):
    """Frame buffering."""
    _names_ = {
        'USE_CONFIG': -2,
        'DEFAULT': -1,
        'OFF': 0,
        'ON': 1,
    }
    _values_ = {
        -2: 'USE_CONFIG',
        -1: 'DEFAULT',
        0: 'OFF',
        1: 'ON',
    }

    #: Use config.
    USE_CONFIG = -2
    #: Buffering default.
    DEFAULT = -1
    #: Buffering disable.
    OFF = 0
    #: Buffering enable.
    ON = 1


class BufferingFormat(_utils.CEnum):
    """Frame buffering format."""
    _names_ = {
        'USE_CONFIG': -2,
        'DEFAULT': -1,
        'DISCARD': 0,
        'OVERWRITE': 1,
    }
    _values_ = {
        -2: 'USE_CONFIG',
        -1: 'DEFAULT',
        0: 'DISCARD',
        1: 'OVERWRITE',
    }
    #: Use config format.
    USE_CONFIG = -2
    #: Default format.
    DEFAULT = -1
    #: Discard the latest frame.
    DISCARD = 0
    #: Overwrite the oldest frame.
    OVERWRITE = 1
    #: Queue format. (Deprecated)
    QUEUE = DISCARD
    #: Ring format. (Deprecated)
    RING = OVERWRITE


class FrameBuffering(ctypes.Structure):
    """Frame buffering setting."""
    _fields_ = [
        ('buffering', Buffering),
        ('num', ctypes.c_int32),
        ('format', BufferingFormat),
    ]
    #: Default frame number.
    DEFAULT_NUM = -1
    #: :var Buffering: Buffering enabling.
    buffering = None
    #: :var c_int32: Max buffering frame number.
    num = None
    #: :var BufferingFormat: Buffering format.
    format = None

    def __repr__(self):
        return '%s(%r, num=%r, %r)' % (
            self.__class__.__name__, self.buffering, self.num, self.format)


class OpenStreamSetting():
    """Open stream setting."""

    def __init__(self):
        #: :var FrameBuffering: Frame buffering setting.
        self.frame_buffering = FrameBuffering()
        self.frame_buffering.buffering = Buffering.DEFAULT
        self.frame_buffering.num = FrameBuffering.DEFAULT_NUM
        self.frame_buffering.format = BufferingFormat.DEFAULT

        #: :var dict: Stream arguments.
        self.arguments = {}

    def __repr__(self):
        return '%s(%r, %r)' % (
            self.__class__.__name__, self.frame_buffering, self.arguments)


class UserData(ctypes.Structure):
    """User data informations."""
    _fields_ = [
        ('address', ctypes.c_void_p),
        ('size', ctypes.c_size_t),
    ]
    #: :var c_void_p: Virtual address.
    address = None
    #: :var c_size_t: Data size.
    size = None

    def get_bytes(self):
        """Get byte-array of user data.

        :return: Byte-array of user data.
        :rtype: bytearray
        """
        if self.address is None or self.size == 0:
            return bytearray()
        buf = (ctypes.c_ubyte * self.size).from_address(self.address)
        return bytearray(buf)

    def get(self, value):
        """Get user data.

        :param value: Variable that stores user data.
        :type value: bytearray or ctypes.Structure
        """
        if isinstance(value, serialize.Serializable):
            decoder = serialize.Decoder(self.get_bytes())
            value.decode(decoder)
        elif isinstance(value, bytearray):
            value[:] = self.get_bytes()
        elif isinstance(value, (ctypes.Structure, ctypes.Array)):
            ctypes.memmove(ctypes.byref(value), self.address, self.size)
        else:
            raise TypeError('Invalid type')

    def __repr__(self):
        return '%s(address=%r, size=%r)' % (
            self.__class__.__name__, self.address, self.size)


class RawData(ctypes.Structure):
    """Raw data informations."""
    _fields_ = [
        ('address', ctypes.c_void_p),
        ('size', ctypes.c_size_t),
        ('type', ctypes.c_char_p),
        ('timestamp', ctypes.c_uint64),
    ]
    #: :var c_void_p: Virtual address.
    address = None
    #: :var c_size_t: Data size.
    size = None
    #: :var c_char_p: Data type.
    type = None
    #: :var c_uint64: Nanoseconds timestamp captured by the device.
    timestamp = None

    def get_bytes(self):
        """Get byte-array of raw data.

        :return: Byte-array of raw data.
        :rtype: bytearray
        """
        if self.address is None or self.size == 0:
            return bytearray()
        buf = (ctypes.c_ubyte * self.size).from_address(self.address)
        return bytearray(buf)

    def get(self, value):
        """Get raw data.

        :param value: Variable that stores raw data.
        :type value: bytearray or ctypes.Structure
        """
        if isinstance(value, serialize.Serializable):
            decoder = serialize.Decoder(self.get_bytes())
            value.decode(decoder)
        elif isinstance(value, bytearray):
            value[:] = self.get_bytes()
        elif isinstance(value, (ctypes.Structure, ctypes.Array)):
            ctypes.memmove(ctypes.byref(value), self.address, self.size)
        elif isinstance(value, _temporal_contrast_data.TemporalContrastData):
            value.from_bytes(self.address, self.size)
        else:
            raise TypeError('Invalid type')

    def __repr__(self):
        return '%s(address=%r, size=%r, type=%r, timestamp=%r)' % (
            self.__class__.__name__, self.address, self.size, self.type,
            self.timestamp)


# Event definitions
EVENT_ANY = b'EventAny'  # only for event receiving
EVENT_ERROR = b'EventError'
EVENT_FATAL = b'EventFatal'
EVENT_FRAME_DROPPED = b'EventFrameDropped'
EVENT_PROPERTY_UPDATED = b'EventPropertyUpdated'
EVENT_PLUGGED = b'EventPlugged'
EVENT_UNPLUGGED = b'EventUnplugged'
EVENT_RECORD_STATE = b'EventRecordState'

EVENT_ARGUMENT_CAUSE = b'cause'
EVENT_ARGUMENT_MESSAGE = b'message'
EVENT_ARGUMENT_SEQUENCE_NUMBER = b'sequence_number'
EVENT_ARGUMENT_PROPERTY_KEY = b'property_key'
EVENT_ARGUMENT_RECORD_STATE = b'state'
EVENT_ARGUMENT_RECORD_COUNT = b'count'
EVENT_ARGUMENT_RECORD_PATH = b'path'

# Timeout definitions
TIMEOUT_POLLING = 0
TIMEOUT_FOREVER = -1


class Version:
    """Version information."""
    def __init__(self):
        #: :var str: Name.
        self.name = ''
        #: :var uint32: Major version.
        self.major = 0
        #: :var uint32: Minor version.
        self.minor = 0
        #: :var uint32: Patch version.
        self.patch = 0
        #: :var str: Version description.
        self.description = ''

    def __repr__(self):
        return '%s(name=%r, major=%r, minor=%r, patch=%r, description=%r)' % (
            self.__class__.__name__, self.name, self.major, self.minor,
            self.patch, self.description)


class StreamVersion:
    """Stream Version informations."""
    def __init__(self):
        #: :var VersionProperty: Stream version.
        self.stream_version = None
        #: :var VersionProperty: Stream linkage versions.
        self.linkage_versions = []
        #: :var int32: Destination ID
        self.destination_id = -1

    def __repr__(self):
        return ('%s(stream_version=%r, linkage_versions=%r, '
                'destination_id=%r)') % (
                    self.__class__.__name__, self.stream_version,
                    self.linkage_versions, self.destination_id)


class SensCordVersion:
    """SensCord version informations."""
    def __init__(self):
        #: :var VersionProperty: SensCord version.
        self.senscord_version = None
        #: :var VersionProperty: Project version.
        self.project_version = None
        #: :var dict: Stream versions. (Key=StreamKey)
        self.stream_versions = {}
        #: :var dict: Server versions. (Key=Destination ID)
        self.server_versions = {}

    def __repr__(self):
        return ('%s(senscord_version=%r, project_version=%r, '
                'stream_versions=%r, server_versions=%r') % (
                    self.__class__.__name__, self.senscord_version,
                    self.project_version, self.stream_versions,
                    self.server_versions)
