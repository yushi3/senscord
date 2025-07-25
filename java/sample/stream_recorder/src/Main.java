/*
 * SPDX-FileCopyrightText: 2019-2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import com.sun.jna.ptr.PointerByReference;

import senscord.SenscordJavaAPI;
import senscord.SenscordJavaAPI.SenscordCLibrary;
import senscord_properties.ChannelInfoProperty;
import senscord_properties.RecordProperty;
import senscord_properties.RecorderListProperty;
import senscord_types.RecordNameRule;
import senscord_types.RecorderFormat;
import senscord_types.SensCordVersion;
import senscord_types.Status;

public class Main {

    static SenscordJavaAPI.SenscordCLibrary senscord = SenscordJavaAPI.SenscordCLibrary.INSTANCE;

    public static void main(String[] args) {

        PointerByReference core = new PointerByReference();

        int ret = 0;
        ret = senscord.senscord_core_init(core);
        System.out.println("senscord_core_init ret = " + ret);
        if (ret != 0) {
            PrintError();
        }

        SensCordVersion.ByReference version = new SensCordVersion.ByReference();

        ret = senscord.senscord_core_get_version(core.getValue(), version);
        System.out.println("senscord_core_get_version ret = " + ret);
        if (ret != 0) {
            PrintError();
        } else {
            System.out.println("version = " + version.GetSensCordVersion().GetName() +
                    " " + version.GetSensCordVersion().GetMajor() +
                    "." + version.GetSensCordVersion().GetMinor() +
                    "." + version.GetSensCordVersion().GetPatch() +
                    " " + version.GetSensCordVersion().GetDescription());
        }


        PointerByReference stream = new PointerByReference();
        String stream_key = "pseudo_image_stream.0";
//        String stream_key = "webcam_image_stream.0";

        ret = senscord.senscord_core_open_stream(core.getValue(), stream_key, stream);
        System.out.println("senscord_core_open_stream ret = " + ret);
        if (ret != 0) {
            PrintError();
        }

        /* Get Format */
        RecorderListProperty.ByReference list_property = new RecorderListProperty.ByReference();

        ret = senscord.senscord_stream_get_property(stream.getValue(),
                SenscordCLibrary.PropertyKey.SENSCORD_RECORDER_LIST_PROPERTY_KEY, list_property,
                list_property.size());
        System.out.println("senscord_stream_getproperty("
                + SenscordCLibrary.PropertyKey.SENSCORD_RECORDER_LIST_PROPERTY_KEY + ") ret = "
                + ret);
        if (ret != 0) {
            PrintError();
        } else {
            for (int i = 0; i < list_property.GetCount(); i++) {
                System.out.println(
                        "format[" + i + "] : " + list_property.GetRecorderFormat()[i].GetName());
            }
        }

        /* Get Channel Count */
        ChannelInfoProperty.ByReference channelinfo_property =
                new ChannelInfoProperty.ByReference();
        ret = senscord.senscord_stream_get_property(stream.getValue(),
                SenscordCLibrary.PropertyKey.SENSCORD_CHANNEL_INFO_PROPERTY_KEY,
                channelinfo_property, channelinfo_property.size());
        System.out.println("senscord_stream_getproperty("
                + SenscordCLibrary.PropertyKey.SENSCORD_CHANNEL_INFO_PROPERTY_KEY + ") ret = "
                + ret);
        if (ret != 0) {
            PrintError();
        } else {
            System.out.println("channel count : " + channelinfo_property.GetCount());
        }

        ret = senscord.senscord_stream_start(stream.getValue());
        System.out.println("senscord_stream_start ret = " + ret);
        if (ret != 0) {
            PrintError();
        }

        /* Record */
        RecordProperty.ByReference record_property = new RecordProperty.ByReference();
        record_property.SetEnabled((byte) 1);
        record_property.SetPath(System.getenv("RECORD_FILE_OUT_PATH"));
        record_property.SetBufferNum(5);

        for (int i = 0; i < channelinfo_property.GetCount(); i++) {
            record_property.GetRecordInfo()[i]
                    .SetChannelId(channelinfo_property.GetChannelInfo()[i].GetChannelId());
            record_property.SetInfoCount(i + 1);
            record_property.GetRecordInfo()[i].GetFormat()
                    .SetName(RecorderFormat.Format.SENSCORD_RECORDING_FORMAT_RAW);
        }

        if (record_property.GetInfoCount() == 0) {
            System.out.println("not record target");
        }

        record_property.GetNameRules()[0].SetDirectoryType(
            RecordNameRule.DirectoryType.SENSCORD_RECORD_DIRECTORY_TOP);
        record_property.GetNameRules()[0].SetFormat("");
        record_property.SetNameRulesCount(1);

        ret = senscord.senscord_stream_set_property(stream.getValue(),
                SenscordCLibrary.PropertyKey.SENSCORD_RECORD_PROPERTY_KEY, record_property,
                record_property.size());
        System.out.println("senscord_stream_set_property("
                + SenscordCLibrary.PropertyKey.SENSCORD_RECORD_PROPERTY_KEY + ") ret = " + ret);
        if (ret != 0) {
            PrintError();
        } else {
            System.out.println("rec started");
        }

        final int GET_FRAME_NUM = 2;
        PointerByReference frame = new PointerByReference();
        for (int i = 0; i < GET_FRAME_NUM; i++) {
            ret = senscord.senscord_stream_get_frame(stream.getValue(), frame, 1000);
            System.out.println("senscord_stream_get_frame ret = " + ret);
            if (ret != 0) {
                PrintError();
            }

            ret = senscord.senscord_stream_release_frame(stream.getValue(), frame.getValue());
            System.out.println("senscord_stream_release_frame ret = " + ret);
            if (ret != 0) {
                PrintError();
            }
        }

        ret = senscord.senscord_stream_stop(stream.getValue());
        System.out.println("senscord_stream_stop ret = " + ret);
        if (ret != 0) {
            PrintError();
        }

        ret = senscord.senscord_core_close_stream(core.getValue(), stream.getValue());
        System.out.println("senscord_core_close_stream ret = " + ret);
        if (ret != 0) {
            PrintError();
        }

        ret = senscord.senscord_core_exit(core.getValue());
        System.out.println("senscord_core_exit ret = " + ret);
        if (ret != 0) {
            PrintError();
        }

    }

    private static void PrintError() {
        Status.ByValue status = senscord.senscord_get_last_error();
        System.out.println("level = " + status.GetLevel() + "("
                + status.GetLevelString(status.GetLevel()) + ")" + ", cause = " + status.GetCause()
                + "(" + status.GetCauseString(status.GetCause()) + ")" + ", message = "
                + status.GetMessase() + ", block = " + status.GetBlock() + ", trace = "
                + status.GetTrace());
    }
}
