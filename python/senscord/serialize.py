# SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

"""
Classes required for serialization.
"""

from __future__ import absolute_import

import sys
import copy
import io
import msgpack

from senscord import _utils

# left: Python 3.x or later, right: Python 2.x
_INTEGER_TYPE = int if sys.version_info >= (3,) else (int, long)
_UNICODE_TYPE = str if sys.version_info >= (3,) else unicode


def decode(encoded_bytes):
    """Decode from binary array to python object.

    :param encoded_bytes: Encoded byte-array.
    :type encoded_bytes: bytes or bytearray
    :return: python object.
    """
    return msgpack.unpackb(encoded_bytes, raw=True, strict_map_key=False)


class Serializable(object):
    """Serializable object."""

    def encode(self, encoder):
        """Encode this object."""
        raise NotImplementedError('Override')

    def decode(self, decoder):
        """Decode this object."""
        raise NotImplementedError('Override')

    @classmethod
    def from_bytes(cls, encoded_bytes):
        """Create an object from the encoded byte-array.

        :param encoded_bytes: Encoded byte-array.
        :type encoded_bytes: bytes or bytearray
        :return: New instance.
        """
        instance = cls()
        decoder = Decoder(encoded_bytes)
        instance.decode(decoder)
        return instance

    def to_bytes(self):
        """Returns an encoded byte-array from this object.

        :return: Encoded byte-array.
        :rtype: bytes
        """
        encoder = Encoder()
        self.encode(encoder)
        return encoder.to_bytes()

    def __eq__(self, other):
        if not isinstance(other, Serializable):
            return False
        return self.to_bytes() == other.to_bytes()

    def __ne__(self, other):
        return not self.__eq__(other)


class StructureMeta(type):
    """Meta class of Structure that parses the '_fields_' member."""
    def __new__(mcs, name, bases, namespace):
        cls = type.__new__(mcs, name, bases, namespace)
        if '_fields_' in namespace:
            for field in namespace['_fields_']:
                if len(field) != 2:
                    raise AttributeError('_fields_ parse error')
                # Add class member.
                member_name, class_type = field
                setattr(cls, member_name, class_type)
        return cls


class Structure(_utils.with_meta(StructureMeta, Serializable)):
    """Class that represents the structure.

    Example:
    _fields_ = [
        ('id', serialize.Int32),
        ('name', serialize.String),
    ]
    """
    _fields_ = []

    def __init__(self):
        for member_name, class_type in self._fields_:
            if isinstance(class_type, tuple):
                class_type = class_type[0]
            value = class_type()
            if isinstance(value, MutableValue):
                value = value.value
            # Add instance member.
            setattr(self, member_name, value)

    def encode(self, encoder):
        """Encode structure object."""
        encoder.push(self.to_dict())

    def to_dict(self):
        """Convert structure to dict"""
        output_dict = {}
        for member_name, class_type in self._fields_:
            value = getattr(self, member_name)
            value = self._encode(value, class_type)
            # member name must be unicode.
            member_unicode = member_name
            if not isinstance(member_unicode, _UNICODE_TYPE):
                member_unicode = member_name.decode()
            output_dict[member_unicode] = value
        return output_dict

    @staticmethod
    def _encode(value, class_type=None):
        if isinstance(class_type, tuple):
            current_class = class_type[0]
            class_type = class_type[1:]
        else:
            current_class = class_type
            class_type = None

        if isinstance(current_class, type):
            if issubclass(current_class, list):
                # In the case of the list, it encodes recursively.
                for index, element in enumerate(value):
                    value[index] = Structure._encode(element, class_type)
            elif issubclass(current_class, dict):
                # In the case of the dict, it encodes recursively.
                if isinstance(class_type, tuple) and len(class_type) == 1:
                    class_type = class_type[0]
                new_dict = {}
                for key, val in value.items():
                    key = Structure._encode(key, class_type[0])
                    new_dict[key] = Structure._encode(val, class_type[1])
                value = new_dict
        if isinstance(value, Structure):
            # In the case of the structure, conversion to the dict.
            value = value.to_dict()
        elif isinstance(value, _utils.CEnum):
            # In the case of the enumeration type, conversion to int type.
            value = int(value)
        return value

    def decode(self, decoder):
        """Decode structure object."""
        self.from_dict(decoder.pop())

    def from_dict(self, input_dict):
        """Convert from dict to structure."""
        for member_name, class_type in self._fields_:
            value = input_dict.get(member_name)
            if not value:
                # Try another string type.
                if isinstance(member_name, _UNICODE_TYPE):
                    tmp_name = member_name.encode()
                else:
                    tmp_name = member_name.decode()
                value = input_dict.get(tmp_name)
            if value:
                value = self._decode(value, class_type)
            setattr(self, member_name, value)

    @staticmethod
    def _decode(value, class_type=None):
        if isinstance(class_type, tuple):
            current_class = class_type[0]
            class_type = class_type[1:]
        else:
            current_class = class_type
            class_type = None

        if isinstance(current_class, type):
            if issubclass(current_class, list):
                # In the case of the list, it decodes recursively.
                for index, element in enumerate(value):
                    value[index] = Structure._decode(element, class_type)
            elif issubclass(current_class, dict):
                # In the case of the dict, it decodes recursively.
                if isinstance(class_type, tuple) and len(class_type) == 1:
                    class_type = class_type[0]
                new_dict = {}
                for key, val in value.items():
                    key = Structure._decode(key, class_type[0])
                    new_dict[key] = Structure._decode(val, class_type[1])
                value = new_dict
            elif issubclass(current_class, Structure):
                # In the case of the structure, to restore from the dict.
                new_value = current_class()
                new_value.from_dict(value)
                value = new_value
            elif issubclass(current_class, bytearray):
                value = bytearray(value)
        return value

    def __repr__(self):
        member_list = ', '.join('%s=%r' % (
            member, getattr(self, member)) for member, _ in self._fields_)
        return '%s(%s)' % (self.__class__.__name__, member_list)


class MutableValue(Serializable):
    """Mutable value object wrapping immutable values."""
    def __init__(self, value=None):
        self.value = value

    def encode(self, encoder):
        encoder.push(self.value)

    def decode(self, decoder):
        self.value = decoder.pop()

    def __repr__(self):
        return '%s(%r)' % (self.__class__.__name__, self.value)


class Integer(MutableValue):
    """Integer value object."""
    def __init__(self, value=int()):
        if not isinstance(value, _INTEGER_TYPE):
            raise TypeError('Invalid type')
        super(Integer, self).__init__(value)


class Int8(Integer):
    """deprecated: Please use Integer. Signed int8 value."""


class Int16(Integer):
    """deprecated: Please use Integer. Signed int16 value."""


class Int32(Integer):
    """deprecated: Please use Integer. Signed int32 value."""


class Int64(Integer):
    """deprecated: Please use Integer. Signed int64 value."""


class Uint8(Integer):
    """deprecated: Please use Integer. Unsigned int8 value."""


class Uint16(Integer):
    """deprecated: Please use Integer. Unsigned int16 value."""


class Uint32(Integer):
    """deprecated: Please use Integer. Unsigned int32 value."""


class Uint64(Integer):
    """deprecated: Please use Integer. Unsigned int64 value."""


class Float(MutableValue):
    """Float value."""
    def __init__(self, value=float()):
        if not isinstance(value, float):
            raise TypeError('Invalid type')
        super(Float, self).__init__(value)


class Double(Float):
    """deprecated: Please use Float. Double value."""


class Bool(MutableValue):
    """Boolean value."""
    def __init__(self, value=bool()):
        if not isinstance(value, bool):
            raise TypeError('Invalid type')
        super(Bool, self).__init__(value)


class String(MutableValue):
    """String value."""
    def __init__(self, value=bytes()):
        if not isinstance(value, (bytes, str)):
            raise TypeError('Invalid type')
        super(String, self).__init__(value)

    def encode(self, encoder):
        encoder.push_string(self.value)

    def decode(self, decoder):
        self.value = bytes(decoder.pop())


class Array(MutableValue):
    """Static array value."""
    def __init__(self, class_type=None, *args):
        """
        Create static array.

        :param class_type: The class type of the element.
        :type class_type: class
        :param args: Number of elements in the array.
                     For multidimensional arrays,
                     specify variable length arguments like N1, N2, N3...
        :type args: int...
        """
        tmp_class_type = [list for _ in range(len(args))]
        if not tmp_class_type:
            tmp_class_type.append(list)
        tmp_value = None
        if class_type:
            tmp_class_type.append(class_type)
            if args:
                tmp_value = class_type()
                if isinstance(tmp_value, MutableValue):
                    tmp_value = tmp_value.value
                for i in reversed(args):
                    tmp_value = [copy.deepcopy(tmp_value) for _ in range(i)]
        if not tmp_value:
            tmp_value = []
        super(Array, self).__init__(tmp_value)
        self._class_type = tuple(tmp_class_type)

    def __call__(self, value=None):
        """Copy array"""
        return copy.deepcopy(self)

    def encode(self, encoder):
        encoder.push(self.value, self._class_type)

    def decode(self, decoder):
        self.value = decoder.pop()


# Encode function table. (Value1=Class type, Value2=Function name)
_ENCODE_FUNC_TABLE = (
    (_utils.CEnum, 'push_int'),
    ((bytes, str), 'push_string'),
    (bytearray, 'push_bytes'),
    (set, 'push_array'),
)


class Encoder(object):
    """Basic encoder class."""
    def __init__(self):
        self._buffer = bytearray()

    def to_bytes(self):
        """ Returns an encoded byte-array.

        :return: Encoded byte-array.
        :rtype: bytes
        """
        return bytes(self._buffer)

    def _write(self, value):
        """Write a value to the buffer."""
        self._buffer.extend(msgpack.packb(value, use_bin_type=False))

    def push(self, value, class_type=None):
        """Encode the value.

        :param value: The value to encode.
        :type value: any
        :param class_type: The class type of the value.
        :type class_type: class or tuple(class...)
        """
        if isinstance(value, Serializable):
            value.encode(self)
            return

        for check_class, func_name in _ENCODE_FUNC_TABLE:
            if isinstance(value, check_class):
                func = getattr(self, func_name)
                func(value, class_type)
                return

        self._write(value)

    def push_int(self, value, *_args, **_kwargs):
        """Encode the integer value."""
        self._write(int(value))

    def push_string(self, value, *_args, **_kwargs):
        """Encode the string value."""
        if isinstance(value, _UNICODE_TYPE):
            str_value = value
        elif isinstance(value, bytes):
            str_value = value.decode()
        else:
            raise TypeError('Invalid type')
        self._write(str_value)

    def push_bytes(self, value, *_args, **_kwargs):
        """Encode the byte-array value."""
        self._write(bytes(value))

    def push_array(self, value, *_args, **_kwargs):
        """Encode the array(list) container."""
        if not isinstance(value, (list, tuple)):
            value = tuple(value)
        self._write(value)

    def push_pair(self, value, *_args, **_kwargs):
        """deprecated: Encode the pair value."""
        if len(value) != 2:
            raise TypeError('Invalid type')
        self._write(value)


class Decoder(object):
    """Basic decoder class."""
    def __init__(self, buf):
        buffer_size = 0
        if not isinstance(buf, io.BytesIO):
            buffer_size = len(buf)
            buf = io.BytesIO(buf)
        self._unpacker = msgpack.Unpacker(buf, max_buffer_size=buffer_size,
                                          raw=True, strict_map_key=False)

    def pop(self, *_args, **_kwargs):
        """Get the decoded value.

        :return: The object that stores the decoded value.
        """
        return self._unpacker.unpack()
