# SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

"""
Error definition of SensCord SDK.
"""

from __future__ import absolute_import

from senscord import _utils


class Error(Exception):
    """Base exception class."""


class InitializationError(Error):
    """Initialization error exception class."""


class OperationError(Error):
    """Operation error exception class."""


class ApiError(Error):
    """API error exception class."""
    def __init__(self, status):
        """Create a new ApiError.

        :param status: Error status.
        :type status: Status
        """
        super(ApiError, self).__init__()
        self._status_level = status.level
        self._status_cause = status.cause
        self._status_message = _utils.bytes_to_str(status.message)
        self._status_block = _utils.bytes_to_str(status.block)
        self._status_trace = _utils.bytes_to_str(status.trace)

    def level(self):
        """Get the level of error.

        :return: Level of error.
        :rtype: ErrorLevel
        """
        return self._status_level

    def cause(self):
        """Get the cause of error.

        :return: Cause of error.
        :rtype: ErrorCause
        """
        return self._status_cause

    def message(self):
        """Get the error message.

        :return: Error message.
        :rtype: str
        """
        return self._status_message

    def block(self):
        """Get the location where the error occurred.

        :return: Error block.
        :rtype: str
        """
        return self._status_block

    def trace(self):
        """Get the trace information.

        :return: Trace information.
        :rtype: str
        """
        return self._status_trace

    def __str__(self):
        return '%r[%r]: %s (%s)\n%s' % (
            self._status_level, self._status_cause, self._status_message,
            self._status_block, self._status_trace)

    def __repr__(self):
        return '%s(%r, %r, %s, %s, %s)' % (
            self.__class__.__name__,
            self._status_level, self._status_cause, self._status_message,
            self._status_block, self._status_trace)
