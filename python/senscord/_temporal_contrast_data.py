# SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

"""Raw data of SensCord SDK."""

from __future__ import absolute_import
import ctypes

from senscord import _utils
from senscord import _rawdata_type as _rawdata


#: TemporalContrastData header size.
TEMPORAL_CONTRAST_DATA_HEADER = 16

#: TemporalContrastEventsTimeslice header size.
TEMPORAL_CONTRAST_EVENTS_TIMESLICE_HEADER = 24


class TemporalContrast(_utils.CEnum):
    """Temporal contrast types for TemporalContrastEvent."""
    _names_ = {
        'NEGATIVE': -1,
        'NONE': 0,
        'POSITIVE': 1,
    }
    _values_ = {
        -1: 'NEGATIVE',
        0: 'NONE',
        1: 'POSITIVE',
    }
    NEGATIVE = -1
    NONE = 0
    POSITIVE = 1


class TemporalContrastEvent(ctypes.Structure):
    """Temporal Contrast Event."""
    _fields_ = [
        ('x', ctypes.c_uint16),
        ('y', ctypes.c_uint16),
        ('p', ctypes.c_int8),  # 0: None, 1: Positive, -1: Negative
        ('reserved', ctypes.c_uint8),
    ]
    x = None
    y = None
    p = None
    reserved = None

    def __repr__(self):
        return '%s(x=%r, y=%r, p=%r)' % (
            self.__class__.__name__, self.x, self.y, self.p)


class TemporalContrastEventArray:
    """List of TemporalContrastEvent"""
    def __init__(self, address, count):
        """Constructor.

        :param address: Rawdata address.
        :type address: int.
        :param count: Event count.
        :type count: int.
        """
        self.count = count
        self.event = ctypes.cast(
            address, ctypes.POINTER(TemporalContrastEvent))
        self.cursor = 0

    def __iter__(self):
        """iterator"""
        return self

    def __next__(self):
        """iterator

        :return: TemporalContrastEvent data.
        :rtype: TemporalContrastEvent.
        :raise: StopIteration: Last data at TemporalContrastEvent.
        """
        if self.count <= self.cursor:
            raise StopIteration
        result = self.event[self.cursor]
        self.cursor += 1
        return result

    def __getitem__(self, num):
        """__getitem__

        :param num: Event data slice value.
        :type num: Slice object.
        :return: TemporalContrastEvent data.
        :rtype: TemporalContrastEvent.
        :raise: IndexError: TemporalContrastEvent count over.
        :raise: TypeError: Index format not slice and int.
        """
        if isinstance(num, slice):
            return tuple(self.event[i] for i in range(*num.indices(self.count)))

        if isinstance(num, int):
            if self.count <= num or num < 0:
                raise IndexError('Index({0}) is out of range.'.format(num))

            return self.event[num]

        raise TypeError('Indices must be integers or slices.')


class TemporalContrastEventsTimeslice:
    """ Temporal Contrast EventsTimeslice."""
    def __init__(self, address, size):
        """Constructor.

        :param address: Rawdata address.
        :type address: int.
        :param size: Rawdata size.
        :type size: int.
        :raise: BufferError: Offset exceeded Rawdata size while creating
                             timeslce data list.
        """
        # Size check: Timeslice header size < size.
        if size < TEMPORAL_CONTRAST_EVENTS_TIMESLICE_HEADER:
            raise BufferError('Timeslice index buffer overrun at '
                              'EventsTimeslice header.')

        self.data_size = 0
        self.timestamp = ctypes.cast(
            address + 0, ctypes.POINTER(ctypes.c_uint64)).contents.value
        self.count = ctypes.cast(
            address + 8, ctypes.POINTER(ctypes.c_uint32)).contents.value

        # Size check: Timeslice data total size < size.
        self.data_size = self.count * ctypes.sizeof(TemporalContrastEvent) + \
            TEMPORAL_CONTRAST_EVENTS_TIMESLICE_HEADER
        if size < self.data_size:
            raise BufferError('Timeslice buffer overrun.')

        self.events = TemporalContrastEventArray(
            address + TEMPORAL_CONTRAST_EVENTS_TIMESLICE_HEADER, self.count)

    def total_size(self):
        """Get Temporal Contrast EventsTimeslice data size."""
        return self.data_size

    def __len__(self):
        """Get event count.

        :return: event count.
        :rtype: int.
        """
        return self.count

    def __repr__(self):
        return '%s(timestamp=%r, count=%r, events=%r)' % (
            self.__class__.__name__, self.timestamp, self.count, self.events)


class TemporalContrastData():
    """Utilities Class for TemporalContrastData."""

    def __init__(self, rawdata=None):
        """Constructor.

        :param rawdata: Channel rawdata.
        :type rawdata: RawData.
        """
        self.count = 0
        self.bundles = None

        if rawdata is None:
            return

        if rawdata.type != _rawdata.RAW_DATA_TYPE_TEMPORAL_CONTRAST:
            raise ValueError('Invalid RawData type {0}.'.format(rawdata.type))

        self.from_bytes(rawdata.address, rawdata.size)

    def from_bytes(self, address, size):
        """Create timeslice data list.

        :param address: Rawdata address.
        :type address: int.
        :param size: Rawdata size.
        :type size: int.
        :raise: BufferError: Offset exceeded Rawdata size while creating
                             timeslce data list.
        """
        if size < TEMPORAL_CONTRAST_DATA_HEADER:
            raise BufferError('size({0}) is smaller than '
                              'TemporalContrastData header size.'.format(size))

        count = ctypes.cast(
            address, ctypes.POINTER(ctypes.c_uint32)).contents.value
        bundles = []
        offset = TEMPORAL_CONTRAST_DATA_HEADER

        for _ in range(count):
            bundle = TemporalContrastEventsTimeslice(address + offset, size - offset)
            bundles.append(bundle)
            offset += bundle.total_size()

        self.count = count
        self.bundles = tuple(bundles)

    def __repr__(self):
        return '%s(count=%r, bundle=%r)' % (
            self.__class__.__name__, self.count, self.bundles)


class PixelPolarityData(TemporalContrastData):
    """Pixel polrity data."""
    pass


class PixelPolarityEvent(TemporalContrastEventArray):
    """Pixel polrity event."""
    pass


class PixelPolarityEventBundle(TemporalContrastEventsTimeslice):
    """Pixel polrity events with the same timestamp."""
    pass
