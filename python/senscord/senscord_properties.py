# SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

"""Properties of SensCord SDK."""

from __future__ import absolute_import

from senscord import _utils
from senscord import serialize
from senscord import senscord_types as _types
from senscord import senscord_rawdata as _rawdata


class VersionProperty(serialize.Structure):
    """Version information."""
    #: Property key.
    KEY = b'version_property'

    _fields_ = [
        ('name', serialize.String),
        ('major', serialize.Uint32),
        ('minor', serialize.Uint32),
        ('patch', serialize.Uint32),
        ('description', serialize.String),
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

    def __repr__(self):
        return '%s(name=%r, major=%r, minor=%r, patch=%r, description=%r)' % (
            self.__class__.__name__, self.name, self.major, self.minor,
            self.patch, self.description)


class StreamTypeProperty(serialize.Structure):
    """Property for the type of the stream."""
    #: Property key.
    KEY = b'stream_type_property'

    _fields_ = [
        ('type', serialize.String),
    ]
    #: :var str: Type of the stream.
    type = None

    def __repr__(self):
        return '%r' % self.type


class StreamKeyProperty(serialize.Structure):
    """Property for the key of the stream."""
    #: Property key.
    KEY = b'stream_key_property'

    _fields_ = [
        ('stream_key', serialize.String),
    ]
    #: :var str: Key of the stream.
    stream_key = None

    def __repr__(self):
        return '%r' % self.stream_key


class StreamState(_utils.CEnum):
    """Stream status."""
    _names_ = {
        'UNDEFINED': 0,
        'READY': 1,
        'RUNNING': 2,
    }
    _values_ = {
        0: 'UNDEFINED',
        1: 'READY',
        2: 'RUNNING',
    }
    #: Undefined state.
    UNDEFINED = 0
    #: Opened but not start.
    READY = 1
    #: Started.
    RUNNING = 2


class StreamStateProperty(serialize.Structure):
    """Property for the current state of the stream."""
    #: Property key.
    KEY = b'stream_state_property'

    _fields_ = [
        ('state', StreamState),
    ]
    #: :var StreamState: Current state of the stream.
    state = None

    def __repr__(self):
        return '%r' % self.state


class CurrentFrameNumProperty(serialize.Structure):
    """Property for the current buffering frames."""
    #: Property key.
    KEY = b'current_frame_num_property'

    _fields_ = [
        ('arrived_number', serialize.Int32),
        ('received_number', serialize.Int32),
    ]
    #: :var int32: Arrived number.
    arrived_number = None
    #: :var int32: Received number.
    received_number = None

    def __repr__(self):
        return '%s(arrived=%r, received=%r)' % (
            self.__class__.__name__, self.arrived_number, self.received_number)


class FrameBufferingProperty(serialize.Structure):
    """Frame buffering setting."""
    #: Property key.
    KEY = b'frame_buffering_property'

    _fields_ = [
        ('buffering', _types.Buffering),
        ('num', serialize.Int32),
        ('format', _types.BufferingFormat),
    ]
    #: Uses the config frame number.
    USE_CONFIG_NUM = -2
    #: Default frame number.
    DEFAULT_NUM = -1
    #: Unlimited frame number.
    UNLIMITED_NUM = 0
    #: :var Buffering: Buffering enabling.
    buffering = None
    #: :var int32: Max buffering frame number
    num = None
    #: :var BufferingFormat: Buffering format.
    format = None

    def __repr__(self):
        return '%s(%r, num=%r, %r)' % (
            self.__class__.__name__, self.buffering, self.num, self.format)


class ChannelInfo(serialize.Structure):
    """Channel information."""
    _fields_ = [
        ('raw_data_type', serialize.String),
        ('description', serialize.String),
    ]
    #: :var str: Type of raw data.
    raw_data_type = None
    #: :var str: Channel description.
    description = None


class ChannelInfoProperty(serialize.Structure):
    """Property for channel information."""
    #: Property key.
    KEY = b'channel_info_property'

    _fields_ = [
        ('channels', (dict, (serialize.Uint32, ChannelInfo))),
    ]
    #: :var dict: Channel information list. (Key=Channel ID)
    channels = None


class ChannelMaskProperty(serialize.Structure):
    """Property for masking the channel."""
    #: Property key.
    KEY = b'channel_mask_property'

    _fields_ = [
        ('channels', (list, serialize.Uint32)),
    ]
    #: :var list: The list of masked channel IDs.
    channels = None


class RecordProperty(serialize.Structure):
    """Property for the recording frames."""
    #: Property key.
    KEY = b'record_property'

    _fields_ = [
        ('enabled', serialize.Bool),
        ('path', serialize.String),
        ('count', serialize.Uint32),
        ('formats', (dict, (serialize.Uint32, serialize.String))),
        ('buffer_num', serialize.Uint32),
        ('name_rules', (dict, (serialize.String, serialize.String))),
    ]
    #: :var enabled: State of recording.
    #:               If set to true, recording will start.
    #:               Startable only in the stream running state.
    enabled = None

    #: :var path: Top directory path of recording files.
    #:            When to stop, this member is ignored.
    path = None

    #: :var count: The count of record frames.
    count = None

    #: :var formats: Format names of each channel ID.
    #:               Frames of no specified channel ID will not be recorded.
    #:               For get the available formats, use RecorderListProperty.
    #:               When to stop, this member is ignored.
    formats = None

    #: :var buffer_num: Number of the buffering of recording frame queue.
    #:                  If set zero means the number equals one.
    #:                  When to stop, this member is ignored.
    buffer_num = None

    #: :var name_rules: Directory naming rules.
    #:                  Key is the directory type, value is a format string.
    #:                  When to stop, this member is ignored.
    name_rules = None


#: Standard recording formats.
RECORDING_FORMAT_RAW = b'raw'
RECORDING_FORMAT_COMPOSITE_RAW = b'composite_raw'
RECORDING_FORMAT_SKV = b'skv'

#: Directory types.
RECORD_DIRECTORY_TOP = b'top'


class RecorderListProperty(serialize.Structure):
    """Property for reference the available recording formats.

    For stream property getting only.
    """
    #: Property key.
    KEY = b'recorder_list_property'

    _fields_ = [
        ('formats', (list, serialize.String)),
    ]
    #: :var formats: List of recording formats.
    formats = None


class PlaySpeed(_utils.CEnum):
    """Frame replay speed with player."""
    _names_ = {
        'BASED_ON_FRAMERATE': 0,
        'BEST_EFFORT': 1,
    }
    _values_ = {
        0: 'BASED_ON_FRAMERATE',
        1: 'BEST_EFFORT',
    }
    #: Sending based on framerate.
    BASED_ON_FRAMERATE = 0
    #: Sending without framerate.
    BEST_EFFORT = 1


class PlayModeProperty(serialize.Structure):
    """Property for the mode of replay the stream."""
    #: Property key.
    KEY = b'play_mode_property'

    _fields_ = [
        ('repeat', serialize.Bool),
    ]
    #: Enabling the repeat play.
    repeat = None


class PlayPauseProperty(serialize.Structure):
    """Property for the mode of pause the stream."""
    #: Property key.
    KEY = b'play_pause_property'

    _fields_ = [
        ('pause', serialize.Bool),
    ]
    #: Enabling the pause play.
    pause = None


class PlayProperty(serialize.Structure):
    """Property for the settings before replay stream."""
    #: Property key.
    KEY = b'play_property'

    _fields_ = [
        ('target_path', serialize.String),
        ('start_offset', serialize.Uint32),
        ('count', serialize.Uint32),
        ('speed', PlaySpeed),
        ('mode', PlayModeProperty),
    ]
    #: Path of the recorded data.
    target_path = None

    #: Offset of starting frame.
    start_offset = None

    #: Playing frames from start_offset.
    count = None

    #: Play speed.
    speed = None

    #: Play mode.
    mode = None


class PlayFileInfoProperty(serialize.Structure):
    """Property for playback file information."""
    #: Property key.
    KEY = b'play_file_info_property'

    _fields_ = [
        ('target_path', serialize.String),
        ('record_date', serialize.String),
        ('stream_key', serialize.String),
        ('stream_type', serialize.String),
        ('frame_count', serialize.Uint32),
    ]
    #: Directory path of recorded.
    target_path = None

    #: Recorded date and time.
    record_date = None

    #: Stream key to recorded.
    stream_key = None

    #: Stream type to recorded.
    stream_type = None

    #: Number of Frames recorded.
    frame_count = None


class PlayPositionProperty(serialize.Structure):
    """Property that indicates the playback position in the player function."""
    #: Property key.
    KEY = b'play_position_property'

    _fields_ = [
        ('position', serialize.Uint32),
    ]
    #: Playback position.
    position = None


class AccelerationUnit(_utils.CEnum):
    """Units used for acceleration."""
    _names_ = {
        'NOT_SUPPORTED': 0,
        'GRAVITATIONAL': 1,
        'METRE_PER_SECOND_SQUARED': 2,
    }
    _values_ = {
        0: 'NOT_SUPPORTED',
        1: 'GRAVITATIONAL',
        2: 'METRE_PER_SECOND_SQUARED',
    }
    #: Sensor not supported.
    NOT_SUPPORTED = 0
    #: Unit:[G].
    GRAVITATIONAL = 1
    #: Unit:[m/s2].
    METRE_PER_SECOND_SQUARED = 2


class AngularVelocityUnit(_utils.CEnum):
    """Units used for angular velocity."""
    _names_ = {
        'NOT_SUPPORTED': 0,
        'DEGREE_PER_SECOND': 1,
        'RADIAN_PER_SECOND': 2,
    }
    _values_ = {
        0: 'NOT_SUPPORTED',
        1: 'DEGREE_PER_SECOND',
        2: 'RADIAN_PER_SECOND',
    }
    #: Sensor not supported.
    NOT_SUPPORTED = 0
    #: Unit:[deg/s].
    DEGREE_PER_SECOND = 1
    #: Unit:[rad/s].
    RADIAN_PER_SECOND = 2


class MagneticFieldUnit(_utils.CEnum):
    """Units used for magnetic field."""
    _names_ = {
        'NOT_SUPPORTED': 0,
        'GAUSS': 1,
        'MICRO_TESLA': 2,
    }
    _values_ = {
        0: 'NOT_SUPPORTED',
        1: 'GAUSS',
        2: 'MICRO_TESLA',
    }
    #: Sensor not supported.
    NOT_SUPPORTED = 0
    #: Unit:[gauss].
    GAUSS = 1
    #: Unit:[uT].
    MICRO_TESLA = 2


class OrientationUnit(_utils.CEnum):
    """Units used for orientataion."""
    _names_ = {
        'NOT_SUPPORTED': 0,
        'DEGREE': 1,
        'RADIAN': 2,
    }
    _values_ = {
        0: 'NOT_SUPPORTED',
        1: 'DEGREE',
        2: 'RADIAN',
    }
    #: Sensor not supported.
    NOT_SUPPORTED = 0
    #: Unit:[deg].
    DEGREE = 1
    #: Unit:[rad].
    RADIAN = 2


class VelocityUnit(_utils.CEnum):
    """Units used for velocity."""
    _names_ = {
        'NOT_SUPPORTED': 0,
        'METRE_PER_SECOND': 1,
        'PIXEL_PER_SECOND': 2,
    }
    _values_ = {
        0: 'NOT_SUPPORTED',
        1: 'METRE_PER_SECOND',
        2: 'PIXEL_PER_SECOND',
    }
    #: Sensor not supported.
    NOT_SUPPORTED = 0
    #: Unit:[m/s].
    METRE_PER_SECOND = 1
    #: Unit:[pixel/s].
    PIXEL_PER_SECOND = 2


class CoordinateSystem(_utils.CEnum):
    """Types of coordinate system."""
    _names_ = {
        'WORLD_COORDINATE': 0,
        'LOCAL_COORDINATE': 1,
        'CAMERA_COORDINATE': 2,
    }
    _values_ = {
        0: 'WORLD_COORDINATE',
        1: 'LOCAL_COORDINATE',
        2: 'CAMERA_COORDINATE',
    }
    #: World coordinate system.
    WORLD_COORDINATE = 0
    #: Local coordinate system.
    LOCAL_COORDINATE = 1
    #: Camera coordinate system.
    CAMERA_COORDINATE = 2


class GridUnit(_utils.CEnum):
    """Units of grid."""
    _names_ = {
        'PIXEL': 0,
        'METER': 1,
    }
    _values_ = {
        0: 'PIXEL',
        1: 'METER',
    }
    #: Unit:[pixel].
    PIXEL = 0
    #: Unit:[m].
    METER = 1


class InterlaceField(_utils.CEnum):
    """The field types of interlace."""
    _names_ = {
        'TOP': 0,
        'BOTTOM': 1,
    }
    _values_ = {
        0: 'TOP',
        1: 'BOTTOM',
    }
    TOP = 0         # Top field
    BOTTOM = 1      # Bottom field


class InterlaceOrder(_utils.CEnum):
    """The order of interlace."""
    _names_ = {
        'TOP_FIRST': 0,
        'BOTTOM_FIRST': 1,
    }
    _values_ = {
        0: 'TOP_FIRST',
        1: 'BOTTOM_FIRST',
    }
    TOP_FIRST = 0         # Top first
    BOTTOM_FIRST = 1      # Bottom first


class SystemHanded(_utils.CEnum):
    """System handed for CoordinateSystem."""
    _names_ = {
        'LEFT': 0,
        'RIGHT': 1,
    }
    _values_ = {
        0: 'LEFT',
        1: 'RIGHT',
    }
    #: Left-handed system.
    LEFT = 0
    #: Right-handed system.
    RIGHT = 1


class UpAxis(_utils.CEnum):
    """Up axis for CoordinateSystem."""
    _names_ = {
        'UNDEFINED': 0,
        'PLUS_X': 1,
        'PLUS_Y': 2,
        'PLUS_Z': 3,
        'MINUS_X': 4,
        'MINUS_Y': 5,
        'MINUS_Z': 6,
    }
    _values_ = {
        0: 'UNDEFINED',
        1: 'PLUS_X',
        2: 'PLUS_Y',
        3: 'PLUS_Z',
        4: 'MINUS_X',
        5: 'MINUS_Y',
        6: 'MINUS_Z',
    }
    #: UpAxis undefined.
    UNDEFINED = 0
    #: X axis up.
    PLUS_X = 1
    #: Y axis up.
    PLUS_Y = 2
    #: Z axis up.
    PLUS_Z = 3
    #: X axis minus.
    MINUS_X = 4
    #: Y axis minus.
    MINUS_Y = 5
    #: Z axis minus.
    MINUS_Z = 6


class ForwardAxis(_utils.CEnum):
    """Forward axis for CoordinateSystem."""
    _names_ = {
        'UNDEFINED': 0,
        'PLUS_X': 1,
        'PLUS_Y': 2,
        'PLUS_Z': 3,
        'MINUS_X': 4,
        'MINUS_Y': 5,
        'MINUS_Z': 6,
    }
    _values_ = {
        0: 'UNDEFINED',
        1: 'PLUS_X',
        2: 'PLUS_Y',
        3: 'PLUS_Z',
        4: 'MINUS_X',
        5: 'MINUS_Y',
        6: 'MINUS_Z',
    }
    #: ForwardAxis undefined.
    UNDEFINED = 0
    #: X axis up.
    PLUS_X = 1
    #: Y axis up.
    PLUS_Y = 2
    #: Z axis up.
    PLUS_Z = 3
    #: X axis minus.
    MINUS_X = 4
    #: Y axis minus.
    MINUS_Y = 5
    #: Z axis minus.
    MINUS_Z = 6


class PresetListProperty(serialize.Structure):
    """Property for the list of property's preset IDs."""
    #: Property key.
    KEY = b'preset_list_property'

    _fields_ = [
        ('presets', (dict, (serialize.Uint32, serialize.String))),
    ]
    #: :var dict: Preset ID + description.
    presets = None


class PresetProperty(serialize.Structure):
    """Structure for the property's preset."""
    #: Property key.
    KEY = b'preset_property'

    _fields_ = [
        ('id', serialize.Uint32),
    ]
    #: :var uint32: Preset ID.
    id = None


class ImageProperty(serialize.Structure):
    """Structures that handle properties of Raw data of Image and
       Depth data."""
    # Property key.
    KEY = b'image_property'

    _fields_ = [
        ('width', serialize.Uint32),
        ('height', serialize.Uint32),
        ('stride_bytes', serialize.Uint32),
        ('pixel_format', serialize.String),
    ]
    #: :var uint32: Image width.
    width = None
    #: :var uint32: Image height.
    height = None
    #: :var uint32: Image stride.
    stride_bytes = None
    #: :var str: The format of a pixel.
    pixel_format = None

    def __repr__(self):
        return '%s(w=%r, h=%r, stride_bytes=%r, pixel_format=%r)' % (
            self.__class__.__name__, self.width, self.height,
            self.stride_bytes, self.pixel_format)


class ConfidenceProperty(serialize.Structure):
    """Structures that handle properties of raw data of confidence."""
    # Property key.
    KEY = b'confidence_property'

    _fields_ = [
        ('width', serialize.Uint32),
        ('height', serialize.Uint32),
        ('stride_bytes', serialize.Uint32),
        ('pixel_format', serialize.String),
    ]
    #: :var uint32: Image width.
    width = None
    #: :var uint32: Image height.
    height = None
    #: :var uint32: Image stride.
    stride_bytes = None
    #: :var str: The format of a pixel.
    pixel_format = None

    def __repr__(self):
        return '%s(w=%r, h=%r, stride_bytes=%r, pixel_format=%r)' % (
            self.__class__.__name__, self.width, self.height,
            self.stride_bytes, self.pixel_format)


# Predefined pixel format.
# Packed RGB
PIXEL_FORMAT_ARGB444 = b'image_argb444'     #: ARGB 4444
PIXEL_FORMAT_XRGB444 = b'image_xrgb444'     #: XRGB 4444
PIXEL_FORMAT_RGB24 = b'image_rgb24'         #: RGB 888
PIXEL_FORMAT_ARGB32 = b'image_argb32'       #: ARGB 8888
PIXEL_FORMAT_XRGB32 = b'image_xrgb32'       #: XRGB 8888
PIXEL_FORMAT_BGR24 = b'image_bgr24'         #: BGR 888
PIXEL_FORMAT_ABGR32 = b'image_abgr32'       #: ABGR 8888
PIXEL_FORMAT_XBGR32 = b'image_xbgr32'       #: XBGR 8888
# Planar RGB
PIXEL_FORMAT_RGB8_PLANAR = b'image_rgb8_planar'  #: RGB 8-bit
PIXEL_FORMAT_RGB16_PLANAR = b'image_rgb16_planar'  #: RGB 16-bit
# Greyscale
PIXEL_FORMAT_GREY = b'image_grey'           #: 8-bit Greyscale
PIXEL_FORMAT_Y10 = b'image_y10'             #: 10-bit Greyscale (on 16bit)
PIXEL_FORMAT_Y12 = b'image_y12'             #: 12-bit Greyscale (on 16bit)
PIXEL_FORMAT_Y14 = b'image_y14'             #: 14-bit Greyscale (on 16bit)
PIXEL_FORMAT_Y16 = b'image_y16'             #: 16-bit Greyscale
PIXEL_FORMAT_Y20 = b'image_y20'             #: 20-bit Greyscale (on 32bit)
PIXEL_FORMAT_Y24 = b'image_y24'             #: 24-bit Greyscale (on 32bit)
# YUV
PIXEL_FORMAT_YUV444 = b'image_yuv444'       #: YUV444
PIXEL_FORMAT_NV12 = b'image_nv12'           #: YUV420SP
PIXEL_FORMAT_NV16 = b'image_nv16'           #: YUV422SP
PIXEL_FORMAT_YUV420 = b'image_yuv420'       #: YUV420
PIXEL_FORMAT_YUV422P = b'image_yuv422p'     #: YUV422P
PIXEL_FORMAT_YUYV = b'image_yuyv'           #: YUYV
PIXEL_FORMAT_UYVY = b'image_uyvy'           #: UYVY
# Bayer
PIXEL_FORMAT_SBGGR8 = b'image_sbggr8'       #:
PIXEL_FORMAT_SGBRG8 = b'image_sgbrg8'       #:
PIXEL_FORMAT_SGRBG8 = b'image_sgrbg8'       #:
PIXEL_FORMAT_SRGGB8 = b'image_srggb8'       #:
PIXEL_FORMAT_SBGGR10 = b'image_sbggr10'     #:
PIXEL_FORMAT_SGBRG10 = b'image_sgbrg10'     #:
PIXEL_FORMAT_SGRBG10 = b'image_sgrbg10'     #:
PIXEL_FORMAT_SRGGB10 = b'image_srggb10'     #:
PIXEL_FORMAT_SBGGR12 = b'image_sbggr12'     #:
PIXEL_FORMAT_SGBRG12 = b'image_sgbrg12'     #:
PIXEL_FORMAT_SGRBG12 = b'image_sgrbg12'     #:
PIXEL_FORMAT_SRGGB12 = b'image_srggb12'     #:
# Quad Bayer
PIXEL_FORMAT_QUAD_SBGGR8 = b'image_quad_sbggr8'   #:
PIXEL_FORMAT_QUAD_SGBRG8 = b'image_quad_sgbrg8'   #:
PIXEL_FORMAT_QUAD_SGRBG8 = b'image_quad_sgrbg8'   #:
PIXEL_FORMAT_QUAD_SRGGB8 = b'image_quad_srggb8'   #:
PIXEL_FORMAT_QUAD_SBGGR10 = b'image_quad_sbggr10' #:
PIXEL_FORMAT_QUAD_SGBRG10 = b'image_quad_sgbrg10' #:
PIXEL_FORMAT_QUAD_SGRBG10 = b'image_quad_sgrbg10' #:
PIXEL_FORMAT_QUAD_SRGGB10 = b'image_quad_srggb10' #:
PIXEL_FORMAT_QUAD_SBGGR12 = b'image_quad_sbggr12' #:
PIXEL_FORMAT_QUAD_SGBRG12 = b'image_quad_sgbrg12' #:
PIXEL_FORMAT_QUAD_SGRBG12 = b'image_quad_sgrbg12' #:
PIXEL_FORMAT_QUAD_SRGGB12 = b'image_quad_srggb12' #:
# Polarization image
PIXEL_FORMAT_POLAR_90_45_135_0_Y8 = b'image_polar_90_45_135_0_y8'         #:
PIXEL_FORMAT_POLAR_90_45_135_0_Y10 = b'image_polar_90_45_135_0_y10'       #:
PIXEL_FORMAT_POLAR_90_45_135_0_Y12 = b'image_polar_90_45_135_0_y12'       #:
PIXEL_FORMAT_POLAR_90_45_135_0_RGGB8 = b'image_polar_90_45_135_0_rggb8'   #:
PIXEL_FORMAT_POLAR_90_45_135_0_RGGB10 = b'image_polar_90_45_135_0_rggb10' #:
PIXEL_FORMAT_POLAR_90_45_135_0_RGGB12 = b'image_polar_90_45_135_0_rggb12' #:
# Compressed image
PIXEL_FORMAT_JPEG = b'image_jpeg'           #:
PIXEL_FORMAT_H264 = b'image_h264'           #:
# Depth
PIXEL_FORMAT_Z16 = b'depth_z16'             #: 16-bit Z-Depth
PIXEL_FORMAT_Z32F = b'depth_z32f'           #: 32-bit float Z-Depth
PIXEL_FORMAT_D16 = b'depth_d16'             #: 16-bit Disparity
# Confidence
PIXEL_FORMAT_C1P = b'confidence_c1p'        #: 1-bit positive confidence
PIXEL_FORMAT_C1N = b'confidence_c1n'        #: 1-bit negative confidence
PIXEL_FORMAT_C16 = b'confidence_c16'        #: 16-bit confidence
PIXEL_FORMAT_C32F = b'confidence_c32f'      #: 32-bit float confidence
# PointCloud
#: signed   16-bit (x, y, depth)
PIXEL_FORMAT_XYZ16 = b'point_cloud_xyz16'
#: signed   16-bit (x, y, depth, rgb)
PIXEL_FORMAT_XYZRGB16 = b'point_cloud_xyzrgb16'
#: signed   32-bit (x, y, depth)
PIXEL_FORMAT_XYZ32 = b'point_cloud_xyz32'
#: signed   32-bit (x, y, depth, rgb)
PIXEL_FORMAT_XYZRGB32 = b'point_cloud_xyzrgb32'
#: unsigned 16-bit (x, y, depth)
PIXEL_FORMAT_XYZ16U = b'point_cloud_xyz16u'
#: unsigned 16-bit (x, y, depth, rgb)
PIXEL_FORMAT_XYZRGB16U = b'point_cloud_xyzrgb16u'
#: unsigned 32-bit (x, y, depth)
PIXEL_FORMAT_XYZ32U = b'point_cloud_xyz32u'
#: unsigned 32-bit (x, y, depth, rgb)
PIXEL_FORMAT_XYZRGB32U = b'point_cloud_xyzrgb32u'
#: signed   32-bit float (x, y, depth)
PIXEL_FORMAT_XYZ32F = b'point_cloud_xyz32f'
#: signed   32-bit float (x, y, depth, rgb)
PIXEL_FORMAT_XYZRGB32F = b'point_cloud_xyzrgb32f'
#: signed   16-bit (x, y, depth) planar array
PIXEL_FORMAT_XYZ16_PLANAR = b'point_cloud_xyz16_planar'
#: unsigned 16-bit (x, y, depth) planar array
PIXEL_FORMAT_XYZ16U_PLANAR = b'point_cloud_xyz16u_planar'
#: signed   32-bit float(x, y, depth) planar array
PIXEL_FORMAT_XYZ32F_PLANAR = b'point_cloud_xyz32f_planar'
# GridMap
#: 1-bit positive voxel data
PIXEL_FORMAT_GRID_MAP_1P1N = b'grid_map_1p1n'


class YCbCrEncoding(_utils.CEnum):
    """Encoding types for YUV (YCbCr)."""
    _names_ = {
        'UNDEFINED': 0,
        'BT601': 1,
        'BT709': 2,
        'BT2020': 3,
        'BT2100': 4,
    }
    _values_ = {
        0: 'UNDEFINED',
        1: 'BT601',
        2: 'BT709',
        3: 'BT2020',
        4: 'BT2100',
    }
    UNDEFINED = 0
    BT601 = 1
    BT709 = 2
    BT2020 = 3
    BT2100 = 4


class YCbCrQuantization(_utils.CEnum):
    """Quantization types for YUV (YCbCr)."""
    _names_ = {
        'UNDEFINED': 0,
        'FULL_RANGE': 1,
        'LIMITED_RANGE': 2,
        'SUPER_WHITE': 3,
    }
    _values_ = {
        0: 'UNDEFINED',
        1: 'FULL_RANGE',
        2: 'LIMITED_RANGE',
        3: 'SUPER_WHITE',
    }
    UNDEFINED = 0
    FULL_RANGE = 1     # Y: 0-255,  C: 0-255
    LIMITED_RANGE = 2  # Y: 16-235, C: 16-240
    SUPER_WHITE = 3


class ColorSpaceProperty(serialize.Structure):
    """Property of color space type for YUV."""
    #: Property key.
    KEY = b'color_space_property'

    _fields_ = [
        ('encoding', YCbCrEncoding),
        ('quantization', YCbCrQuantization),
    ]
    #: :var YCbCrEncoding: Encoding type.
    encoding = None
    #: :var YCbCrQuantization: Quantization type.
    quantization = None


class FrameRateProperty(serialize.Structure):
    """Structure for setting frame rate.

    Specify in the style of numerator / denominator.
    ex) 60fps : num = 60, denom = 1
    """
    #: Property key.
    KEY = b'frame_rate_property'

    _fields_ = [
        ('num', serialize.Uint32),
        ('denom', serialize.Uint32),
    ]
    #: :var uint32: Framerate numerator.
    num = None
    #: :var uint32: Framerate denominator.
    denom = None

    def __repr__(self):
        return '%s(num=%r, denom=%r)' % (
            self.__class__.__name__, self.num, self.denom)


class SkipFrameProperty(serialize.Structure):
    """Structure for setting the skip rate of the frame.

    If 'rate = 1' is specified, frames are not skipped.
    If 'rate = N' (N is 2 or more) is specified, the frame is skipped and
    the frame rate drops to 1 / N.
    """
    #: Property key.
    KEY = b'skip_frame_property'

    _fields_ = [
        ('rate', serialize.Uint32),
    ]
    #: :var uint32: Skip rate.
    rate = None


class LensProperty(serialize.Structure):
    """Structure used to acquire field angle of camera."""
    #: Property key.
    KEY = b'lens_property'

    _fields_ = [
        ('horizontal_field_of_view', serialize.Float),
        ('vertical_field_of_view', serialize.Float),
    ]
    #: :var float: The horizontal viewing angle of the lens.
    horizontal_field_of_view = None
    #: :var float: The vertical viewing angle of the lens.
    vertical_field_of_view = None

    def __repr__(self):
        return '%s(horizontal_fov=%r, vertical_fov=%r)' % (
            self.__class__.__name__, self.horizontal_field_of_view,
            self.vertical_field_of_view)


class DepthProperty(serialize.Structure):
    """Structure for handling Depth data properties."""
    #: Property key.
    KEY = b'depth_property'

    _fields_ = [
        ('scale', serialize.Float),
        ('depth_min_range', serialize.Float),
        ('depth_max_range', serialize.Float),
    ]
    #: :var float: Scale of the depth value, in metres.
    #:             By multiplying this value, the depth value is converted
    #:             to metres.
    scale = None
    #: :var float: Minimum depth value of the sensor.
    depth_min_range = None
    #: :var float: Maximum depth value of the sensor.
    depth_max_range = None

    def __repr__(self):
        return '%s(scale=%r, min=%r, max=%r)' % (
            self.__class__.__name__, self.scale, self.depth_min_range,
            self.depth_max_range)


class ImageSensorFunctionProperty(serialize.Structure):
    """Structures used to set the functions used in the sensor."""
    #: Property key.
    KEY = b'image_sensor_function_property'

    _fields_ = [
        ('auto_exposure', serialize.Bool),
        ('auto_white_balance', serialize.Bool),
        ('brightness', serialize.Int32),
        ('iso_sensitivity', serialize.Uint32),
        ('exposure_time', serialize.Uint32),
        ('exposure_metering', serialize.String),
        ('gamma_value', serialize.Float),
        ('gain_value', serialize.Uint32),
        ('hue', serialize.Int32),
        ('saturation', serialize.Int32),
        ('sharpness', serialize.Int32),
        ('white_balance', serialize.Int32),
    ]
    #: :var bool: Setting auto exposure function.
    auto_exposure = None
    #: :var bool: Setting automatic white balance function.
    auto_white_balance = None
    #: :var int32: Brightness value.
    brightness = None
    #: :var uint32: ISO sensitivity. (100,200,400,800,1600,3200,...)
    iso_sensitivity = None
    #: :var uint32: Time of exposure [100usec].
    exposure_time = None
    #: :var str: Exposure metering mode.
    exposure_metering = None
    #: :var float: Gamma correction value.
    gamma_value = None
    #: :var uint32: Gain value.
    gain_value = None
    #: :var int32: Hue value.
    hue = None
    #: :var int32: Saturation value.
    saturation = None
    #: :var int32: Sharpness value.
    sharpness = None
    #: :var int32: White balance value.
    white_balance = None

    def __repr__(self):
        return ('%s(auto_exposure=%r, auto_white_balance=%r, brightness=%r, '
                'iso_sensitivity=%r, exposure_time=%r, exposure_metering=%r, '
                'gamma_value=%r, gain_value=%r, hue=%r, saturation=%r, '
                'sharpness=%r, white_balance =%r)') % (
                    self.__class__.__name__, self.auto_exposure,
                    self.auto_white_balance, self.brightness,
                    self.iso_sensitivity,
                    self.exposure_time, self.exposure_metering,
                    self.gamma_value, self.gain_value, self.hue,
                    self.saturation, self.sharpness, self.white_balance)


# Predefined exposure fifed values.
# Exposure time : Auto
EXPOSURE_TIME_AUTO = 0
# ISO Sensitivity : Auto
ISO_SENSITIVITY_AUTO = 0

# Predefined exposure metering mode.
# Metering mode : None
EXPOSURE_METERING_NONE = b'none'
# Metering mode : Average
EXPOSURE_METERING_AVERAGE = b'average'
# Metering mode : Center weighted
EXPOSURE_METERING_CENTER_WEIGHTED = b'center_weighted'
# Metering mode : Spot
EXPOSURE_METERING_SPOT = b'spot'
# Metering mode : Matrix
EXPOSURE_METERING_MATRIX = b'matrix'


class ImageSensorFunctionSupportedProperty(serialize.Structure):
    """Structure for acquiring functions supported by Component.

    For each function of the image sensor, set the counter corresponding
    to Component as a boolean value.
    """
    #: Property key.
    KEY = b'image_sensor_function_supported_property'

    _fields_ = [
        ('auto_exposure_supported', serialize.Bool),
        ('auto_white_balance_supported', serialize.Bool),
        ('brightness_supported', serialize.Bool),
        ('iso_sensitivity_supported', serialize.Bool),
        ('exposure_time_supported', serialize.Bool),
        ('exposure_metering_supported', serialize.Bool),
        ('gamma_value_supported', serialize.Bool),
        ('gain_value_supported', serialize.Bool),
        ('hue_supported', serialize.Bool),
        ('saturation_supported', serialize.Bool),
        ('sharpness_supported', serialize.Bool),
        ('white_balance_supported', serialize.Bool),
    ]
    #: :var bool: Auto exposure support.
    auto_exposure_supported = None
    #: :var bool: Automatic white balance support.
    auto_white_balance_supported = None
    #: :var bool: Brightness support.
    brightness_supported = None
    #: :var bool: ISO sensitivity support.
    iso_sensitivity_supported = None
    #: :var bool: Exposure Time support.
    exposure_time_supported = None
    #: :var bool: Exposure metering support.
    exposure_metering_supported = None
    #: :var bool: Gamma correction value support.
    gamma_value_supported = None
    #: :var bool: Gain value support.
    gain_value_supported = None
    #: :var bool: Hue value support.
    hue_supported = None
    #: :var bool: Saturation support.
    saturation_supported = None
    #: :var bool: Sharpness support.
    sharpness_supported = None
    #: :var bool: White balance support.
    white_balance_supported = None

    def __repr__(self):
        return ('%s(auto_exposure=%r, auto_white_balance=%r, brightness=%r, '
                'iso_sensitivity=%r, exposure_time=%r, exposure_metering=%r, '
                'gamma_value=%r, gain_value=%r, hue=%r, saturation=%r, '
                'sharpness=%r, white_balance =%r)') % (
                    self.__class__.__name__, self.auto_exposure_supported,
                    self.auto_white_balance_supported,
                    self.brightness_supported, self.iso_sensitivity_supported,
                    self.exposure_time_supported,
                    self.exposure_metering_supported,
                    self.gamma_value_supported, self.gain_value_supported,
                    self.hue_supported, self.saturation_supported,
                    self.sharpness_supported, self.white_balance_supported)


class ExposureProperty(serialize.Structure):
    """Structure for the image of the camera exposure."""
    #: Property key.
    KEY = b'exposure_property'

    _fields_ = [
        ('mode', serialize.String),
        ('ev_compensation', serialize.Float),
        ('exposure_time', serialize.Uint32),
        ('iso_sensitivity', serialize.Uint32),
        ('metering', serialize.String),
        ('target_region', _rawdata.RectangleRegionParameter),
    ]
    #: :var str: Mode of exposure.
    mode = None
    #: :var float: Compensation value of EV.
    ev_compensation = None
    #: :var uint32: Time of exposure [100usec].
    exposure_time = None
    #: :var uint32: ISO sensitivity. (100,200,400,800,1600,3200,...)
    iso_sensitivity = None
    #: :var str: Exposure metering mode.
    metering = None
    #: :var RectangleRegionParameter: Target region of the camera exposure.
    target_region = None

    def __repr__(self):
        return ('%s(mode=%r, ev_compensation=%r, exposure_time=%r, '
                'iso_sensitivity=%r, metering=%r, %r)') % (
                    self.__class__.__name__,
                    self.mode,
                    self.ev_compensation,
                    self.exposure_time,
                    self.iso_sensitivity,
                    self.metering,
                    self.target_region)


# Predefined exposure modes.
# Exposure mode : Auto
EXPOSURE_MODE_AUTO = b'auto'
# Exposure mode : Hold
EXPOSURE_MODE_HOLD = b'hold'
# Exposure mode : Manual
EXPOSURE_MODE_MANUAL = b'manual'
# Exposure mode : Gain Fix
EXPOSURE_MODE_GAIN_FIX = b'gainfix'
# Exposure mode : Time Fix
EXPOSURE_MODE_TIME_FIX = b'timefix'


class WhiteBalanceProperty(serialize.Structure):
    """Structure for the white balance."""
    #: Property key.
    KEY = b'white_balance_property'

    _fields_ = [
        ('mode', serialize.String),
    ]
    #: :var str: Mode of white balance.
    mode = None

    def __repr__(self):
        return '%s(mode=%r)' % (
            self.__class__.__name__, self.mode)


# Predefined while balance modes.
# White balance mode : Auto
WHITE_BALANCE_MODE_AUTO = b'auto'
# White balance mode : Manual
WHITE_BALANCE_MODE_MANUAL = b'manual'


class IntrinsicCalibrationParameter(serialize.Structure):
    """Structure for handling internal parameters of calibration."""
    _fields_ = [
        ('cx', serialize.Float),
        ('cy', serialize.Float),
        ('fx', serialize.Float),
        ('fy', serialize.Float),
        ('s', serialize.Float),
    ]
    #: :var float: The x-axis coordinate of the optical center point.
    cx = None
    #: :var float: The y-axis coordinate of the optical center point.
    cy = None
    #: :var float: Focal length on x axis.
    fx = None
    #: :var float: Focal length on y axis.
    fy = None
    #: :var float: skewness.
    s = None

    def __repr__(self):
        return '%s(cx=%r, cy=%r, fx=%r, fy=%r, s=%r)' % (
            self.__class__.__name__,
            self.cx, self.cy, self.fx, self.fy, self.s)


class ExtrinsicCalibrationParameter(serialize.Structure):
    """Structure for handling extrinsic parameters of calibration."""
    _fields_ = [
        ('r11', serialize.Float),
        ('r12', serialize.Float),
        ('r13', serialize.Float),
        ('r21', serialize.Float),
        ('r22', serialize.Float),
        ('r23', serialize.Float),
        ('r31', serialize.Float),
        ('r32', serialize.Float),
        ('r33', serialize.Float),
        ('t1', serialize.Float),
        ('t2', serialize.Float),
        ('t3', serialize.Float),
        ('p', _types.Matrix3x4f),
    ]
    r11 = None  #: :var float: Extrinsic parameter r11.
    r12 = None  #: :var float: Extrinsic parameter r12.
    r13 = None  #: :var float: Extrinsic parameter r13.
    r21 = None  #: :var float: Extrinsic parameter r21.
    r22 = None  #: :var float: Extrinsic parameter r22.
    r23 = None  #: :var float: Extrinsic parameter r23.
    r31 = None  #: :var float: Extrinsic parameter r31.
    r32 = None  #: :var float: Extrinsic parameter r32.
    r33 = None  #: :var float: Extrinsic parameter r33.
    t1 = None  #: :var float: Extrinsic parameter t1.
    t2 = None  #: :var float: Extrinsic parameter t2.
    t3 = None  #: :var float: Extrinsic parameter t3.
    p = None  #: :var float: Extrinsic parameter p11-p34.

    def __repr__(self):
        return ('%s(r11=%r, r12=%r, r13=%r, r21=%r, r22=%r, r23=%r, r31=%r, '
                'r32=%r, r33=%r, t1=%r, t2=%r, t3=%r, p=%r)') % (
                    self.__class__.__name__,
                    self.r11, self.r12, self.r13, self.r21, self.r22, self.r23,
                    self.r31, self.r32, self.r33, self.t1, self.t2, self.t3,
                    self.p)


class DistortionCalibrationParameter(serialize.Structure):
    """Structure for handling camera distortion coefficient."""
    _fields_ = [
        ('k1', serialize.Float),
        ('k2', serialize.Float),
        ('k3', serialize.Float),
        ('k4', serialize.Float),
        ('k5', serialize.Float),
        ('k6', serialize.Float),
        ('p1', serialize.Float),
        ('p2', serialize.Float),
    ]
    k1 = None  #: :var float: Camera distortion coefficient k1.
    k2 = None  #: :var float: Camera distortion coefficient k2.
    k3 = None  #: :var float: Camera distortion coefficient k3.
    k4 = None  #: :var float: Camera distortion coefficient k4.
    k5 = None  #: :var float: Camera distortion coefficient k5.
    k6 = None  #: :var float: Camera distortion coefficient k6.
    p1 = None  #: :var float: Camera distortion coefficient p1.
    p2 = None  #: :var float: Camera distortion coefficient p2.

    def __repr__(self):
        return '%s(k1=%r, k2=%r, k3=%r, k4=%r, k5=%r, k6=%r, p1=%r, p2=%r)' % (
            self.__class__.__name__, self.k1, self.k2, self.k3, self.k4,
            self.k5, self.k6, self.p1, self.p2)


class CameraCalibrationParameters(serialize.Structure):
    """Calibration parameters of a single camera."""
    _fields_ = [
        ('intrinsic', IntrinsicCalibrationParameter),
        ('distortion', DistortionCalibrationParameter),
        ('extrinsic', ExtrinsicCalibrationParameter),
    ]
    #: :var IntrinsicCalibrationParameter: Camera internal parameters.
    intrinsic = None
    #: :var DistortionCalibrationParameter: Distortion correction coefficient.
    distortion = None
    #: :var ExtrinsicCalibrationParameter: Extrinsic parameters.
    extrinsic = None

    def __repr__(self):
        return '%s(%r, %r, %r)' % (
            self.__class__.__name__, self.intrinsic, self.distortion,
            self.extrinsic)


class CameraCalibrationProperty(serialize.Structure):
    """Property for camera calibration."""
    #: Property key.
    KEY = b'camera_calibration_property'

    # [C++] std::map<uint32_t, CameraCalibrationParameters> parameters;
    _fields_ = [
        ('parameters', (dict, (serialize.Uint32, CameraCalibrationParameters))),
    ]
    #: :var dict: List of camera calibration parameters.
    parameters = None

    def __repr__(self):
        return '%s(%r)' % (self.__class__.__name__, self.parameters)


class InterlaceProperty(serialize.Structure):
    """Structure for interlace of channel."""
    #: Property key.
    KEY = b'interlace_property'

    _fields_ = [
        ('field', InterlaceField),
    ]
    #: :var InterlaceField: contained field type.
    field = None

    def __repr__(self):
        return '%r' % self.field


class InterlaceInfoProperty(serialize.Structure):
    """Structure for interlace informations."""
    #: Property key.
    KEY = b'interlace_info_property'

    _fields_ = [
        ('order', InterlaceOrder),
    ]
    #: :var InterlaceOrder: order of field
    order = None

    def __repr__(self):
        return '%r' % self.order


class ImageCropProperty(serialize.Structure):
    """Structure for image cropping."""
    #: Property key.
    KEY = b'image_crop_property'

    _fields_ = [
        ('left', serialize.Uint32),
        ('top', serialize.Uint32),
        ('width', serialize.Uint32),
        ('height', serialize.Uint32),
    ]
    #: :var uint32: Horizontal offset of the top left corner of the cropping rectangle.
    left = None
    #: :var uint32: Vertical offset of the top left corner of the cropping rectangle.
    top = None
    #: :var uint32: Width of the cropping rectangle.
    width = None
    #: :var uint32: Height of the cropping rectangle.
    height = None

    def __repr__(self):
        return '%s(left=%r, top=%r, width=%r, height=%r)' % (
            self.__class__.__name__, self.left, self.top, self.width, self.height)


class ImageCropBoundsProperty(serialize.Structure):
    """Structure for bounds of the image crop."""
    #: Property key.
    KEY = b'image_crop_bounds_property'

    _fields_ = [
        ('left', serialize.Uint32),
        ('top', serialize.Uint32),
        ('width', serialize.Uint32),
        ('height', serialize.Uint32),
    ]
    #: :var uint32: Horizontal offset of the cropable rectangle area. In pixels.
    left = None
    #: :var uint32: Vertical offset of the cropable rectangle. In pixels.
    top = None
    #: :var uint32: Width of the cropable rectangle from the offset. In pixels.
    width = None
    #: :var uint32: Height of the cropable rectangle from the offset. In pixels.
    height = None

    def __repr__(self):
        return '%s(left=%r, top=%r, width=%r, height=%r)' % (
            self.__class__.__name__, self.left, self.top, self.width, self.height)


class BaselineLengthProperty(serialize.Structure):
    """Structure for handling baseline length between cameras."""
    #: Property key.
    KEY = b'baseline_length_property'

    _fields_ = [
        ('length_mm', serialize.Float),
    ]
    #: :var float: Baseline length in millimeters.
    length_mm = None

    def __repr__(self):
        return '%s(length_mm=%r)' % (
            self.__class__.__name__, self.length_mm)


class ImuDataUnitProperty(serialize.Structure):
    """Property for obtaining unit of RawData."""
    #: Property key.
    KEY = b'imu_data_unit_property'

    _fields_ = [
        ('acceleration', AccelerationUnit),
        ('angular_velocity', AngularVelocityUnit),
        ('magnetic_field', MagneticFieldUnit),
        ('orientation', OrientationUnit),
    ]
    #: :var AccelerationUnit: Unit of data of accelerometer.
    acceleration = None
    #: :var AngularVelocityUnit: Unit of data of angular velocity.
    angular_velocity = None
    #: :var MagneticFieldUnit: Unit of data of the magnetic field.
    magnetic_field = None
    #: :var OrientationUnit: Unit of data of the orientation.
    orientation = None

    def __repr__(self):
        return '%s(%r, %r, %r, %r)' % (
            self.__class__.__name__, self.acceleration, self.angular_velocity,
            self.magnetic_field, self.orientation)


class SamplingFrequencyProperty(_types.ScalarF):
    """Set the sampling frequency in units of [Hz]."""
    #: Property key.
    KEY = b'sampling_frequency_property'


class AccelerometerRangeProperty(_types.ScalarF):
    """Set the acceleration range."""
    #: Property key.
    KEY = b'accelerometer_range_property'


class GyrometerRangeProperty(_types.ScalarF):
    """Set the gyrometer range."""
    #: Property key.
    KEY = b'gyrometer_range_property'


class MagnetometerRangeProperty(_types.ScalarF):
    """Set the magnetometer range."""
    #: Property key.
    KEY = b'magnetometer_range_property'


class MagnetometerRange3Property(_types.Vector3f):
    """Set the range of magnetometer for each xyz."""
    #: Property key.
    KEY = b'magnetometer_range3_property'


class AxisMisalignment(serialize.Structure):
    """Misalignment of the axis direction."""
    _fields_ = [
        ('ms', _types.Matrix3x3f),
        ('offset', _types.Vector3f),
    ]
    #: :var Matrix3x3f: matrix.
    ms = None
    #: :var Vector3f: offset.
    offset = None

    def __repr__(self):
        return '%s(%r, offset=%r)' % (
            self.__class__.__name__, self.ms, self.offset)


class AccelerationCalibProperty(AxisMisalignment):
    """Property used for calibration of acceleration data."""
    #: Property key.
    KEY = b'acceleration_calib_property'


class AngularVelocityCalibProperty(AxisMisalignment):
    """Property used for calibration of angular velocity data."""
    #: Property key.
    KEY = b'angular_velocity_calib_property'


class MagneticFieldCalibProperty(AxisMisalignment):
    """Property used for calibration of magnetic field data."""
    #: Property key.
    KEY = b'magnetic_field_calib_property'


class MagneticNorthCalibProperty(serialize.Structure):
    """Property for calibration magnetic north."""
    #: Property key.
    KEY = b'magnetic_north_calib_property'

    _fields_ = [
        ('declination', serialize.Float),
        ('inclination', serialize.Float),
    ]
    #: :var float: Magnetic declination.
    declination = None
    #: :var float: Magnetic inclination.
    inclination = None

    def __repr__(self):
        return '%s(declination=%r, inclination=%r)' % (
            self.__class__.__name__, self.declination, self.inclination)


class SlamDataSupportedProperty(serialize.Structure):
    """Data format supported by SLAM stream."""
    #: Property key.
    KEY = b'slam_data_supported_property'

    _fields_ = [
        ('odometry_supported', serialize.Bool),
        ('gridmap_supported', serialize.Bool),
        ('pointcloud_supported', serialize.Bool),
    ]
    #: :var bool: Support for position and attitude data.
    odometry_supported = None
    #: :var bool: GridMap support.
    gridmap_supported = None
    #: :var bool: PointCloud support.
    pointcloud_supported = None

    def __repr__(self):
        return ('%s(odometry_supported=%r, gridmap_supported=%r, '
                'pointcloud_supported=%r)') % (
                    self.__class__.__name__, self.odometry_supported,
                    self.gridmap_supported, self.pointcloud_supported)


class InitialPoseProperty(_rawdata.PoseData):
    """Initial pose"""
    #: Property key.
    KEY = b'initial_pose_property'


class PoseDataProperty(serialize.Structure):
    """Pose data property."""
    #: Property key.
    KEY = b'pose_data_property'

    _fields_ = [
        ('data_format', serialize.String),
    ]

    #: :var str: format of pose data.
    data_format = None

    # Pose data format.
    # Pose data : Quaternion
    DATA_FORMAT_QUATERNION = b'pose_data_quaternion'
    # Pose data : Matrix
    DATA_FORMAT_MATRIX = b'pose_data_matrix'

    def __repr__(self):
        return '%s(%r)' % (
            self.__class__.__name__, self.data_format)


class OdometryDataProperty(serialize.Structure):
    """Position Data Property."""
    #: Property key.
    KEY = b'odometry_data_property'

    _fields_ = [
        ('coordinate_system', CoordinateSystem),
    ]
    #: :var CoordinateSystem: Coordinate system.
    coordinate_system = None

    def __repr__(self):
        return '%s(%r)' % (
            self.__class__.__name__, self.coordinate_system)


class GridSize(serialize.Structure):
    """Grid Size."""
    _fields_ = [
        ('x', serialize.Float),
        ('y', serialize.Float),
        ('z', serialize.Float),
        ('unit', GridUnit),
    ]
    #: :var float: Grid size of x axis.
    x = None
    #: :var float: Grid size of y axis.
    y = None
    #: :var float: Grid size of z axis.
    z = None
    #: :var GridUnit: Types of Grid Map.
    unit = None

    def __repr__(self):
        return '%s(x=%r, y=%r, z=%r, unit=%r)' % (
            self.__class__.__name__, self.x, self.y, self.z, self.unit)


class GridSizeProperty(GridSize):
    """Grid Size Property."""
    #: Property key.
    KEY = b'grid_size_property'


class GridMapProperty(serialize.Structure):
    """Grid Data Property."""
    #: Property key.
    KEY = b'grid_map_property'

    _fields_ = [
        ('grid_num_x', serialize.Uint32),
        ('grid_num_y', serialize.Uint32),
        ('grid_num_z', serialize.Uint32),
        ('pixel_format', serialize.String),
        ('grid_size', GridSize),
    ]
    #: :var uint32: Number of x axis grids in grid map data.
    grid_num_x = None
    #: :var uint32: Number of y axis grids in grid map data.
    grid_num_y = None
    #: :var uint32: Number of z axis grids in grid map data.
    grid_num_z = None
    #: :var str: Pixel format of the grid map.
    pixel_format = None
    #: :var GridSize: Size of grids.
    grid_size = None

    def __repr__(self):
        return ('%s(grid_num_x=%r, grid_num_y=%r, grid_num_z=%r, '
                'pixel_format=%r, grid_size=%r)') % (
                    self.__class__.__name__, self.grid_num_x, self.grid_num_y,
                    self.grid_num_z, self.pixel_format, self.grid_size)


class PointCloudProperty(serialize.Structure):
    """Point cloud Property."""
    #: Property key.
    KEY = b'point_cloud_property'

    _fields_ = [
        ('width', serialize.Uint32),
        ('height', serialize.Uint32),
        ('pixel_format', serialize.String),
    ]
    #: :var uint32: Width of the point cloud.
    width = None
    #: :var uint32: Height of the point cloud.
    height = None
    #: :var str: The format of a pixel.
    pixel_format = None

    def __repr__(self):
        return '%s(w=%r, h=%r, pixel_format=%r)' % (
            self.__class__.__name__, self.width, self.height,
            self.pixel_format)


class RegisterAccess8Property(serialize.Structure):
    """Property of standard register 8 bit read/write access."""
    #: Property key.
    KEY = b'register_access_8_property'

    class Element(serialize.Structure):
        """8 bit element"""
        _fields_ = [
            ('address', serialize.Uint64),
            ('data', serialize.Uint8),
        ]
        #: :var uint64: Target address.
        address = None
        #: :var uint8: Writing data or read data.
        data = None

        def __repr__(self):
            return '{ address:%s, data:%s }' % (
                hex(self.address), hex(self.data))

    _fields_ = [
        ('id', serialize.Uint32),
        ('element', (list, Element)),
    ]
    #: :var uint32: Register ID.
    id = None
    #: :var list(Element): List of element.
    element = None

    def __repr__(self):
        return '%s(id=%r, element=%r)' % (
            self.__class__.__name__, self.id, self.element)


class RegisterAccess16Property(serialize.Structure):
    """Property of standard register 16 bit read/write access."""
    #: Property key.
    KEY = b'register_access_16_property'

    class Element(serialize.Structure):
        """16 bit element"""
        _fields_ = [
            ('address', serialize.Uint64),
            ('data', serialize.Uint16),
        ]
        #: :var uint64: Target address.
        address = None
        #: :var uint16: Writing data or read data.
        data = None

        def __repr__(self):
            return '{ address:%s, data:%s }' % (
                hex(self.address), hex(self.data))

    _fields_ = [
        ('id', serialize.Uint32),
        ('element', (list, Element)),
    ]
    #: :var uint32: Register ID.
    id = None
    #: :var list(Element): List of element.
    element = None

    def __repr__(self):
        return '%s(id=%r, element=%r)' % (
            self.__class__.__name__, self.id, self.element)


class RegisterAccess32Property(serialize.Structure):
    """Property of standard register 32 bit read/write access."""
    #: Property key.
    KEY = b'register_access_32_property'

    class Element(serialize.Structure):
        """32 bit element"""
        _fields_ = [
            ('address', serialize.Uint64),
            ('data', serialize.Uint32),
        ]
        #: :var uint64: Target address.
        address = None
        #: :var uint32: Writing data or read data.
        data = None

        def __repr__(self):
            return '{ address:%s, data:%s }' % (
                hex(self.address), hex(self.data))

    _fields_ = [
        ('id', serialize.Uint32),
        ('element', (list, Element)),
    ]
    #: :var uint32: Register ID.
    id = None
    #: :var list(Element): List of element.
    element = None

    def __repr__(self):
        return '%s(id=%r, element=%r)' % (
            self.__class__.__name__, self.id, self.element)


class RegisterAccess64Property(serialize.Structure):
    """Property of standard register 64 bit read/write access."""
    #: Property key.
    KEY = b'register_access_64_property'

    class Element(serialize.Structure):
        """64 bit element"""
        _fields_ = [
            ('address', serialize.Uint64),
            ('data', serialize.Uint64),
        ]
        #: :var uint64: Target address.
        address = None
        #: :var uint64: Writing data or read data.
        data = None

        def __repr__(self):
            return '{ address:%s, data:%s }' % (
                hex(self.address), hex(self.data))

    _fields_ = [
        ('id', serialize.Uint32),
        ('element', (list, Element)),
    ]
    #: :var uint32: Register ID.
    id = None
    #: :var list(Element): List of element.
    element = None

    def __repr__(self):
        return '%s(id=%r, element=%r)' % (
            self.__class__.__name__, self.id, self.element)


class TemperatureInfo(serialize.Structure):
    """Temperature information."""
    _fields_ = [
        ('temperature', serialize.Float),
        ('description', serialize.String),
    ]
    #: :var float: Temperature data.
    temperature = None
    #: :var str: Description of sensor.
    description = None

    def __repr__(self):
        return '%s(temperature=%r, description=%r)' % (
            self.__class__.__name__,
            self.temperature, self.description)


class TemperatureProperty(serialize.Structure):
    """Property for temperature information."""
    #: Property key.
    KEY = b'temperature_property'

    _fields_ = [
        ('temperatures', (dict, (serialize.Uint32, TemperatureInfo))),
    ]
    #: :var dict: Information for each temperature sensor. (Key = Sensor id)
    temperatures = None

    def __repr__(self):
        return '%s(temperatures=%r)' % (
            self.__class__.__name__, self.temperatures)


class PolarizationDopCorrectionProperty(serialize.Structure):
    """Property used for calclation of degree of polarization."""
    #: Property key.
    KEY = b'polarization_dop_correction_property'

    _fields_ = [
        ('noise_model', serialize.Bool),
        ('analog_gain', serialize.Float),
        ('dop_gain', serialize.Float),
    ]
    #: :var bool: Support for position and attitude data.
    noise_model = None
    #: :var float: Gain for calclation of DOP.
    analog_gain = None
    #: :var float: Gain for display of DOP.
    dop_gain = None

    def __repr__(self):
        return '%s(noise_model=%r, analog_gain=%r, dop_gain=%r)' % (
            self.__class__.__name__, self.noise_model,
            self.analog_gain, self.dop_gain)


class PolarizationInvalidMaskProperty(serialize.Structure):
    """Property used for specify invalid pixel of dop and normal image."""
    #: Property key.
    KEY = b'polarization_invalid_mask_property'

    _fields_ = [
        ('enable', serialize.Bool),
        ('pixel_white_threshold', serialize.Uint16),
        ('pixel_black_threshold', serialize.Uint16),
    ]
    #: :var bool: Enble invalid pixel mask setting.
    enable = None
    #: :var uint16: Threshold of halation in pixels.
    pixel_white_threshold = None
    #: :var uint16: Threshold of black defects in pixels.
    pixel_black_threshold = None

    def __repr__(self):
        return ('%s(enable=%r, pixel_white_threshold=%r, '
                'pixel_black_threshold=%r)') % (
                    self.__class__.__name__, self.enable,
                    self.pixel_white_threshold, self.pixel_black_threshold)


class ColorType(_utils.CEnum):
    """Color types for Normal vector expression."""
    _names_ = {
        'RGB': 0,
        'HSV': 1,
    }
    _values_ = {
        0: 'RGB',
        1: 'HSV',
    }
    RGB = 0
    HSV = 1


class PolarizationNormalVectorProperty(serialize.Structure):
    """Property used for specify the expression for normal vector(RGB/HSV)."""
    #: Property key.
    KEY = b'polarization_normal_vector_property'

    _fields_ = [
        ('color_type', ColorType),
        ('rotation', serialize.Uint16),
    ]
    #: :var ColorType: Mode of expression for normal vector.
    color_type = None
    #: :var uint16: Hue offset for HSV expression.
    rotation = None

    def __repr__(self):
        return '%s(color_type=%r, rotation=%r)' % (
            self.__class__.__name__, self.color_type, self.rotation)


class PolarizationReflectionProperty(serialize.Structure):
    """Property used for specify reflection setting of polarized image."""
    #: Property key.
    KEY = b'polarization_reflection_property'

    _fields_ = [
        ('extraction_gain', serialize.Float),
    ]
    #: :var float: Gain for display extraction image.
    extraction_gain = None

    def __repr__(self):
        return '%s(extraction_gain=%r)' % (
            self.__class__.__name__, self.extraction_gain)


class TemporalContrastTriggerType(_utils.CEnum):
    """The trigger type for frame generation of TemporalContrastStream."""
    _names_ = {
        'TRIGGER_TYPE_TIME': 0,
        'TRIGGER_TYPE_EVENT': 1,
    }
    _values_ = {
        0: 'TRIGGER_TYPE_TIME',
        1: 'TRIGGER_TYPE_EVENT',
    }
    TRIGGER_TYPE_TIME = 0
    TRIGGER_TYPE_EVENT = 1


class PixelPolarityTriggerType(TemporalContrastTriggerType):
    """The trigger type for frame generation of PixelPolarityStream."""
    _names_ = {}
    _values_ = {}


class TemporalContrastDataProperty(serialize.Structure):
    """Property used for specify TemporalContrastStream's frame generation."""
    #: Property key.
    KEY = b'pixel_polarity_data_property'

    _fields_ = [
        ('trigger_type', TemporalContrastTriggerType),
        ('event_count', serialize.Uint32),
        ('accumulation_time', serialize.Uint32),
    ]
    #: :var TemporalContrastTriggerType: Frame generation trigger type.
    trigger_type = None
    #: :var uint32: event count used when trigger type is TRIGGER_TYPE_EVENT.
    event_count = None
    #: :var uint32: the time used for image frame generation [usec].
    accumulation_time = None

    def __repr__(self):
        return '%s(trigger_type=%r, event_count=%r, accumulation_time=%r)' % (
            self.__class__.__name__, self.trigger_type, self.event_count,
            self.accumulation_time)


class PixelPolarityDataProperty(TemporalContrastDataProperty):
    """Property used for specify PixelPolarityStream's frame generation."""
    pass


class RoiProperty(serialize.Structure):
    """Structure for ROI setting."""
    #: Property key.
    KEY = b'roi_property'

    _fields_ = [
        ('left', serialize.Uint32),
        ('top', serialize.Uint32),
        ('width', serialize.Uint32),
        ('height', serialize.Uint32),
    ]
    #: :var uint32: Horizontal offset of the top left corner of the cropping rectangle.
    left = None
    #: :var uint32: Vertical offset of the top left corner of the cropping rectangle.
    top = None
    #: :var uint32: Width of the cropping rectangle.
    width = None
    #: :var uint32: Height of the cropping rectangle.
    height = None

    def __repr__(self):
        return '%s(left=%r, top=%r, width=%r, height=%r)' % (
            self.__class__.__name__, self.left, self.top, self.width, self.height)


class ScoreThresholdProperty(serialize.Structure):
    """Structure for score threshold setting."""
    #: Property key.
    KEY = b'score_threshold_property'

    _fields_ = [
        ('score_threshold', serialize.Float),
    ]
    #: :var float: Threshold of score.
    score_threshold = None

    def __repr__(self):
        return '%s(score_threshold=%r)' % (
            self.__class__.__name__, self.score_threshold)


class VelocityDataUnitProperty(serialize.Structure):
    """Property for obtaining unit of RawData."""
    #: Property key.
    KEY = b'velocity_data_unit_property'

    _fields_ = [
        ('velocity', VelocityUnit),
    ]
    #: :var VelocityUnit: Unit of data of velocity.
    velocity = None

    def __repr__(self):
        return '%s(%r)' % (self.__class__.__name__, self.velocity)


class DataRateProperty(serialize.Structure):
    """Property for data rate."""
    KEY = b'data_rate_property'

    class Element(serialize.Structure):
        """Data rate element."""
        _fields_ = [
            ('size', serialize.Float),
            ('name', serialize.String),
            ('unit', serialize.String),
        ]
        #: :var Float: Size.
        size = None
        #: :var String: Name.
        name = None
        #: :var String: Unit.
        unit = None

        def __repr__(self):
            return '%s{ size:%r, name:%r, unit:%r }' % (
                self.__class__.__name__, self.size, self.name, self.unit)

    _fields_ = [
        ('elements', (list, Element)),
    ]
    #: :var list(Element): List of element.
    elements = None

    def __repr__(self):
        return '%s(element=%r)' % (
            self.__class__.__name__, self.elements)


class CoordinateSystemProperty(serialize.Structure):
    """Property showing the information of coordinate system."""
    #: Property key.
    KEY = b'coordinate_system_property'

    _fields_ = [
        ('handed', SystemHanded),
        ('up_axis', UpAxis),
        ('forward_axis', ForwardAxis),
    ]
    #: :var SystemHanded: System handed.
    handed = None
    #: :var UpAxis: Up axis information.
    up_axis = None
    #: :var ForwardAxis: Forward axis information.
    forward_axis = None

    def __repr__(self):
        return '%s(handed=%r, up_axis=%r, forward_axis=%r)' % (
            self.__class__.__name__, self.handed, self.up_axis,
            self.forward_axis)
