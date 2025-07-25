# SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

from __future__ import absolute_import
from __future__ import print_function

import argparse
import senscord
import senscord.serialize


# default values.
_DEFAULT_STREAM_KEY = b'pseudo_image_stream.0'
# _DEFAULT_STREAM_KEY = b'webcam_image_stream.0'
_DEFAULT_GET_FRAME_NUM = 20
_DEFAULT_GET_FRAME_WAIT_TIME = 3000  # msec


def main():
    print('=== SensCord Stream Recorder ===')

    # parse arguments.
    parser = argparse.ArgumentParser()
    parser.add_argument('-k', '--stream-key', default=_DEFAULT_STREAM_KEY)
    parser.add_argument('-f', '--format', default=b'raw')
    parser.add_argument('-o', '--output', default=b'.')
    parser.add_argument('-n', '--frame-num', type=int,
                        default=_DEFAULT_GET_FRAME_NUM)
    parser.add_argument('-t', '--name-rule', default=b'')
    parser.add_argument('--no-vendor', action='store_true')
    parser.add_argument('--silent', action='store_true')

    args = parser.parse_args()
    print(' - stream key: %r' % args.stream_key)
    print(' - format type: %r' % args.format)
    print(' - output path: %r' % args.output)
    print(' - get frame count: %r' % args.frame_num)
    print(' - top directory name rule: %r' % args.name_rule)
    print(" - enabled vendor's channels: %r" % (not args.no_vendor,))
    print(' - enabled silent: %r' % args.silent)

    core = senscord.Core()

    # core init
    core.init()

    # get version
    print('core.get_version: %r' % core.get_version())

    # open stream
    stream = core.open_stream(args.stream_key)

    # get_property(RecorderListProperty)
    format_list = senscord.RecorderListProperty()
    stream.get_property(format_list.KEY, format_list)
    print('stream.get_property(%r): %r' % (format_list.KEY, format_list))

    for format_type in format_list.formats:
        print(' - type: %r' % format_type)

    # get_property(ChannelInfoProperty)
    channel_info_list = senscord.ChannelInfoProperty()
    stream.get_property(channel_info_list.KEY, channel_info_list)
    print('stream.get_property(%r): %r' % (
        channel_info_list.KEY, channel_info_list))

    # start stream
    stream.start()

    # start recording
    record = senscord.RecordProperty()
    record.enabled = True
    record.path = args.output
    record.formats = dict()
    record.buffer_num = 5
    for channel_id in channel_info_list.channels.keys():
        # TODO: Python API (kChannelIdVendorBase)
        if args.no_vendor and channel_id >= 0x80000000:
            continue
        record.formats[channel_id] = args.format
    record.name_rules = dict()
    record.name_rules[senscord.RECORD_DIRECTORY_TOP] = args.name_rule

    print('Start recording. %r' % record)
    stream.set_property(record.KEY, record)

    # get frames
    for count in range(args.frame_num):
        try:
            frame = stream.get_frame(_DEFAULT_GET_FRAME_WAIT_TIME)
            if not args.silent:
                print('get_frame(): seq_num=%r' % frame.get_sequence_number())

            stream.release_frame(frame)
        except senscord.errors.Error as error:
            print('[Error] get_frame(): %r' % error)

    # stop stream
    stream.stop()

    # close stream
    core.close_stream(stream)

    # exit core
    core.exit()

    print('=== SensCord Stream Recorder End ===')


if __name__ == '__main__':
    main()
