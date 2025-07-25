# SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

from __future__ import absolute_import
from __future__ import print_function

import senscord


TEST_STREAM_KEY = b'pseudo_image_stream.0'
TEST_GET_FRAME_NUM = 20


def main():
    # Sample
    print('=== SensCordSimpleStream Player ===')

    core = senscord.Core()

    core.init()

    version = core.get_version()
    print('core.get_version: %r' % version)

    stream = core.open_stream(TEST_STREAM_KEY)

    property_list = stream.get_property_list()
    print('stream.get_property_list: %r' % len(property_list))
    for property_key in property_list:
        print(' - %r' % property_key)

    stream.start()

    for count in range(TEST_GET_FRAME_NUM):
        frame = stream.get_frame(senscord.TIMEOUT_FOREVER)

        number = frame.get_sequence_number()
        frame_type = frame.get_type()
        print(' - Frame: seq_num=%r, type=%r' % (number, frame_type))

        channel_list = frame.get_channel_list()
        for channel in channel_list:
            # print("channel: %r" % channel)

            channel_id = channel.get_channel_id()
            raw_data = channel.get_raw_data()
            print(' - Channel[%s]: %r' % (channel_id, raw_data))

        stream.release_frame(frame)

    stream.stop()

    core.close_stream(stream)

    core.exit()


if __name__ == '__main__':
    main()
