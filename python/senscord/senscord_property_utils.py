# SPDX-FileCopyrightText: 2022-2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

"""Property Utility API of SensCord SDK."""

from __future__ import absolute_import


class PropertyUtilsParam(object):
    """Utilities API for Property parameter manager."""
    #: Append information identifier.
    SET_CHANNEL_ID = "ch="


    def __init__(self, key):
        """Constructor.

        :param key: Property key.
        :type key: bytes.
        """
        self._append_dict = {}
        self._key = self._parse_key(key)


    def _parse_key(self, key):
        """Append information format check.

        :param key: Property key.
        :type key: bytes.
        :return: Append information format status.
        :rtype: str.
        """
        if isinstance(key, bytes):
            decode_key = key.decode('utf-8')
        elif isinstance(key, str):
            decode_key = key
        else:
            return ""

        spos = decode_key.find("[")
        epos = decode_key.rfind("]")

        if spos == -1:
            if epos == -1:
                return decode_key

            return ""

        if epos != (len(decode_key) - 1):
            return ""

        if spos == 0:
            return ""

        self._parse_append_info(decode_key)
        return decode_key[0:spos]


    def _parse_append_info(self, key):
        """Parse append information from property key.

        :param key: Property key.
        :type key: str.
        """
        last_spos = key.rfind("[")
        first_epos = key.find("]")

        append = key[last_spos + 1:first_epos]
        append_split = append.split(",")

        for append in append_split:
            pos = append.find("=")
            if pos != -1:
                self._append_dict[append[0:(pos + 1)]] = append[(pos + 1):len(append)]


    def set_channel_id(self, value):
        """Update append information.

        :value : Append value.
        :type value: int.
        """
        self._append_dict[self.SET_CHANNEL_ID] = str(value)


    def get_full_key(self):
        """Make property key with append information.

        :return: key + append information.
        :rtype: str.
        """
        if self._key == '':
            return ''

        count = 0
        make_key = '['

        for dict_key, value in self._append_dict.items():
            make_key = make_key + dict_key + value
            count = count + 1
            if count != len(self._append_dict):
                make_key = make_key + ","

        return self._key + make_key + "]"

class PropertyUtils(object):
    """Utilities API for Property."""
    @staticmethod
    def set_channel_id(key, channel_id):
        """Set Channel ID to property key.

        :param key: Property key.
        :type key: bytes.
        :param channel_id : Channel ID to be set to key.
        :type channel_id: int.
        :return: Channel ID to be set to key.
        :rtype: bytes.
        """
        if not key:
            return b""

        if not isinstance(channel_id, int):
            return b""

        if channel_id < 0 or channel_id > 0xFFFFFFFF:
            return b""

        utils = PropertyUtilsParam(key)
        utils.set_channel_id(channel_id)

        return utils.get_full_key().encode('utf-8')
