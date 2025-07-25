# SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

"""
Utility classes.
"""

from __future__ import absolute_import

import ctypes
import sys


class CEnumMeta(type(ctypes.c_int)):
    """Enum class that extends c_int."""
    def __new__(mcs, name, bases, namespace):
        cls = type(ctypes.c_int).__new__(mcs, name, bases, namespace)
        if namespace.get("__module__") != __name__:
            namespace["_values_"].clear()
            for value_name in namespace["_names_"].keys():
                if value_name.startswith("_"):
                    continue
                setattr(cls, value_name, cls(namespace[value_name]))
                namespace["_names_"][value_name] = namespace[value_name]
                namespace["_values_"][namespace[value_name]] = value_name
        return cls


def with_meta(meta, base=object):
    """Create a base class with a metaclass."""
    return meta("NewBase", (base,), {"__module__": __name__})


class CEnum(with_meta(CEnumMeta, ctypes.c_int)):
    """Python class equivalent to the enum of C/C++."""
    _names_ = {}
    _values_ = {}
    __slots__ = []

    def __repr__(self):
        name = self._values_.get(self.value)
        if name is None:
            return "%s(%r)" % (self.__class__.__name__, self.value)
        return "%s.%s" % (self.__class__.__name__, name)

    def __int__(self):
        return int(self.value)

    def __index__(self):
        return int(self)

    def __eq__(self, other):
        return int(self) == int(other)

    def __ne__(self, other):
        return int(self) != int(other)

    def __gt__(self, other):
        return int(self) > int(other)

    def __ge__(self, other):
        return int(self) >= int(other)

    def __lt__(self, other):
        return int(self) < int(other)

    def __le__(self, other):
        return int(self) <= int(other)

    def __or__(self, other):
        return int(self) | int(other)

    def __and__(self, other):
        return int(self) & int(other)

    def __hash__(self):
        return hash(int(self))


def bytes_to_str(value, encoding='utf-8'):
    """bytes to string"""
    if sys.version_info >= (3,):
        # Python 3
        try:
            return value.decode(encoding)
        except UnicodeError:
            return str(value)
    else:
        # Python 2
        return str(value)  # copy(clone)
