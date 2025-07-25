# SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

"""Audio related properties."""

from __future__ import absolute_import

from senscord import _utils
from senscord import serialize


class AudioPcmFormat(_utils.CEnum):
    """PCM format."""
    _names_ = {
        'UNKNOWN': -1,
        'S8': 0,
        'U8': 1,
        'S16LE': 2,
        'S16BE': 3,
        'U16LE': 4,
        'U16BE': 5,
        'S24LE3': 6,
        'S24BE3': 7,
        'U24LE3': 8,
        'U24BE3': 9,
        'S24LE': 10,
        'S24BE': 11,
        'U24LE': 12,
        'U24BE': 13,
        'S32LE': 14,
        'S32BE': 15,
        'U32LE': 16,
        'U32BE': 17,
        'FLOAT32LE': 18,
        'FLOAT32BE': 19,
        'FLOAT64LE': 20,
        'FLOAT64BE': 21,
    }
    _values_ = {}
    #: Unknown format.
    UNKNOWN = -1
    #: Signed 8bit.
    S8 = 0
    #: Unsigned 8bit.
    U8 = 1
    #: Signed 16bit Little Endian.
    S16LE = 2
    #: Signed 16bit Big Endian.
    S16BE = 3
    #: Unsigned 16bit Little Endian.
    U16LE = 4
    #: Unsigned 16bit Big Endian.
    U16BE = 5
    #: Signed 24bit Little Endian (3 bytes format).
    S24LE3 = 6
    #: Signed 24bit Big Endian (3 bytes format).
    S24BE3 = 7
    #: Unsigned 24bit Little Endian (3 bytes format).
    U24LE3 = 8
    #: Unsigned 24bit Big Endian (3 bytes format).
    U24BE3 = 9
    #: Signed 24bit Little Endian (4 bytes format).
    S24LE = 10
    #: Signed 24bit Big Endian (4 bytes format).
    S24BE = 11
    #: Unsigned 24bit Little Endian (4 bytes format).
    U24LE = 12
    #: Unsigned 24bit Big Endian (4 bytes format).
    U24BE = 13
    #: Signed 32bit Little Endian.
    S32LE = 14
    #: Signed 32bit Big Endian.
    S32BE = 15
    #: Unsigned 32bit Little Endian.
    U32LE = 16
    #: Unsigned 32bit Big Endian.
    U32BE = 17
    #: Float 32bit Little Endian.
    FLOAT32LE = 18
    #: Float 32bit Big Endian.
    FLOAT32BE = 19
    #: Float 64bit Little Endian.
    FLOAT64LE = 20
    #: Float 64bit Big Endian.
    FLOAT64BE = 21

    __table = {
        # 0:byte, 1:bit, 2:sint, 3:uint, 4:float, 5:little, 6:big
        S8: (1, 8, True, False, False, True, True),
        U8: (1, 8, False, True, False, True, True),
        S16LE: (2, 16, True, False, False, True, False),
        S16BE: (2, 16, True, False, False, False, True),
        U16LE: (2, 16, False, True, False, True, False),
        U16BE: (2, 16, False, True, False, False, True),
        S24LE3: (3, 24, True, False, False, True, False),
        S24BE3: (3, 24, True, False, False, False, True),
        U24LE3: (3, 24, False, True, False, True, False),
        U24BE3: (3, 24, False, True, False, False, True),
        S24LE: (4, 24, True, False, False, True, False),
        S24BE: (4, 24, True, False, False, False, True),
        U24LE: (4, 24, False, True, False, True, False),
        U24BE: (4, 24, False, True, False, False, True),
        S32LE: (4, 32, True, False, False, True, False),
        S32BE: (4, 32, True, False, False, False, True),
        U32LE: (4, 32, False, True, False, True, False),
        U32BE: (4, 32, False, True, False, False, True),
        FLOAT32LE: (4, 32, False, False, True, True, False),
        FLOAT32BE: (4, 32, False, False, True, False, True),
        FLOAT64LE: (8, 64, False, False, True, True, False),
        FLOAT64BE: (8, 64, False, False, True, False, True),
    }

    def byte_width(self):
        """Returns the byte width."""
        value = self.__table.get(self)
        if value:
            return value[0]
        return 0

    def bits_per_sample(self):
        """Returns the number of bits per sample."""
        value = self.__table.get(self)
        if value:
            return value[1]
        return 0

    def is_signed(self):
        """Returns true if signed type."""
        value = self.__table.get(self)
        if value:
            return value[2]
        return False

    def is_unsigned(self):
        """Returns true if unsigned type."""
        value = self.__table.get(self)
        if value:
            return value[3]
        return False

    def is_float(self):
        """Returns true if float type."""
        value = self.__table.get(self)
        if value:
            return value[4]
        return False

    def is_little_endian(self):
        """Returns true if little endian."""
        value = self.__table.get(self)
        if value:
            return value[5]
        return False

    def is_big_endian(self):
        """Returns true if big endian."""
        value = self.__table.get(self)
        if value:
            return value[6]
        return False


class AudioProperty(serialize.Structure):
    """Structure containing information about the audio raw data."""
    #: Property key.
    KEY = b'audio_property'

    _fields_ = [
        ('format', serialize.String),
    ]
    #: :var format: Audio format.
    format = None


#: Audio format - Linear PCM.
AUDIO_FORMAT_LINEAR_PCM = b'audio_lpcm'


class AudioPcmProperty(serialize.Structure):
    """Structure containing information about the PCM."""
    #: Property key.
    KEY = b'audio_pcm_property'

    _fields_ = [
        ('channels', serialize.Integer),
        ('interleaved', serialize.Bool),
        ('format', AudioPcmFormat),
        ('samples_per_second', serialize.Integer),
        ('samples_per_frame', serialize.Integer),
    ]
    #: :var channels: Number of channels.
    channels = None
    #: :var interleaved: True: interleaved, False: non-interleaved.
    interleaved = None
    #: :var format: PCM format.
    format = None
    #: :var samples_per_second: Samples per second (e.g. 44100, 48000, ...)
    samples_per_second = None
    #: :var samples_per_frame: Samples per frame.
    samples_per_frame = None
