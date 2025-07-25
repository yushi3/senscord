# SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
#
# SPDX-License-Identifier: Apache-2.0

################################################################################

from __future__ import absolute_import
from __future__ import print_function

import ctypes
import senscord
import senscord.serialize


TEST_STREAM_KEY = b'pseudo_image_stream.0'
TEST_GET_FRAME_NUM = 20
TEST_CHANGE_PROPERTY_FRAME = 10
TEST_PROPERTY_KEY = b'PseudoImageProperty'
TEST_USER_DATA_SIZE = 16


class PseudoImageProperty(senscord.serialize.Structure):
    _fields_ = [
        ('x', senscord.serialize.Integer),
        ('y', senscord.serialize.Integer),
        ('z', senscord.serialize.String),
    ]


def callback_frame(stream, private_data):
    print('Frame arrived!!: stream=%r, private_data=%r' % (
        stream, private_data))

    stream_key = senscord.StreamKeyProperty()
    stream.get_property(senscord.StreamKeyProperty.KEY, stream_key)
    print(' - stream key: %r' % stream_key)

    stream_type = senscord.StreamTypeProperty()
    stream.get_property(senscord.StreamTypeProperty.KEY, stream_type)
    print(' - stream type: %r' % stream_type)


def callback_event(stream, event, param, private_data):
    print('Event arrived!!: stream=%r, event=%r, param=%r, private_data=%r' % (
        stream, event, param, private_data))


class ElementData(ctypes.Structure):
    _fields_ = [
        ('data', (ctypes.c_ubyte * 50))
    ]
    data = []


def do_register_access(stream):
    prop = senscord.RegisterAccess64Property()

    prop.id = 1000

    elem1 = senscord.RegisterAccess64Property.Element()
    elem1.address = 0x01234567
    elem1.data = 0x50505050
    prop.element.append(elem1)

    elem2 = senscord.RegisterAccess64Property.Element()
    elem2.address = 0x89ABCDEF
    elem2.data = 0x50505050
    prop.element.append(elem2)

    stream.get_property(senscord.RegisterAccess64Property.KEY, prop)

    print(' - id: %r' % prop.id)
    print(' - element.size: %r' % len(prop.element))

    for elem in prop.element:
        print('   - adr: 0x{:016x}'.format(elem.address))
        print('    data: 0x{:016x}'.format(elem.data))


def main():
    # Sample
    print('=== SensCord SmokeTest Player ===')

    core = senscord.Core()

    core.init()

    version = core.get_version()
    print('core.get_version: %r' % version)

    stream_list = core.get_stream_list()
    print('core.get_stream_list: %r' % len(stream_list))
    for stream_info in stream_list:
        print(' - %r' % (stream_info,))

    opened_count = core.get_opened_stream_count(TEST_STREAM_KEY)
    print('core.get_opened_stream_count: %r' % opened_count)

    image_stream = core.open_stream(TEST_STREAM_KEY)

    open_config = senscord.OpenStreamSetting()
    print('open_config=%r' % open_config)
    open_config.frame_buffering.buffering = senscord.Buffering.ON
    open_config.frame_buffering.num = 5
    open_config.frame_buffering.format = senscord.BufferingFormat.DISCARD
    print('open_config=%r' % open_config)

    image_stream2 = core.open_stream(TEST_STREAM_KEY, setting=open_config)

    opened_count = core.get_opened_stream_count(TEST_STREAM_KEY)
    print('stream.get_opened_stream_count: %r' % opened_count)

    image_stream.register_event_callback(
        senscord.EVENT_ERROR, callback_event, 0x100)

    image_stream.register_event_callback(
        senscord.EVENT_PROPERTY_UPDATED, callback_event, 0x200)

    image_stream.register_event_callback(
        senscord.EVENT_FRAME_DROPPED, callback_event, 0x300)

    # get_property(stream key)
    stream_key = senscord.StreamKeyProperty()
    image_stream.get_property(senscord.StreamKeyProperty.KEY, stream_key)
    print('stream.get_property(stream key): %r' % stream_key)

    # get_property(stream type)
    stream_type = senscord.StreamTypeProperty()
    image_stream.get_property(senscord.StreamTypeProperty.KEY, stream_type)
    print('stream.get_property(stream type): %r' % stream_type)

    # get_property(frame buffering stream1)
    buffering1 = senscord.FrameBufferingProperty()
    image_stream.get_property(senscord.FrameBufferingProperty.KEY, buffering1)
    print('stream.get_property(frame buffering[1]): %r' % buffering1)

    # get_property(frame buffering stream2)
    buffering2 = senscord.FrameBufferingProperty()
    image_stream2.get_property(senscord.FrameBufferingProperty.KEY, buffering2)
    print('stream.get_property(frame buffering[2]): %r' % buffering2)

    # get_property(stream state)
    stream_state = senscord.StreamStateProperty()
    image_stream.get_property(senscord.StreamStateProperty.KEY, stream_state)
    print('stream.get_property(stream_state): %r' % stream_state)

    property_list = image_stream.get_property_list()
    print('stream.get_property_list: %r' % len(property_list))
    for property_key in property_list:
        print(' - %r' % property_key)

    image_stream.register_frame_callback(callback_frame, 0x300)

    image_stream.start()

    image_stream2.start()

    # get_property(stream state)
    stream_state = senscord.StreamStateProperty()
    image_stream.get_property(senscord.StreamStateProperty.KEY, stream_state)
    print('stream.get_property(stream_state): %r' % stream_state)

    # property (structure)
    property_value = PseudoImageProperty()
    print('property_value=%r' % property_value)
    image_stream.get_property(TEST_PROPERTY_KEY, property_value)
    print('stream.get_property: %r' % property_value)

    property_value.x = 300
    property_value.y = 400
    property_value.z += b' fuga'
    image_stream.set_property(TEST_PROPERTY_KEY, property_value)

    image_stream.get_property(TEST_PROPERTY_KEY, property_value)
    print('stream.get_property(struct): %r' % property_value)

    # property (bytearray)
    property_value = bytearray(PseudoImageProperty().to_bytes())
    image_stream.lock_property(senscord.TIMEOUT_FOREVER)
    image_stream.get_property(TEST_PROPERTY_KEY, property_value)
    image_stream.unlock_property()
    print('stream.get_property(bytearray): %r' % property_value)

    # bytearray -> structure
    property_value2 = PseudoImageProperty().from_bytes(property_value)
    print('stream.get_property(bytearray->struct): %r' % property_value2)

    # register property
    try:
        do_register_access(image_stream)
    except senscord.errors.Error as error:
        print('do_register_access: %r' % error)

    # userdata property
    user_data = bytearray(TEST_USER_DATA_SIZE)
    image_stream.get_userdata_property(user_data)
    print('stream.get_userdata_property: %r' % user_data)

    for index in range(TEST_USER_DATA_SIZE):
        user_data[index] = index

    image_stream.set_userdata_property(user_data)

    user_data = bytearray(TEST_USER_DATA_SIZE)
    image_stream.get_userdata_property(user_data)
    print('stream.get_userdata_property: %r' % user_data)

    # get_property(current_frame_num)
    current_frame_num = senscord.CurrentFrameNumProperty()
    image_stream.get_property(senscord.CurrentFrameNumProperty.KEY,
                              current_frame_num)
    print('stream.get_property(current_frame_num): %r' % current_frame_num)

    for count in range(TEST_GET_FRAME_NUM):
        frame = image_stream.get_frame(senscord.TIMEOUT_FOREVER)

        number = frame.get_sequence_number()
        frame_type = frame.get_type()
        print(' - Frame: seq_num=%r, type=%r' % (number, frame_type))

        user_data = frame.get_user_data()
        print('   - %r' % user_data)
        if user_data.size != 0:
            print('     - %r' % user_data.get_bytes())

        # channel = frame.get_channel(0)
        # print("channel: %r" % channel)

        channel_list = frame.get_channel_list()
        for channel in channel_list:
            # print("channel: %r" % channel)

            channel_id = channel.get_channel_id()
            raw_data = channel.get_raw_data()
            print(' - Channel[%s]: %r' % (channel_id, raw_data))

            deserialized_data = None
            if raw_data.type == senscord.RAW_DATA_TYPE_ACCELERATION:
                deserialized_data = senscord.AccelerationData.from_bytes(
                    raw_data.get_bytes())
            elif raw_data.type == senscord.RAW_DATA_TYPE_ANGULAR_VELOCITY:
                deserialized_data = senscord.AngularVelocityData.from_bytes(
                    raw_data.get_bytes())
            elif raw_data.type == senscord.RAW_DATA_TYPE_MAGNETIC_FIELD:
                deserialized_data = senscord.MagneticFieldData.from_bytes(
                    raw_data.get_bytes())
            elif raw_data.type == senscord.RAW_DATA_TYPE_ROTATION:
                deserialized_data = senscord.RotationData.from_bytes(
                    raw_data.get_bytes())
            elif raw_data.type == senscord.RAW_DATA_TYPE_GRID_MAP:
                deserialized_data = senscord.GridMapData.from_bytes(
                    raw_data.get_bytes())

            if deserialized_data:
                print('   - rawdata: %r' % deserialized_data)

            stored_list = channel.get_property_list()
            print('   - property_list: %r' % len(stored_list))
            for property_key in stored_list:
                if property_key == TEST_PROPERTY_KEY:
                    property_value = PseudoImageProperty()
                else:
                    property_value = bytearray()
                channel.get_property(property_key, property_value)
                print('     - stored: %r' % property_key)
                print('       - %r' % property_value)

            updated_list = channel.get_updated_property_list()
            print('   - updated_property_list: %r' % len(updated_list))
            for property_key in updated_list:
                if property_key == TEST_PROPERTY_KEY:
                    property_value = PseudoImageProperty()
                else:
                    property_value = bytearray()
                channel.get_property(property_key, property_value)
                print('     - updated: %r' % property_key)
                print('       - %r' % property_value)

        image_stream.release_frame(frame)

        if count == TEST_CHANGE_PROPERTY_FRAME:
            # update property
            property_value = PseudoImageProperty()
            image_stream.get_property(TEST_PROPERTY_KEY, property_value)
            print('stream.get_property: %r' % property_value)

            property_value.x += 100
            property_value.y += 100
            property_value.z += b' piyo'
            image_stream.set_property(TEST_PROPERTY_KEY, property_value)

            # clear user data
            image_stream.set_userdata_property(None)

            image_stream.unregister_frame_callback()

    image_stream.stop()

    # get_property(stream state)
    stream_state = senscord.StreamStateProperty()
    image_stream.get_property(senscord.StreamStateProperty.KEY, stream_state)
    print('stream.get_property(stream_state): %r' % stream_state)

    clear_count = image_stream2.clear_frames()
    print('stream.clear_frames: %r' % clear_count)

    image_stream2.stop()

    core.close_stream(image_stream)
    core.close_stream(image_stream2)

    core.exit()


if __name__ == '__main__':
    main()
