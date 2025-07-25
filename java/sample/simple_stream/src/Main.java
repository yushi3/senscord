/*
 * SPDX-FileCopyrightText: 2019-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import com.sun.jna.Memory;
import com.sun.jna.Pointer;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.LongByReference;
import com.sun.jna.ptr.PointerByReference;
import senscord.SenscordJavaAPI;
import senscord.SenscordJavaAPI.SenscordCLibrary;
import senscord_properties.AccelerationCalibProperty;
import senscord_properties.AccelerometerRangeProperty;
import senscord_properties.AngularVelocityCalibProperty;
import senscord_properties.BaselineLengthProperty;
import senscord_properties.CameraCalibrationProperty;
import senscord_properties.ChannelInfoProperty;
import senscord_properties.ChannelMaskProperty;
import senscord_properties.ColorSpaceProperty;
import senscord_properties.ConfidenceProperty;
import senscord_properties.CurrentFrameNumProperty;
import senscord_properties.DataRateProperty;
import senscord_properties.DepthProperty;
import senscord_properties.ExposureProperty;
import senscord_properties.FrameBufferingProperty;
import senscord_properties.FrameRateProperty;
import senscord_properties.GridMapProperty;
import senscord_properties.GridSizeProperty;
import senscord_properties.GridSizeProperty.GridUnit;
import senscord_properties.GyrometerRangeProperty;
import senscord_properties.ImageProperty;
import senscord_properties.ImageSensorFunctionProperty;
import senscord_properties.ImageSensorFunctionSupportedProperty;
import senscord_properties.ImuDataUnitProperty;
import senscord_properties.InitialPoseProperty;
import senscord_properties.InterlaceInfoProperty;
import senscord_properties.InterlaceProperty;
import senscord_properties.LensProperty;
import senscord_properties.MagneticFieldCalibProperty;
import senscord_properties.MagneticNorthCalibProperty;
import senscord_properties.MagnetometerRange3Property;
import senscord_properties.MagnetometerRangeProperty;
import senscord_properties.OdometryDataProperty;
import senscord_properties.PointCloudProperty;
import senscord_properties.PresetListProperty;
import senscord_properties.PresetProperty;
import senscord_properties.RegisterAccess16Property;
import senscord_properties.RegisterAccess32Property;
import senscord_properties.RegisterAccess64Property;
import senscord_properties.RegisterAccess8Property;
import senscord_properties.SamplingFrequencyProperty;
import senscord_properties.SkipFrameProperty;
import senscord_properties.SlamDataSupportedProperty;
import senscord_properties.StreamKeyProperty;
import senscord_properties.StreamStateProperty;
import senscord_properties.StreamTypeProperty;
import senscord_properties.VersionProperty;
import senscord_properties.WhiteBalanceProperty;
import senscord_types.OpenStreamSetting;
import senscord_types.PoseQuaternionData;
import senscord_types.RawData;
import senscord_types.RotationData;
import senscord_types.SensCordVersion;
import senscord_types.Status;
import senscord_types.StreamTypeInfo;
import senscord_types.UserData;
import senscord_types.Vector3Float;

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

        IntByReference count = new IntByReference();

        ret = senscord.senscord_core_get_stream_count(core.getValue(), count);
        System.out.println("senscord_core_get_stream_count ret = " + ret);
        if (ret != 0) {
            PrintError();
        } else {
            System.out.println("count = " + count.getValue());
        }

        for (int i = 0; i < count.getValue(); i++) {
            StreamTypeInfo.ByReference info = new StreamTypeInfo.ByReference();
            ret = senscord.senscord_core_get_stream_info(core.getValue(), i, info);
            System.out.println("senscord_core_stream_type_info ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("key = " + info.GetKey() + ", type = " + info.GetType());;
            }
        }


        PointerByReference stream = new PointerByReference();
        String stream_key = "pseudo_image_stream.0";
        // String stream_key = "webcam_image_stream.0";
        // String stream_key = "core_test_component_image1";
        // String stream_key = "test_component_stream.0";

        boolean open_with_buffer_test = false;

        if (!open_with_buffer_test) {
            ret = senscord.senscord_core_open_stream(core.getValue(), stream_key, stream);
            System.out.println("senscord_core_open_stream ret = " + ret);
            if (ret != 0) {
                PrintError();
            }
        } else {
            OpenStreamSetting.ByReference setting = new OpenStreamSetting.ByReference();
            setting.GetBuffuringProperty()
                    .SetBuffering(FrameBufferingProperty.Buffering.SENSCORD_BUFFERING_ON);
            setting.GetBuffuringProperty().SetNum(0);
            setting.GetBuffuringProperty()
                    .SetFormat(FrameBufferingProperty.Format.SENSCORD_BUFFERING_FORMAT_DISCARD);
            System.out.println("senscord_core_open_stream_with_setting ret = " + ret);
            ret = senscord.senscord_core_open_stream_with_setting(core.getValue(), stream_key,
                    setting, stream);
            if (ret != 0) {
                PrintError();
            }
        }

        ret = senscord.senscord_stream_lock_property(stream.getValue(), 100);
        System.out.println("senscord_stream_lock_property ret = " + ret);
        if (ret != 0) {
            PrintError();
        }

        ret = senscord.senscord_stream_unlock_property(stream.getValue());
        System.out.println("senscord_stream_unlock_property ret = " + ret);
        if (ret != 0) {
            PrintError();
        }

        ret = senscord.senscord_core_get_opened_stream_count(core.getValue(), stream_key, count);
        System.out.println("senscord_core_opened_stream_count ret = " + ret);
        if (ret != 0) {
            PrintError();
        } else {
            System.out.println("count = " + count.getValue());
        }

        ret = senscord.senscord_stream_register_event_callback(stream.getValue(),
                SenscordCLibrary.Event.SENSCORD_EVENT_FRAME_DROPPED,
                new SenscordCLibrary.senscord_event_received_callback_interface() {
                    @Override
                    public void invoke(String event, Pointer param, Pointer priv) {
                        System.out.println("Event arrived!! : " + event);
                    }
                }, null);
        System.out.println("senscord_stream_register_event_callback ret = " + ret);
        if (ret != 0) {
            PrintError();
        }

        boolean version_test = false;
        if (version_test) {
            VersionProperty.ByReference version_property = new VersionProperty.ByReference();
            version_property.SetName("");
            version_property.SetMajor(0);
            version_property.SetMinor(0);
            version_property.SetPatch(0);
            version_property.SetDescription("");

            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_VERSION_PROPERTY_KEY, version_property,
                    version_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_VERSION_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("name = " + version_property.GetName() + 
                        ", major = " + version_property.GetMajor() + ", minor = " +
                        version_property.GetMinor() + ", patch = " +
                        version_property.GetPatch() + ", description = " +
                        version_property.GetDescription());
            }

            version_property.SetName("hoge");
            version_property.SetMajor(10);
            version_property.SetMinor(20);
            version_property.SetPatch(30);
            version_property.SetDescription("fuga");
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_VERSION_PROPERTY_KEY, version_property,
                    version_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_VERSION_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            }
        }


        StreamKeyProperty.ByReference stream_key_property = new StreamKeyProperty.ByReference();
        ret = senscord.senscord_stream_get_property(stream.getValue(),
                SenscordCLibrary.PropertyKey.SENSCORD_STREAM_KEY_PROPERTY_KEY, stream_key_property,
                stream_key_property.size());
        System.out.println("senscord_stream_get_property ("
                + SenscordCLibrary.PropertyKey.SENSCORD_STREAM_KEY_PROPERTY_KEY + ") ret = " + ret);
        if (ret != 0) {
            PrintError();
        } else {
            System.out.println("senscord_stream_key = " + stream_key_property.GetKey());
        }

        StreamTypeProperty.ByReference stream_type_property = new StreamTypeProperty.ByReference();
        ret = senscord.senscord_stream_get_property(stream.getValue(),
                SenscordCLibrary.PropertyKey.SENSCORD_STREAM_TYPE_PROPERTY_KEY,
                stream_type_property, stream_type_property.size());
        System.out.println("senscord_stream_get_property ("
                + SenscordCLibrary.PropertyKey.SENSCORD_STREAM_TYPE_PROPERTY_KEY + ") ret = "
                + ret);
        if (ret != 0) {
            PrintError();
        } else {
            System.out.println("senscord_stream_type = " + stream_type_property.GetType());
        }

        StreamStateProperty.ByReference stream_state_property =
                new StreamStateProperty.ByReference();
        ret = senscord.senscord_stream_get_property(stream.getValue(),
                SenscordCLibrary.PropertyKey.SENSCORD_STREAM_STATE_PROPERTY_KEY,
                stream_state_property, stream_state_property.size());
        System.out.println("senscord_stream_get_property ("
                + SenscordCLibrary.PropertyKey.SENSCORD_STREAM_STATE_PROPERTY_KEY + ") ret = "
                + ret);
        if (ret != 0) {
            PrintError();
        } else {
            System.out.println("senscord_stream_state = " + stream_state_property.GetState() + "("
                    + stream_state_property.GetStateString(stream_state_property.GetState()) + ")");
        }

        FrameBufferingProperty.ByReference frame_buffering_property =
                new FrameBufferingProperty.ByReference();
        ret = senscord.senscord_stream_get_property(stream.getValue(),
                SenscordCLibrary.PropertyKey.SENSCORD_FRAME_BUFFERING_PROPERTY_KEY,
                frame_buffering_property, frame_buffering_property.size());
        System.out.println("senscord_stream_get_property ("
                + SenscordCLibrary.PropertyKey.SENSCORD_FRAME_BUFFERING_PROPERTY_KEY + ") ret = "
                + ret);
        if (ret != 0) {
            PrintError();
        } else {
            System.out.println("buffering = " + frame_buffering_property.GetBuffering() + ", num = "
                    + frame_buffering_property.GetNum() + ", format = "
                    + frame_buffering_property.GetFormat());
        }

        IntByReference property_count = new IntByReference();

        ret = senscord.senscord_stream_get_property_count(stream.getValue(), property_count);
        System.out.println("senscord_stream_get_property_count = " + ret);
        if (ret != 0) {
            PrintError();
        } else {
            System.out.println("count = " + property_count.getValue());
        }

        String[] stream_key_array = new String[1];
        for (int i = 0; i < property_count.getValue(); i++) {
            ret = senscord.senscord_stream_get_property_key(stream.getValue(), i, stream_key_array);
            System.out.println("senscord_stream_get_property_key = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("key[" + i + "] = " + stream_key_array[0]);
            }
        }

        ret = senscord.senscord_stream_start(stream.getValue());
        System.out.println("senscord_stream_start ret = " + ret);
        if (ret != 0) {
            PrintError();
        }


        UserData.ByReference user_data_set = new UserData.ByReference();
        Pointer u_data_set = new Memory(10);
        for (byte i = 0; i < 10; i++) {
            u_data_set.setByte(i, i);
            System.out.println("u_data_set :" + u_data_set.getByte(i));
        }
        user_data_set.SetAddress(u_data_set);
        user_data_set.SetSize(10);

        ret = senscord.senscord_stream_set_userdata_property(stream.getValue(), user_data_set,
                user_data_set.size);
        System.out.println("senscord_stream_set_userdata_property ret = " + ret);
        if (ret != 0) {
            PrintError();
        }

        UserData.ByReference user_data_get = new UserData.ByReference();
        Pointer u_data_get = new Memory(10);

        user_data_get.SetAddress(u_data_get);
        user_data_get.SetSize(10);
        ret = senscord.senscord_stream_get_userdata_property(stream.getValue(), user_data_get,
                user_data_get.size);
        System.out.println("senscord_stream_get_userdata_property ret = " + ret);
        if (ret != 0) {
            PrintError();
        } else {
            System.out.println("get size = " + user_data_get.GetSize());

            for (byte i = 0; i < 10; i++) {
                System.out.println("user_Data = " + user_data_get.GetAddress().getByte(i));
            }
        }



        FrameRateProperty.ByReference frame_rate_property = new FrameRateProperty.ByReference();
        frame_rate_property.SetNum(1);
        frame_rate_property.SetDenom(2);

        ret = senscord.senscord_stream_get_property(stream.getValue(),
                SenscordCLibrary.PropertyKey.SENSCORD_FRAME_RATE_PROPERTY_KEY, frame_rate_property,
                frame_rate_property.size());
        System.out.println("senscord_stream_get_property ("
                + SenscordCLibrary.PropertyKey.SENSCORD_FRAME_RATE_PROPERTY_KEY + ") ret = " + ret);
        if (ret != 0) {
            PrintError();
        } else {
            System.out.println("num = " + frame_rate_property.GetNum() + ", denom = "
                    + frame_rate_property.GetDenom());
        }

        boolean frame_rate_set_test = false;
        if (frame_rate_set_test) {
            frame_rate_property.SetNum(1);
            frame_rate_property.SetDenom(2);
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_FRAME_RATE_PROPERTY_KEY,
                    frame_rate_property, frame_rate_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_FRAME_RATE_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            }
        }

        ImageProperty.ByReference image_property = new ImageProperty.ByReference();
        image_property.SetWidth(1);
        image_property.SetHeight(2);
        image_property.SetStrideBytes(3);
        image_property.SetPixelFormat("hoge");

        ret = senscord.senscord_stream_get_property(stream.getValue(),
                SenscordCLibrary.PropertyKey.SENSCORD_IMAGE_PROPERTY_KEY, image_property,
                image_property.size());
        System.out.println("senscord_stream_get_property ("
                + SenscordCLibrary.PropertyKey.SENSCORD_IMAGE_PROPERTY_KEY + ") ret = " + ret);
        if (ret != 0) {
            PrintError();
        } else {
            System.out.println("width = " + image_property.GetWidth() + ", height = "
                    + image_property.GetHeight());
            System.out.println("stride_bytes = " + image_property.GetStrideBytes() + ", format = "
                    + image_property.GetPixelFormat());
        }

        ImageSensorFunctionSupportedProperty.ByReference image_sensor_supported_property =
                new ImageSensorFunctionSupportedProperty.ByReference();
        image_sensor_supported_property.SetAutoExposureSupported((byte) 1);
        image_sensor_supported_property.SetAutoWhiteBalanceSupported((byte) 1);
        image_sensor_supported_property.SetBrightessSupported((byte) 1);
        image_sensor_supported_property.SetIsoSensitivitySupported((byte) 1);
        image_sensor_supported_property.SetExposureTimeSupported((byte) 1);
        image_sensor_supported_property.SetExposureMeteringSupported((byte) 1);
        image_sensor_supported_property.SetGammaValueSupported((byte) 1);
        image_sensor_supported_property.SetGainValueSupported((byte) 1);
        image_sensor_supported_property.SetHueSupported((byte) 1);
        image_sensor_supported_property.SetSaturationSupported((byte) 1);
        image_sensor_supported_property.SetSharpnessSupported((byte) 1);
        image_sensor_supported_property.SetWhiteBalanceSupported((byte) 1);

        ret = senscord.senscord_stream_get_property(stream.getValue(),
                SenscordCLibrary.PropertyKey.SENSCORD_IMAGE_SENSOR_FUNCTION_SUPPORTED_PROPERTY_KEY,
                image_sensor_supported_property, image_sensor_supported_property.size());
        System.out.println("senscord_stream_get_property ("
                + SenscordCLibrary.PropertyKey.SENSCORD_IMAGE_SENSOR_FUNCTION_SUPPORTED_PROPERTY_KEY
                + ") ret = " + ret);
        if (ret != 0) {
            PrintError();
        } else {
            System.out.println(
                    "auto_exposure = " + image_sensor_supported_property.GetAutoExposureSupported()
                            + ", auto_white_balance = "
                            + image_sensor_supported_property.GetAutoWhiteBalanceSupported());
            System.out.println(
                    "brightness = " + image_sensor_supported_property.GetBrightessSupported()
                            + ", iso_sensitibity = "
                            + image_sensor_supported_property.GetIsoSensitivitySupported());
            System.out.println(
                    "exposure_time = " + image_sensor_supported_property.GetExposureTimeSupported()
                            + ", exposure metering = "
                            + image_sensor_supported_property.GetExposureMeteringSupported());
            System.out.println("gamma = " + image_sensor_supported_property.GetGammaValueSupported()
                    + ", gain = " + image_sensor_supported_property.GetGainValueSupported());
            System.out.println("hue = " + image_sensor_supported_property.GetHueSupported()
                    + ", saturation = " + image_sensor_supported_property.GetSaturationSupported());
            System.out.println("sharpness = "
                    + image_sensor_supported_property.GetSharpnessSupported() + ", white balance = "
                    + image_sensor_supported_property.GetWhiteBalanceSupported());
        }

        boolean image_sensor_supported_set_test = false;
        if (image_sensor_supported_set_test) {
            image_sensor_supported_property.SetAutoExposureSupported((byte) 1);
            image_sensor_supported_property.SetAutoWhiteBalanceSupported((byte) 1);
            image_sensor_supported_property.SetBrightessSupported((byte) 1);
            image_sensor_supported_property.SetIsoSensitivitySupported((byte) 1);
            image_sensor_supported_property.SetExposureTimeSupported((byte) 1);
            image_sensor_supported_property.SetExposureMeteringSupported((byte) 1);
            image_sensor_supported_property.SetGammaValueSupported((byte) 1);
            image_sensor_supported_property.SetGainValueSupported((byte) 1);
            image_sensor_supported_property.SetHueSupported((byte) 1);
            image_sensor_supported_property.SetSaturationSupported((byte) 1);
            image_sensor_supported_property.SetSharpnessSupported((byte) 1);
            image_sensor_supported_property.SetWhiteBalanceSupported((byte) 1);

            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_IMAGE_SENSOR_FUNCTION_SUPPORTED_PROPERTY_KEY,
                    image_sensor_supported_property, image_sensor_supported_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_IMAGE_SENSOR_FUNCTION_SUPPORTED_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            }
        }

        boolean image_sensor_function_test = false;
        if (image_sensor_function_test) {
            ImageSensorFunctionProperty.ByReference image_sensor_property =
                    new ImageSensorFunctionProperty.ByReference();
            image_sensor_property.SetAutoExposure((byte) 1);
            image_sensor_property.SetAutoWhiteBalance((byte) 1);
            image_sensor_property.SetBrightess(1);
            image_sensor_property.SetIsoSensitivity(100);
            image_sensor_property.SetExposureTime(2);
            image_sensor_property.SetExposureMetering(
                    ImageSensorFunctionProperty.Metering.SENSOCRD_EXPOSURE_METERING_CENTER_WEIGHTED);
            image_sensor_property.SetGammaValue(3.33f);
            image_sensor_property.SetGainValue(4);
            image_sensor_property.SetHue(5);
            image_sensor_property.SetSaturation(6);
            image_sensor_property.SetSharpness(7);
            image_sensor_property.SetWhiteBalance(8);

            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_IMAGE_SENSOR_FUNCTION_PROPERTY_KEY,
                    image_sensor_property, image_sensor_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_IMAGE_SENSOR_FUNCTION_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("auto_exposure = " + image_sensor_property.GetAutoExposure()
                        + ", auto_white_balance = " + image_sensor_property.GetAutoWhiteBalance());
                System.out.println("brightness = " + image_sensor_property.GetBrightess()
                        + ", iso_sensitibity = " + image_sensor_property.GetIsoSensitivity());
                System.out.println("exposure_time = " + image_sensor_property.GetExposureTime()
                        + ", exposure metering = " + image_sensor_property.GetExposureMetering());
                System.out.println("gamma = " + image_sensor_property.GetGammaValue() + ", gain = "
                        + image_sensor_property.GetGainValue());
                System.out.println("hue = " + image_sensor_property.GetHue() + ", saturation = "
                        + image_sensor_property.GetSaturation());
                System.out.println("sharpness = " + image_sensor_property.GetSharpness()
                        + ", white balance = " + image_sensor_property.GetWhiteBalance());
            }

            image_sensor_property.SetAutoExposure((byte) 1);
            image_sensor_property.SetAutoWhiteBalance((byte) 1);
            image_sensor_property.SetBrightess(1);
            image_sensor_property.SetIsoSensitivity(100);
            image_sensor_property.SetExposureTime(2);
            image_sensor_property.SetExposureMetering(
                    ImageSensorFunctionProperty.Metering.SENSOCRD_EXPOSURE_METERING_CENTER_WEIGHTED);
            image_sensor_property.SetGammaValue(3.33f);
            image_sensor_property.SetGainValue(4);
            image_sensor_property.SetHue(5);
            image_sensor_property.SetSaturation(6);
            image_sensor_property.SetSharpness(7);
            image_sensor_property.SetWhiteBalance(8);

            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_IMAGE_SENSOR_FUNCTION_PROPERTY_KEY,
                    image_sensor_property, image_sensor_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_IMAGE_SENSOR_FUNCTION_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            }
        }

        boolean exposure_test = false;
        if (exposure_test) {
            ExposureProperty.ByReference exposure_property = new ExposureProperty.ByReference();

            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_EXPOSURE_PROPERTY_KEY, exposure_property,
                    exposure_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_EXPOSURE_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("width = " + exposure_property.GetMode() + ", ev_compensation = "
                        + exposure_property.GetEvCompensation());
                System.out.println("exposure_time = " + exposure_property.GetExposureTime()
                        + ", iso_sensitivity = " + exposure_property.GetIsoSensitivity());
                System.out.println("metering = " + exposure_property.GetMetering());
            }

            exposure_property.SetMode(ExposureProperty.Mode.SENSCORD_EXPOSURE_MODE_HOLD);
            exposure_property.SetEvCompensation(0.1f);
            exposure_property.SetExposureTime(2);
            exposure_property.SetIsoSensitivity(3);
            exposure_property
                    .SetMetering(ExposureProperty.Metering.SENSOCRD_EXPOSURE_METERING_AVERAGE);

            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_EXPOSURE_PROPERTY_KEY, exposure_property,
                    exposure_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_EXPOSURE_PROPERTY_KEY + ") ret = "
                    + ret);
        }

        boolean white_balance_test = false;
        if (white_balance_test) {
            WhiteBalanceProperty.ByReference white_balance_property =
                    new WhiteBalanceProperty.ByReference();
            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_WHITE_BALANCE_PROPERTY_KEY,
                    white_balance_property, white_balance_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_WHITE_BALANCE_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("mode = " + white_balance_property.GetMode());
            }

            white_balance_property.SetMode(WhiteBalanceProperty.Mode.SENSCORD_WHITE_BALANCE_AUTO);
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_WHITE_BALANCE_PROPERTY_KEY,
                    white_balance_property, white_balance_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_WHITE_BALANCE_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            }
        }

        boolean camera_calibration_test = false;
        if (camera_calibration_test) {
            CameraCalibrationProperty.ByReference camera_calibration_property =
                    new CameraCalibrationProperty.ByReference();
            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_CAMERA_CALIBRATION_PROPERTY_KEY,
                    camera_calibration_property, camera_calibration_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_CAMERA_CALIBRATION_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {

                System.out.println("cx = "
                        + camera_calibration_property.GetCameraCalibrationParameter()[0]
                                .GetIntrinsicCalibrationParameter().GetCoordinateX()
                        + " cy = " + camera_calibration_property.GetCameraCalibrationParameter()[0]
                                .GetIntrinsicCalibrationParameter().GetCoordinateY());
                System.out.println("fx = "
                        + camera_calibration_property.GetCameraCalibrationParameter()[0]
                                .GetIntrinsicCalibrationParameter().GetFocalX()
                        + " fy = " + camera_calibration_property.GetCameraCalibrationParameter()[0]
                                .GetIntrinsicCalibrationParameter().GetFocalY());
                System.out.println(
                        "s = " + camera_calibration_property.GetCameraCalibrationParameter()[0]
                                .GetIntrinsicCalibrationParameter().GetSkew());

                System.out.println("r11 = "
                        + camera_calibration_property.GetCameraCalibrationParameter()[0]
                                .GetExtrinsicCalibrationParameter().GetR11()
                        + " r12 = "
                        + camera_calibration_property.GetCameraCalibrationParameter()[0]
                                .GetExtrinsicCalibrationParameter().GetR12()
                        + " r13 = " + camera_calibration_property.GetCameraCalibrationParameter()[0]
                                .GetExtrinsicCalibrationParameter().GetR13());

                System.out.println("r21 = "
                        + camera_calibration_property.GetCameraCalibrationParameter()[0]
                                .GetExtrinsicCalibrationParameter().GetR21()
                        + " r22 = "
                        + camera_calibration_property.GetCameraCalibrationParameter()[0]
                                .GetExtrinsicCalibrationParameter().GetR22()
                        + " r23 = " + camera_calibration_property.GetCameraCalibrationParameter()[0]
                                .GetExtrinsicCalibrationParameter().GetR23());

                System.out.println("r31 = "
                        + camera_calibration_property.GetCameraCalibrationParameter()[0]
                                .GetExtrinsicCalibrationParameter().GetR31()
                        + " r32 = "
                        + camera_calibration_property.GetCameraCalibrationParameter()[0]
                                .GetExtrinsicCalibrationParameter().GetR32()
                        + " r33 = " + camera_calibration_property.GetCameraCalibrationParameter()[0]
                                .GetExtrinsicCalibrationParameter().GetR33());

                System.out.println("t1 = "
                        + camera_calibration_property.GetCameraCalibrationParameter()[0]
                                .GetExtrinsicCalibrationParameter().GetT1()
                        + " t2 = "
                        + camera_calibration_property.GetCameraCalibrationParameter()[0]
                                .GetExtrinsicCalibrationParameter().GetT2()
                        + " t3 = " + camera_calibration_property.GetCameraCalibrationParameter()[0]
                                .GetExtrinsicCalibrationParameter().GetT3());

                System.out.println("k1 = "
                        + camera_calibration_property.GetCameraCalibrationParameter()[0]
                                .GetDistortionCalibrationParameter().GetK1()
                        + " k2 = "
                        + camera_calibration_property.GetCameraCalibrationParameter()[0]
                                .GetDistortionCalibrationParameter().GetK2()
                        + " k3 = "
                        + camera_calibration_property.GetCameraCalibrationParameter()[0]
                                .GetDistortionCalibrationParameter().GetK3()
                        + " k4 = "
                        + camera_calibration_property.GetCameraCalibrationParameter()[0]
                                .GetDistortionCalibrationParameter().GetK4()
                        + " k5 = "
                        + camera_calibration_property.GetCameraCalibrationParameter()[0]
                                .GetDistortionCalibrationParameter().GetK5()
                        + " k6 = " + camera_calibration_property.GetCameraCalibrationParameter()[0]
                                .GetDistortionCalibrationParameter().GetK6());

                System.out.println("p1 = "
                        + camera_calibration_property.GetCameraCalibrationParameter()[0]
                                .GetDistortionCalibrationParameter().GetP1()
                        + " p2 = " + camera_calibration_property.GetCameraCalibrationParameter()[0]
                                .GetDistortionCalibrationParameter().GetP2());

            }

            camera_calibration_property.GetCameraCalibrationParameter()[0]
                    .GetIntrinsicCalibrationParameter().SetCoordinateX(11.11f);
            camera_calibration_property.GetCameraCalibrationParameter()[0]
                    .GetIntrinsicCalibrationParameter().SetCoordinateY(22.22f);
            camera_calibration_property.GetCameraCalibrationParameter()[0]
                    .GetIntrinsicCalibrationParameter().SetFocalX(33.33f);
            camera_calibration_property.GetCameraCalibrationParameter()[0]
                    .GetIntrinsicCalibrationParameter().SetFocalY(44.44f);
            camera_calibration_property.GetCameraCalibrationParameter()[0]
                    .GetIntrinsicCalibrationParameter().SetSkew(55.55f);

            camera_calibration_property.GetCameraCalibrationParameter()[0]
                    .GetExtrinsicCalibrationParameter().SetR11(0);
            camera_calibration_property.GetCameraCalibrationParameter()[0]
                    .GetExtrinsicCalibrationParameter().SetR12(0);
            camera_calibration_property.GetCameraCalibrationParameter()[0]
                    .GetExtrinsicCalibrationParameter().SetR13(0);
            camera_calibration_property.GetCameraCalibrationParameter()[0]
                    .GetExtrinsicCalibrationParameter().SetR21(0);
            camera_calibration_property.GetCameraCalibrationParameter()[0]
                    .GetExtrinsicCalibrationParameter().SetR22(0);
            camera_calibration_property.GetCameraCalibrationParameter()[0]
                    .GetExtrinsicCalibrationParameter().SetR23(0);
            camera_calibration_property.GetCameraCalibrationParameter()[0]
                    .GetExtrinsicCalibrationParameter().SetR31(0);
            camera_calibration_property.GetCameraCalibrationParameter()[0]
                    .GetExtrinsicCalibrationParameter().SetR32(0);
            camera_calibration_property.GetCameraCalibrationParameter()[0]
                    .GetExtrinsicCalibrationParameter().SetR33(0);
            camera_calibration_property.GetCameraCalibrationParameter()[0]
                    .GetExtrinsicCalibrationParameter().SetT1(0);
            camera_calibration_property.GetCameraCalibrationParameter()[0]
                    .GetExtrinsicCalibrationParameter().SetT2(0);
            camera_calibration_property.GetCameraCalibrationParameter()[0]
                    .GetExtrinsicCalibrationParameter().SetT3(0);

            camera_calibration_property.GetCameraCalibrationParameter()[0]
                    .GetDistortionCalibrationParameter().SetK1(11.11f);
            camera_calibration_property.GetCameraCalibrationParameter()[0]
                    .GetDistortionCalibrationParameter().SetK2(12.11f);
            camera_calibration_property.GetCameraCalibrationParameter()[0]
                    .GetDistortionCalibrationParameter().SetK3(13.11f);
            camera_calibration_property.GetCameraCalibrationParameter()[0]
                    .GetDistortionCalibrationParameter().SetK4(21.11f);
            camera_calibration_property.GetCameraCalibrationParameter()[0]
                    .GetDistortionCalibrationParameter().SetK5(22.11f);
            camera_calibration_property.GetCameraCalibrationParameter()[0]
                    .GetDistortionCalibrationParameter().SetK6(23.11f);
            camera_calibration_property.GetCameraCalibrationParameter()[0]
                    .GetDistortionCalibrationParameter().SetP1(31.11f);
            camera_calibration_property.GetCameraCalibrationParameter()[0]
                    .GetDistortionCalibrationParameter().SetP2(32.11f);
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_CAMERA_CALIBRATION_PROPERTY_KEY,
                    camera_calibration_property, camera_calibration_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_CAMERA_CALIBRATION_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            }
        }

        boolean color_space_test = false;
        if (color_space_test) {
            ColorSpaceProperty.ByReference color_space_property =
                    new ColorSpaceProperty.ByReference();
            color_space_property
                    .SetEncoding(ColorSpaceProperty.Encoding.SENSCORD_YCBCR_ENCODING_BT601);
            color_space_property.SetQuantization(
                    ColorSpaceProperty.Quantization.SENSCORD_YCBCR_QUANTIZATION_FULL_RANGE);

            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_COLOR_SPACE_PROPERTY_KEY,
                    color_space_property, color_space_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_COLOR_SPACE_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("encoding = " + color_space_property.GetEncoding()
                        + ", quantization = " + color_space_property.GetQuantization());
            }

            color_space_property
                    .SetEncoding(ColorSpaceProperty.Encoding.SENSCORD_YCBCR_ENCODING_BT601);
            color_space_property.SetQuantization(
                    ColorSpaceProperty.Quantization.SENSCORD_YCBCR_QUANTIZATION_FULL_RANGE);
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_COLOR_SPACE_PROPERTY_KEY,
                    color_space_property, color_space_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_COLOR_SPACE_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            }
        }

        boolean lens_test = false;
        if (lens_test) {
            LensProperty.ByReference lens_property = new LensProperty.ByReference();
            lens_property.SetHorizontal(11.11f);
            lens_property.SetVertical(22.22f);

            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_LENS_PROPERTY_KEY, lens_property,
                    lens_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_LENS_PROPERTY_KEY + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("horizontal = " + lens_property.GetHorizontal() + ", vertical = "
                        + lens_property.GetVertical());
            }

            lens_property.SetHorizontal(11.11f);
            lens_property.SetVertical(22.22f);
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_LENS_PROPERTY_KEY, lens_property,
                    lens_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_LENS_PROPERTY_KEY + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            }
        }

        boolean baseline_test = false;
        if (baseline_test) {
            BaselineLengthProperty.ByReference baseline_property =
                    new BaselineLengthProperty.ByReference();
            baseline_property.SetLength(12.11f);

            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_BASELINE_LENGTH_PROPERTY_KEY,
                    baseline_property, baseline_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_BASELINE_LENGTH_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("length = " + baseline_property.GetLength());
            }

            baseline_property.SetLength(12.11f);
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_BASELINE_LENGTH_PROPERTY_KEY,
                    baseline_property, baseline_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_BASELINE_LENGTH_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            }
        }

        boolean interlace_info_test = false;
        if (interlace_info_test) {
            InterlaceInfoProperty.ByReference interlace_info_property =
                    new InterlaceInfoProperty.ByReference();
            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_INTERLACE_INFO_PROPERTY_KEY,
                    interlace_info_property, interlace_info_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_INTERLACE_INFO_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("order = " + interlace_info_property.GetOrder());
            }

            interlace_info_property
                    .SetOrder(InterlaceInfoProperty.Order.SENSCORD_INTERLACE_ORDER_TOP_FIRST);
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_INTERLACE_INFO_PROPERTY_KEY,
                    interlace_info_property, interlace_info_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_INTERLACE_INFO_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            }
        }


        boolean interlace_test = false;
        if (interlace_test) {
            InterlaceProperty.ByReference interlace_property = new InterlaceProperty.ByReference();
            interlace_property.SetField(InterlaceProperty.Feild.SENSCORD_INTERLACE_FIELD_TOP);
            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_INTERLACE_PROPERTY_KEY,
                    interlace_property, interlace_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_INTERLACE_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("field  = " + interlace_property.GetField());
            }

            interlace_property.SetField(InterlaceProperty.Feild.SENSCORD_INTERLACE_FIELD_TOP);
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_INTERLACE_PROPERTY_KEY,
                    interlace_property, interlace_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_INTERLACE_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            }
        }


        boolean register_test = false;
        if (register_test) {
            int id = 1000;
            long address = 0x01234567;
            RegisterAccess64Property.ByReference register_64_property =
                    new RegisterAccess64Property.ByReference();

            long data_64 = 0x100000001L;
            register_64_property.SetId(id);
            register_64_property.SetAddress(address);
            register_64_property.SetData(data_64);

            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_REGISTER_ACCESS_64_PROPERTY_KEY,
                    register_64_property, register_64_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_REGISTER_ACCESS_64_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("id = " + register_64_property.GetId() + ", address = "
                        + register_64_property.GetAddress() + ", data = "
                        + register_64_property.GetData());
            }

            register_64_property.SetId(id);
            register_64_property.SetAddress(address);
            register_64_property.SetData(data_64);
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_REGISTER_ACCESS_64_PROPERTY_KEY,
                    register_64_property, register_64_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_REGISTER_ACCESS_64_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            }

            RegisterAccess32Property.ByReference register_32_property =
                    new RegisterAccess32Property.ByReference();

            int data_32 = 0x00000001;
            register_32_property.SetId(id);
            register_32_property.SetAddress(address);
            register_32_property.SetData(data_32);

            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_REGISTER_ACCESS_32_PROPERTY_KEY,
                    register_32_property, register_32_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_REGISTER_ACCESS_32_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("id = " + register_32_property.GetId() + ", address = "
                        + register_32_property.GetAddress() + ", data = "
                        + register_32_property.GetData());
            }

            register_32_property.SetId(id);
            register_32_property.SetAddress(address);
            register_32_property.SetData(data_32);
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_REGISTER_ACCESS_32_PROPERTY_KEY,
                    register_32_property, register_32_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_REGISTER_ACCESS_32_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            }

            RegisterAccess16Property.ByReference register_16_property =
                    new RegisterAccess16Property.ByReference();

            short data_16 = 0x0001;
            register_16_property.SetId(id);
            register_16_property.SetAddress(address);
            register_16_property.SetData(data_16);

            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_REGISTER_ACCESS_16_PROPERTY_KEY,
                    register_16_property, register_16_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_REGISTER_ACCESS_16_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("id = " + register_16_property.GetId() + ", address = "
                        + register_16_property.GetAddress() + ", data = "
                        + register_16_property.GetData());
            }

            register_16_property.SetId(id);
            register_16_property.SetAddress(address);
            register_16_property.SetData(data_16);
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_REGISTER_ACCESS_16_PROPERTY_KEY,
                    register_16_property, register_16_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_REGISTER_ACCESS_16_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            }

            RegisterAccess8Property.ByReference register_8_property =
                    new RegisterAccess8Property.ByReference();

            byte data_8 = 0x01;
            register_8_property.SetId(id);
            register_8_property.SetAddress(address);
            register_8_property.SetData(data_8);

            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_REGISTER_ACCESS_8_PROPERTY_KEY,
                    register_8_property, register_8_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_REGISTER_ACCESS_8_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("id = " + register_8_property.GetId() + ", address = "
                        + register_8_property.GetAddress() + ", data = "
                        + register_8_property.GetData());
            }

            register_8_property.SetId(id);
            register_8_property.SetAddress(address);
            register_8_property.SetData(data_8);
            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_REGISTER_ACCESS_8_PROPERTY_KEY,
                    register_8_property, register_8_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_REGISTER_ACCESS_8_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            }
        }

        PointerByReference frame = new PointerByReference();

        boolean frame_drop_test = false;
        for (int i = 0; i < 10; i++) {
            ret = senscord.senscord_stream_get_frame(stream.getValue(), frame, 1000);
            System.out.println("senscord_stream_get_frame ret = " + ret);
            if (ret != 0) {
                PrintError();
            }

            LongByReference frame_num = new LongByReference();
            ret = senscord.senscord_frame_get_sequence_number(frame.getValue(), frame_num);
            System.out.println("senscord_frame_get_sequence_number ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("sequence num = " + frame_num.getValue());
            }

            String[] frame_type = new String[1];
            ret = senscord.senscord_frame_get_type(frame.getValue(), frame_type);
            System.out.println("senscord_frame_get_frame_type ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("type = " + frame_type[0]);
            }


            UserData.ByReference user_data = new UserData.ByReference();
            ret = senscord.senscord_frame_get_user_data(frame.getValue(), user_data);
            System.out.println("senscord_frame_get_frame_get_user_data ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("size = " + user_data.GetSize());
                Pointer u = new Pointer(user_data.GetAddress().getLong(0L));
                for (byte j = 0; j < user_data.GetSize(); j++) {
                    System.out.println("user_data = " + u.getByte(j));
                }
            }

            CurrentFrameNumProperty.ByReference current_frame_num =
                    new CurrentFrameNumProperty.ByReference();
            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_CURRENT_FRAME_NUM_PROPERTY_KEY,
                    current_frame_num, current_frame_num.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_CURRENT_FRAME_NUM_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("arrived num = " + current_frame_num.GetArrivedNumber());
                System.out.println("received num = " + current_frame_num.GetReceivedNumber());
            }

            ChannelInfoProperty.ByReference channel_info_property =
                    new ChannelInfoProperty.ByReference();
            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_CHANNEL_INFO_PROPERTY_KEY,
                    channel_info_property, channel_info_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_CHANNEL_INFO_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("count = " + channel_info_property.GetCount());
                for (int j = 0; j < channel_info_property.GetCount(); j++) {
                    System.out.println("id = "
                            + channel_info_property.GetChannelInfo()[j].GetChannelId() + ", type = "
                            + channel_info_property.GetChannelInfo()[j].GetRawDataTypeString());

                    System.out.println("description = "
                            + channel_info_property.GetChannelInfo()[j].GetDescriptionString());
                }
            }

            ChannelMaskProperty.ByReference channel_mask_property =
                    new ChannelMaskProperty.ByReference();
            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_CHANNEL_MASK_PROPERTY_KEY,
                    channel_mask_property, channel_mask_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_CHANNEL_MASK_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("count = " + channel_mask_property.GetCount());
                for (int j = 0; j < channel_mask_property.GetCount(); j++) {
                    System.out.println("id = " + channel_mask_property.GetChannels()[i]);
                }
            }

            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_CHANNEL_MASK_PROPERTY_KEY,
                    channel_mask_property, channel_mask_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_CHANNEL_MASK_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            }

            if (frame_drop_test && open_with_buffer_test) {
                try {
                    Thread.sleep(100);
                } catch (Exception e) {

                }
            }

            IntByReference cleared_num = new IntByReference((int) frame_num.getValue());
            ret = senscord.senscord_stream_clear_frames(stream.getValue(), cleared_num);
            System.out.println("senscord_stream_clear_frames ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("clear num  = " + cleared_num.getValue());
            }

        }

        if (open_with_buffer_test) {
            ret = senscord.senscord_stream_register_frame_callback(stream.getValue(),
                    new SenscordJavaAPI.SenscordCLibrary.senscord_frame_callback_interface() {
                        @Override
                        public void invoke(Pointer stream, Pointer arg) {
                            System.out.println("frame_callback_called");
                        }
                    }, null);

            System.out.println("senscord_stream_register_frame_callback ret = " + ret);
            if (ret != 0) {
                PrintError();
            }

            try {
                Thread.sleep(1000);
            } catch (Exception e) {
            }

        }

        boolean preset_list_test = false;
        if (preset_list_test) {
            int preset_count = 3;
            PresetListProperty.ByReference preset_list_property =
                    new PresetListProperty.ByReference();

            preset_list_property.SetCount(preset_count);
            String[] preset_description = {"hoge", "fuga", "hogehoge"};

            for (int j = 0; j < preset_count; j++) {
                preset_list_property.GetPresetInfo()[j].SetId(j);
                preset_list_property.GetPresetInfo()[j].SetDescription(preset_description[j]);
            }

            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_PRESET_LIST_PROPERTY_KEY,
                    preset_list_property, preset_list_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_PRESET_LIST_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("count = " + preset_list_property.GetCount());
                for (int j = 0; j < preset_list_property.GetCount(); j++) {
                    System.out.println("id = " + preset_list_property.GetPresetInfo()[j].GetId()
                            + ", description = "
                            + preset_list_property.GetPresetInfo()[j].GetDescription());
                }

            }

            for (int j = 0; j < preset_count; j++) {
                preset_list_property.GetPresetInfo()[j].SetId(j);
                preset_list_property.GetPresetInfo()[j].SetDescription(preset_description[j]);
            }
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_PRESET_LIST_PROPERTY_KEY,
                    preset_list_property, preset_list_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_PRESET_LIST_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            }

        }

        boolean preset_test = false;
        if (preset_test) {
            PresetProperty.ByReference preset_property = new PresetProperty.ByReference();
            preset_property.SetId(1);

            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_PRESET_PROPERTY_KEY, preset_property,
                    preset_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_PRESET_PROPERTY_KEY + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("count = " + preset_property.GetId());
            }

            preset_property.SetId(1);
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_PRESET_PROPERTY_KEY, preset_property,
                    preset_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_PRESET_PROPERTY_KEY + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            }
        }

        boolean depth_test = false;
        if (depth_test) {
            DepthProperty.ByReference depth_property = new DepthProperty.ByReference();
            depth_property.SetScale(1.0f);
            depth_property.SetDepthMinRange(1.23f);
            depth_property.SetDepthMaxRange(2.34f);

            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_DEPTH_PROPERTY_KEY, depth_property,
                    depth_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_DEPTH_PROPERTY_KEY + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("scale = " + depth_property.GetScale());
                System.out.println("min range = " + depth_property.GetDepthMinRange());
                System.out.println("max range = " + depth_property.GetDepthMaxRange());
            }

            depth_property.SetScale(1.0f);
            depth_property.SetDepthMinRange(1.23f);
            depth_property.SetDepthMaxRange(2.34f);
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_DEPTH_PROPERTY_KEY, depth_property,
                    depth_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_DEPTH_PROPERTY_KEY + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            }
        }

        boolean confidence_test = false;
        if (confidence_test) {
            ConfidenceProperty.ByReference confidence_property =
                    new ConfidenceProperty.ByReference();
            confidence_property.SetWidth(1);
            confidence_property.SetHeight(2);
            confidence_property.SetStrideBytes(3);
            confidence_property.SetPixelFormat("hoge");

            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_CONFIDENCE_PROPERTY_KEY,
                    confidence_property, confidence_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_CONFIDENCE_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("width = " + confidence_property.GetWidth());
                System.out.println("height = " + confidence_property.GetHeight());
                System.out.println("stride_bytes = " + confidence_property.GetStrideBytes());
                System.out.println("pixel_format = " + confidence_property.GetPixelFormat());
            }

            confidence_property.SetWidth(1);
            confidence_property.SetHeight(2);
            confidence_property.SetStrideBytes(3);
            confidence_property.SetPixelFormat("hoge");
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_CONFIDENCE_PROPERTY_KEY,
                    confidence_property, confidence_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_CONFIDENCE_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            }
        }

        boolean imu_unit_test = false;
        if (imu_unit_test) {
            ImuDataUnitProperty.ByReference imu_unit_property =
                    new ImuDataUnitProperty.ByReference();
            imu_unit_property.SetAccelerationUnit(
                    ImuDataUnitProperty.AccelerationUnit.SENSCORD_GRAVITATIONAL);
            imu_unit_property.SetAngularVelocityUnit(
                    ImuDataUnitProperty.AngularVelocityUnit.SENSCORD_DEGREE_PER_SECOND);
            imu_unit_property
                    .SetMagneticFieldUnit(ImuDataUnitProperty.MagneticFieldUnit.SENSCORD_GAUSS);
            imu_unit_property
                    .SetOrientationUnit(ImuDataUnitProperty.OrientationUnit.SENSCORD_DEGREE);

            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_IMU_DATA_UNIT_PROPERTY_KEY,
                    imu_unit_property, imu_unit_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_IMU_DATA_UNIT_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("acceleration = "
                        + imu_unit_property.GetAccelerationUnit() + "(" + imu_unit_property
                                .GetAccelerationUnitString(imu_unit_property.GetAccelerationUnit())
                        + ")");
                System.out.println("angular velocity= " + imu_unit_property.GetAngularVelocityUnit()
                        + "(" + imu_unit_property.GetAngularVelocityUnitString(
                                imu_unit_property.GetAngularVelocityUnit())
                        + ")");
                System.out.println("magnatic field = " + imu_unit_property.GetMagneticFieldUnit()
                        + "(" + imu_unit_property.GetMagneticFieldUnitString(
                                imu_unit_property.GetMagneticFieldUnit())
                        + ")");
                System.out.println("orientation = "
                        + imu_unit_property.GetOrientationUnit() + "(" + imu_unit_property
                                .GetOrientationUnitString(imu_unit_property.GetOrientationUnit())
                        + ")");
            }

            imu_unit_property.SetAccelerationUnit(
                    ImuDataUnitProperty.AccelerationUnit.SENSCORD_GRAVITATIONAL);
            imu_unit_property.SetAngularVelocityUnit(
                    ImuDataUnitProperty.AngularVelocityUnit.SENSCORD_DEGREE_PER_SECOND);
            imu_unit_property
                    .SetMagneticFieldUnit(ImuDataUnitProperty.MagneticFieldUnit.SENSCORD_GAUSS);
            imu_unit_property
                    .SetOrientationUnit(ImuDataUnitProperty.OrientationUnit.SENSCORD_DEGREE);
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_IMU_DATA_UNIT_PROPERTY_KEY,
                    imu_unit_property, imu_unit_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_IMU_DATA_UNIT_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            }
        }

        boolean sampling_freq_test = false;
        if (sampling_freq_test) {
            SamplingFrequencyProperty.ByReference sampling_freq_property =
                    new SamplingFrequencyProperty.ByReference();
            sampling_freq_property.SetValue(1);

            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_SAMPLING_FREQUENCY_PROPERTY_KEY,
                    sampling_freq_property, sampling_freq_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_SAMPLING_FREQUENCY_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("sampling frequency = " + sampling_freq_property.GetValue());
            }

            sampling_freq_property.SetValue(1);
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_SAMPLING_FREQUENCY_PROPERTY_KEY,
                    sampling_freq_property, sampling_freq_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_SAMPLING_FREQUENCY_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            }
        }

        boolean accelerometer_range_test = false;
        if (accelerometer_range_test) {
            AccelerometerRangeProperty.ByReference accelerometer_range_property =
                    new AccelerometerRangeProperty.ByReference();
            accelerometer_range_property.SetValue(2);

            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_ACCELEROMETER_RANGE_PROPERTY_KEY,
                    accelerometer_range_property, accelerometer_range_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_ACCELEROMETER_RANGE_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println(
                        "accelerometer range = " + accelerometer_range_property.GetValue());
            }

            accelerometer_range_property.SetValue(2);
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_ACCELEROMETER_RANGE_PROPERTY_KEY,
                    accelerometer_range_property, accelerometer_range_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_ACCELEROMETER_RANGE_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            }
        }

        boolean gyrometer_range_test = false;
        if (gyrometer_range_test) {
            GyrometerRangeProperty.ByReference gyrometer_range_property =
                    new GyrometerRangeProperty.ByReference();
            gyrometer_range_property.SetValue(3);

            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_GYROMETER_RANGE_PROPERTY_KEY,
                    gyrometer_range_property, gyrometer_range_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_GYROMETER_RANGE_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("gyrometer range = " + gyrometer_range_property.GetValue());
            }

            gyrometer_range_property.SetValue(3);
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_GYROMETER_RANGE_PROPERTY_KEY,
                    gyrometer_range_property, gyrometer_range_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_GYROMETER_RANGE_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            }
        }

        boolean magnetometer_range_test = false;
        if (magnetometer_range_test) {
            MagnetometerRangeProperty.ByReference magnetometer_range_property =
                    new MagnetometerRangeProperty.ByReference();
            magnetometer_range_property.SetValue(4);

            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_MAGNETOMETER_RANGE_PROPERTY_KEY,
                    magnetometer_range_property, magnetometer_range_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_MAGNETOMETER_RANGE_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out
                        .println("magnetometer range = " + magnetometer_range_property.GetValue());
            }

            magnetometer_range_property.SetValue(4);
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_MAGNETOMETER_RANGE_PROPERTY_KEY,
                    magnetometer_range_property, magnetometer_range_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_MAGNETOMETER_RANGE_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            }
        }

        boolean magnetometer_range3_test = false;
        if (magnetometer_range3_test) {
            MagnetometerRange3Property.ByReference magnetometer_range3_property =
                    new MagnetometerRange3Property.ByReference();
            magnetometer_range3_property.SetX(4);
            magnetometer_range3_property.SetY(5);
            magnetometer_range3_property.SetZ(6);

            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_MAGNETOMETER_RANGE3_PROPERTY_KEY,
                    magnetometer_range3_property, magnetometer_range3_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_MAGNETOMETER_RANGE3_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("magneometer range x = " + magnetometer_range3_property.GetX());
                System.out.println("magneometer range y = " + magnetometer_range3_property.GetY());
                System.out.println("magneometer range z = " + magnetometer_range3_property.GetZ());
            }

            magnetometer_range3_property.SetX(4);
            magnetometer_range3_property.SetY(5);
            magnetometer_range3_property.SetZ(6);
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_MAGNETOMETER_RANGE3_PROPERTY_KEY,
                    magnetometer_range3_property, magnetometer_range3_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_MAGNETOMETER_RANGE3_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            }
        }

        boolean acceleration_calib_test = false;
        if (acceleration_calib_test) {
            AccelerationCalibProperty.ByReference acceleration_calib_property =
                    new AccelerationCalibProperty.ByReference();

            float[] element = {1.1f, 1.2f, 1.3f, 1.4f, 1.5f, 1.6f, 1.7f, 1.8f, 1.9f};
            acceleration_calib_property.GetValueMatrix().SetElement(element);
            acceleration_calib_property.GetOffset().SetX(1.1f);
            acceleration_calib_property.GetOffset().SetY(1.2f);
            acceleration_calib_property.GetOffset().SetZ(1.3f);

            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_ACCELERATION_CALIB_PROPERTY_KEY,
                    acceleration_calib_property, acceleration_calib_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_ACCELERATION_CALIB_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                float[][] matrix = new float[3][3];
                for (int i = 0; i < 3; i++) {
                    for (int j = 0; j < 3; j++) {
                        matrix[i][j] = acceleration_calib_property.GetValueMatrix()
                                .GetElement()[i * 3 + j];
                        System.out.println("matrix[" + i + "][" + j + "] = " + matrix[i][j]);
                    }
                }
                System.out.println("offset x = " + acceleration_calib_property.GetOffset().GetX());
                System.out.println("offset y = " + acceleration_calib_property.GetOffset().GetY());
                System.out.println("offset z = " + acceleration_calib_property.GetOffset().GetZ());
            }

            float[] element_set = {1.1f, 1.2f, 1.3f, 1.4f, 1.5f, 1.6f, 1.7f, 1.8f, 1.9f};
            acceleration_calib_property.GetValueMatrix().SetElement(element_set);
            acceleration_calib_property.GetOffset().SetX(1.1f);
            acceleration_calib_property.GetOffset().SetY(1.2f);
            acceleration_calib_property.GetOffset().SetZ(1.3f);
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_ACCELERATION_CALIB_PROPERTY_KEY,
                    acceleration_calib_property, acceleration_calib_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_ACCELERATION_CALIB_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            }

        }

        boolean angular_calib_test = false;
        if (angular_calib_test) {
            AngularVelocityCalibProperty.ByReference angular_calib_property =
                    new AngularVelocityCalibProperty.ByReference();
            float[] element = {2.1f, 2.2f, 2.3f, 2.4f, 2.5f, 2.6f, 2.7f, 2.8f, 2.9f};
            angular_calib_property.GetValueMatrix().SetElement(element);
            angular_calib_property.GetOffset().SetX(2.1f);
            angular_calib_property.GetOffset().SetY(2.2f);
            angular_calib_property.GetOffset().SetZ(2.3f);

            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_ANGULAR_VELOCITY_CALIB_PROPERTY_KEY,
                    angular_calib_property, angular_calib_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_ANGULAR_VELOCITY_CALIB_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                float[][] matrix = new float[3][3];
                for (int i = 0; i < 3; i++) {
                    for (int j = 0; j < 3; j++) {
                        matrix[i][j] =
                                angular_calib_property.GetValueMatrix().GetElement()[i * 3 + j];
                        System.out.println("matrix[" + i + "][" + j + "] = " + matrix[i][j]);
                    }
                }
                System.out.println("offset x = " + angular_calib_property.GetOffset().GetX());
                System.out.println("offset y = " + angular_calib_property.GetOffset().GetY());
                System.out.println("offset z = " + angular_calib_property.GetOffset().GetZ());
            }


            float[] element_set = {2.1f, 2.2f, 2.3f, 2.4f, 2.5f, 2.6f, 2.7f, 2.8f, 2.9f};
            angular_calib_property.GetValueMatrix().SetElement(element_set);
            angular_calib_property.GetOffset().SetX(2.1f);
            angular_calib_property.GetOffset().SetY(2.2f);
            angular_calib_property.GetOffset().SetZ(2.3f);
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_ANGULAR_VELOCITY_CALIB_PROPERTY_KEY,
                    angular_calib_property, angular_calib_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_ANGULAR_VELOCITY_CALIB_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            }

        }

        boolean magnetic_calib_test = false;
        if (magnetic_calib_test) {
            MagneticFieldCalibProperty.ByReference magnetic_calib_property =
                    new MagneticFieldCalibProperty.ByReference();
            float[] element = {3.1f, 3.2f, 3.3f, 3.4f, 3.5f, 3.6f, 3.7f, 3.8f, 3.9f};
            magnetic_calib_property.GetValueMatrix().SetElement(element);
            magnetic_calib_property.GetOffset().SetX(3.1f);
            magnetic_calib_property.GetOffset().SetY(3.2f);
            magnetic_calib_property.GetOffset().SetZ(3.3f);

            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_MAGNETIC_FIELD_CALIB_PROPERTY_KEY,
                    magnetic_calib_property, magnetic_calib_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_MAGNETIC_FIELD_CALIB_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                float[][] matrix = new float[3][3];
                for (int i = 0; i < 3; i++) {
                    for (int j = 0; j < 3; j++) {
                        matrix[i][j] =
                                magnetic_calib_property.GetValueMatrix().GetElement()[i * 3 + j];
                        System.out.println("matrix[" + i + "][" + j + "] = " + matrix[i][j]);
                    }
                }
                System.out.println("offset x = " + magnetic_calib_property.GetOffset().GetX());
                System.out.println("offset y = " + magnetic_calib_property.GetOffset().GetY());
                System.out.println("offset z = " + magnetic_calib_property.GetOffset().GetZ());
            }

            float[] element_set = {3.1f, 3.2f, 3.3f, 3.4f, 3.5f, 3.6f, 3.7f, 3.8f, 3.9f};
            magnetic_calib_property.GetValueMatrix().SetElement(element_set);
            magnetic_calib_property.GetOffset().SetX(3.1f);
            magnetic_calib_property.GetOffset().SetY(3.2f);
            magnetic_calib_property.GetOffset().SetZ(3.3f);
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_MAGNETIC_FIELD_CALIB_PROPERTY_KEY,
                    magnetic_calib_property, magnetic_calib_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_MAGNETIC_FIELD_CALIB_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            }
        }

        boolean magnetic_north_test = false;
        if (magnetic_north_test) {
            MagneticNorthCalibProperty.ByReference magnetic_north_property =
                    new MagneticNorthCalibProperty.ByReference();
            magnetic_north_property.SetDeclination(11.11f);
            magnetic_north_property.SetInclination(12.11f);

            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_MAGNETIC_NORTH_CALIB_PROPERTY_KEY,
                    magnetic_north_property, magnetic_north_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_MAGNETIC_NORTH_CALIB_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("declination = " + magnetic_north_property.GetDeclination());
                System.out.println("inclination = " + magnetic_north_property.GetInclination());
            }

            magnetic_north_property.SetDeclination(11.11f);
            magnetic_north_property.SetInclination(12.11f);
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_MAGNETIC_NORTH_CALIB_PROPERTY_KEY,
                    magnetic_north_property, magnetic_north_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_MAGNETIC_NORTH_CALIB_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            }
        }

        boolean slam_supported_test = false;
        if (slam_supported_test) {
            SlamDataSupportedProperty.ByReference slam_supported_property =
                    new SlamDataSupportedProperty.ByReference();
            slam_supported_property.SetOdometrySupported((byte) 1);
            slam_supported_property.SetGridMapSupported((byte) 1);
            slam_supported_property.SetPointCloudSupported((byte) 1);

            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_SLAM_DATA_SUPPORTED_PROPERTY_KEY,
                    slam_supported_property, slam_supported_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_SLAM_DATA_SUPPORTED_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out
                        .println("odometry = " + slam_supported_property.GetOdometrySupported());
                System.out.println("gridmap = " + slam_supported_property.GetGridMapSupported());
                System.out.println(
                        "pointcloud = " + slam_supported_property.GetPointCloudSupported());
            }

            slam_supported_property.SetOdometrySupported((byte) 1);
            slam_supported_property.SetGridMapSupported((byte) 1);
            slam_supported_property.SetPointCloudSupported((byte) 1);
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_SLAM_DATA_SUPPORTED_PROPERTY_KEY,
                    slam_supported_property, slam_supported_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_SLAM_DATA_SUPPORTED_PROPERTY_KEY
                    + ") ret = " + ret);
            if (ret != 0) {
                PrintError();
            }
        }

        boolean initial_pose_test = false;
        if (initial_pose_test) {
            InitialPoseProperty.ByReference initial_pose_property =
                    new InitialPoseProperty.ByReference();
            initial_pose_property.GetPose().GetPosition().SetX(0.1f);
            initial_pose_property.GetPose().GetPosition().SetY(0.2f);
            initial_pose_property.GetPose().GetPosition().SetZ(0.3f);
            initial_pose_property.GetPose().GetOrientation().SetX(0.4f);
            initial_pose_property.GetPose().GetOrientation().SetY(0.5f);
            initial_pose_property.GetPose().GetOrientation().SetZ(0.6f);
            initial_pose_property.GetPose().GetOrientation().SetW(0.7f);

            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_INITIAL_POSE_PROPERTY_KEY,
                    initial_pose_property, initial_pose_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_INITIAL_POSE_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println(
                        "position x = " + initial_pose_property.GetPose().GetPosition().GetX());
                System.out.println(
                        "position y = " + initial_pose_property.GetPose().GetPosition().GetY());
                System.out.println(
                        "position z = " + initial_pose_property.GetPose().GetPosition().GetZ());
                System.out.println("orientation x = "
                        + initial_pose_property.GetPose().GetOrientation().GetX());
                System.out.println("orientation y = "
                        + initial_pose_property.GetPose().GetOrientation().GetY());
                System.out.println("orientation z = "
                        + initial_pose_property.GetPose().GetOrientation().GetZ());
                System.out.println("orientation w = "
                        + initial_pose_property.GetPose().GetOrientation().GetW());
            }

            initial_pose_property.GetPose().GetPosition().SetX(0.1f);
            initial_pose_property.GetPose().GetPosition().SetY(0.2f);
            initial_pose_property.GetPose().GetPosition().SetZ(0.3f);
            initial_pose_property.GetPose().GetOrientation().SetX(0.4f);
            initial_pose_property.GetPose().GetOrientation().SetY(0.5f);
            initial_pose_property.GetPose().GetOrientation().SetZ(0.6f);
            initial_pose_property.GetPose().GetOrientation().SetW(0.7f);
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_INITIAL_POSE_PROPERTY_KEY,
                    initial_pose_property, initial_pose_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_INITIAL_POSE_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            }
        }

        boolean odometry_test = false;
        if (odometry_test) {
            OdometryDataProperty.ByReference odometry_property =
                    new OdometryDataProperty.ByReference();
            odometry_property.SetCoordinateSystem(
                    OdometryDataProperty.CoordinateSystem.SENSCORD_WORLD_COORDINATWE);

            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_ODOMETRY_DATA_PROPERTY_KEY,
                    odometry_property, odometry_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_ODOMETRY_DATA_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("coordinate system = "
                        + odometry_property.GetCoordinateSystem() + "(" + odometry_property
                                .GetCoordinateSystemString(odometry_property.GetCoordinateSystem())
                        + ")");
            }

            odometry_property.SetCoordinateSystem(
                    OdometryDataProperty.CoordinateSystem.SENSCORD_WORLD_COORDINATWE);
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_ODOMETRY_DATA_PROPERTY_KEY,
                    odometry_property, odometry_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_ODOMETRY_DATA_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            }
        }

        boolean grid_map_test = false;
        if (grid_map_test) {
            GridMapProperty.ByReference grid_map_property = new GridMapProperty.ByReference();
            grid_map_property.SetGridNumX(0);
            grid_map_property.SetGridNumY(0);
            grid_map_property.SetGridNumZ(0);
            grid_map_property.SetPixelFormat(GridMapProperty.PixelFormat.SENSCORD_PIXEL_FORMAT_GRID_MAP_1P1N);
            GridSizeProperty grid_Size = new GridSizeProperty();
            grid_Size.SetX(0);
            grid_Size.SetY(0);
            grid_Size.SetZ(0);
            grid_Size.SetUnit(GridUnit.Pixel);
            grid_map_property.SetGridSize(grid_Size);

            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_GRID_MAP_PROPERTY_KEY, grid_map_property,
                    grid_map_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_GRID_MAP_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("grid num x = " + grid_map_property.GetGridNumX());
                System.out.println("grid num y = " + grid_map_property.GetGridNumY());
                System.out.println("grid num z = " + grid_map_property.GetGridNumZ());
                System.out.println("pixelformat = " + grid_map_property.GetPixelFormat());
                System.out.println("unit x = " + grid_map_property.GetGridSize().GetX());
                System.out.println("unit y = " + grid_map_property.GetGridSize().GetY());
                System.out.println("unit z = " + grid_map_property.GetGridSize().GetZ());
                System.out.println("unit = " + grid_map_property.GetGridSize().GetUnit());
            }

            grid_map_property.SetGridNumX(0);
            grid_map_property.SetGridNumY(0);
            grid_map_property.SetGridNumZ(0);
            grid_map_property.SetPixelFormat(GridMapProperty.PixelFormat.SENSCORD_PIXEL_FORMAT_GRID_MAP_1P1N);
            grid_Size.SetX(0);
            grid_Size.SetY(0);
            grid_Size.SetZ(0);
            grid_Size.SetUnit(GridUnit.Pixel);
            grid_map_property.SetGridSize(grid_Size);
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_GRID_MAP_PROPERTY_KEY, grid_map_property,
                    grid_map_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_GRID_MAP_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            }
        }

        boolean point_cloud_test = false;
        if (point_cloud_test) {
            PointCloudProperty.ByReference point_cloud_property =
                    new PointCloudProperty.ByReference();
            point_cloud_property.SetHeight(1);
            point_cloud_property.SetWidth(2);
            point_cloud_property.SetPixelFormat("hoge");

            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_POINT_CLOUD_PROPERTY_KEY,
                    point_cloud_property, point_cloud_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_POINT_CLOUD_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("height = " + point_cloud_property.GetHeight());
                System.out.println("width = " + point_cloud_property.GetWidth());
                System.out.println("pixel format = " + point_cloud_property.GetPixelFormat());
            }

            point_cloud_property.SetHeight(1);
            point_cloud_property.SetWidth(2);
            point_cloud_property.SetPixelFormat("hoge");
            ret = senscord.senscord_stream_set_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_POINT_CLOUD_PROPERTY_KEY,
                    point_cloud_property, point_cloud_property.size());
            System.out.println("senscord_stream_set_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_POINT_CLOUD_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            }
        }



        IntByReference channel_count = new IntByReference();
        ret = senscord.senscord_frame_get_channel_count(frame.getValue(), channel_count);
        System.out.println("senscord_frame_get_channel_count ret = " + ret);
        if (ret != 0) {
            PrintError();
        } else {
            System.out.println("channel count  = " + channel_count.getValue());
        }

        for (int i = 0; i < channel_count.getValue(); i++) {
            PointerByReference channel = new PointerByReference();
            ret = senscord.senscord_frame_get_channel(frame.getValue(), i, channel);
            System.out.println("senscord_frame_get_channel ret = " + ret);
            if (ret != 0) {
                PrintError();
            }

            IntByReference channel_property_count = new IntByReference();
            ret = senscord.senscord_channel_get_property_count(channel.getValue(),
                    channel_property_count);
            System.out.println("senscord_channel_get_property_count ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("count  " + channel_property_count.getValue());
            }

            for (int j = 0; j < channel_property_count.getValue(); j++) {
                String[] property_key = {""};
                ret = senscord.senscord_channel_get_property_key(channel.getValue(), j,
                        property_key);
                System.out.println("senscord_channel_get_property_key ret = " + ret);
                if (ret != 0) {
                    PrintError();
                } else {
                    System.out.println("property_key [" + j + "] = " + property_key[0]);
                }
            }



            IntByReference channel_id = new IntByReference();
            ret = senscord.senscord_channel_get_channel_id(channel.getValue(), channel_id);
            System.out.println("senscord_channel_get_channel_id ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("channel_id  = " + channel_id.getValue());
            }

            PointerByReference channel_from_id = new PointerByReference();
            ret = senscord.senscord_frame_get_channel_from_channel_id(frame.getValue(),
                    channel_id.getValue(), channel_from_id);
            System.out.println("senscord_frame_get_channel_from_channel_id ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                if (channel.getValue().equals(channel_from_id.getValue())) {
                    System.out.println("channel poiner matched");
                } else {
                    System.out.println("channel poiner mismatched");
                }
            }


            RawData.ByReference raw_data = new RawData.ByReference();

            ret = senscord.senscord_channel_get_raw_data(channel.getValue(), raw_data);
            System.out.println("senscord_channel_get_raw_data ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("type  = " + raw_data.GetType() + ", size = "
                        + raw_data.GetSize() + ", timestamp = " + raw_data.GetTimestamp());
                if (raw_data.GetAddress() != Pointer.NULL) {

                    Pointer image_data = raw_data.GetAddress();

                    for (int j = 0; j < raw_data.GetSize(); j++) {
                        System.out.println("image data[" + j + "] = " + image_data.getByte(j));
                    }
                }
            }


            boolean confidence_channel_test = false;
            if (confidence_channel_test) {
                ConfidenceProperty.ByReference confidence_property =
                        new ConfidenceProperty.ByReference();
                ret = senscord.senscord_channel_get_property(channel.getValue(),
                        SenscordCLibrary.PropertyKey.SENSCORD_CONFIDENCE_PROPERTY_KEY,
                        confidence_property, confidence_property.size());
                if (ret != 0) {
                    PrintError();
                } else {
                    System.out.println("width = " + confidence_property.GetWidth());
                    System.out.println("height = " + confidence_property.GetHeight());
                    System.out.println("stride byte = " + confidence_property.GetStrideBytes());
                    System.out.println("pixel_format = " + confidence_property.GetPixelFormat());
                }

            }

            IntByReference updated_count = new IntByReference();
            ret = senscord.senscord_channel_get_updated_property_count(channel.getValue(),
                    updated_count);
            System.out.println("senscord_channel_get_updated_property_count ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("count  = " + updated_count.getValue());
            }

            for (int j = 0; j < updated_count.getValue(); j++) {
                String[] updated_key = {""};
                ret = senscord.senscord_channel_get_updated_property_key(channel.getValue(), 1,
                        updated_key);
                System.out.println("senscord_channel_get_updated_property_key ret = " + ret);
                if (ret != 0) {
                    PrintError();
                } else {
                    System.out.println("key  = " + updated_key[0]);
                }
            }

            boolean vector3_deserialize_test = false;
            if (vector3_deserialize_test && raw_data.address != Pointer.NULL) {
                Vector3Float.ByReference[] vector3_deserialize_data =
                        new Vector3Float.ByReference[1];
                vector3_deserialize_data[0] = new Vector3Float.ByReference();

                ret = senscord.senscord_deserialize_vector3_data(raw_data.address, raw_data.size,
                        vector3_deserialize_data);
                System.out.println("senscord_deserialize_vector3_data ret = " + ret);
                if (ret != 0) {
                    PrintError();
                } else {
                    System.out.println("x = " + vector3_deserialize_data[0].GetX());
                    System.out.println("y = " + vector3_deserialize_data[0].GetY());
                    System.out.println("z = " + vector3_deserialize_data[0].GetZ());

                    ret = senscord.senscord_release_vector3_data(vector3_deserialize_data[0]);
                    System.out.println("senscord_deserialize_vector3_data ret = " + ret);
                    if (ret != 0) {
                        PrintError();
                    }
                }
            }

            boolean rotation_deserialize_test = false;
            if (rotation_deserialize_test && raw_data.address != Pointer.NULL) {
                RotationData.ByReference[] rotation_deserialize_data =
                        new RotationData.ByReference[1];
                rotation_deserialize_data[0] = new RotationData.ByReference();

                ret = senscord.senscord_deserialize_rotation_data(raw_data.address, raw_data.size,
                        rotation_deserialize_data);
                System.out.println("senscord_deserialize_rotation_data ret = " + ret);
                if (ret != 0) {
                    PrintError();
                } else {
                    System.out.println("roll = " + rotation_deserialize_data[0].GetRoll());
                    System.out.println("pitch = " + rotation_deserialize_data[0].GetPitch());
                    System.out.println("yaw = " + rotation_deserialize_data[0].GetYaw());

                    ret = senscord.senscord_release_rotation_data(rotation_deserialize_data[0]);
                    System.out.println("senscord_release_rotation_data ret = " + ret);
                    if (ret != 0) {
                        PrintError();
                    }
                }
            }

            boolean pose_deserialize_test = false;
            if (pose_deserialize_test && raw_data.address != Pointer.NULL) {
                PoseQuaternionData.ByReference[] pose_deserialize_data = new PoseQuaternionData.ByReference[1];
                pose_deserialize_data[0] = new PoseQuaternionData.ByReference();

                ret = senscord.senscord_deserialize_pose_quaternion_data(raw_data.address, raw_data.size,
                        pose_deserialize_data);
                System.out.println("senscord_deserialize_pose_data ret = " + ret);
                if (ret != 0) {
                    PrintError();
                } else {
                    System.out
                            .println("vecotr x = " + pose_deserialize_data[0].GetPosition().GetX());
                    System.out
                            .println("vector y = " + pose_deserialize_data[0].GetPosition().GetY());
                    System.out
                            .println("vector z = " + pose_deserialize_data[0].GetPosition().GetZ());
                    System.out.println(
                            "quaternion x = " + pose_deserialize_data[0].GetOrientation().GetX());
                    System.out.println(
                            "quaternion y = " + pose_deserialize_data[0].GetOrientation().GetY());
                    System.out.println(
                            "quaternion z = " + pose_deserialize_data[0].GetOrientation().GetZ());
                    System.out.println(
                            "quaternion w = " + pose_deserialize_data[0].GetOrientation().GetW());

                    ret = senscord.senscord_release_pose_quaternion_data(pose_deserialize_data[0]);
                    System.out.println("senscord_release_pose_data ret = " + ret);
                    if (ret != 0) {
                        PrintError();
                    }
                }
            }

            IntByReference ch_property_count = new IntByReference();
            ret = senscord.senscord_channel_get_property_count(channel.getValue(),
                    ch_property_count);
            System.out.println("senscord_channel_get_property_count ret = " + ret);
            if (ret != 0) {
                PrintError();
            } else {
                System.out.println("count  = " + ch_property_count.getValue());
            }

            String[] property_key = new String[1];
            for (int j = 0; j < ch_property_count.getValue(); j++) {
                ret = senscord.senscord_channel_get_property_key(channel.getValue(), j,
                        property_key);
                System.out.println("senscord_channel_get_property_key ret = " + ret);
                if (ret != 0) {
                    PrintError();
                } else {
                    System.out.println("key  = " + property_key[0]);
                }
            }
        }

        boolean skip_frame_test = false;
        if (skip_frame_test) {
            SkipFrameProperty.ByReference skipframe_property = new SkipFrameProperty.ByReference();
            ret = senscord.senscord_stream_get_property(stream.getValue(),
                    SenscordCLibrary.PropertyKey.SENSCORD_SKIP_FRAME_PROPERTY_KEY,
                    skipframe_property, skipframe_property.size());
            System.out.println("senscord_stream_get_property ("
                    + SenscordCLibrary.PropertyKey.SENSCORD_SKIP_FRAME_PROPERTY_KEY + ") ret = "
                    + ret);
            if (ret != 0) {
                PrintError();
            }
        }

        boolean release_frame_unused_test = false;
        if (release_frame_unused_test) {
            ret = senscord.senscord_stream_release_frame_unused(stream.getValue(),
                    frame.getValue());
            System.out.println("senscord_stream_release_frame_unused ret = " + ret);
            if (ret != 0) {
                PrintError();
            }
        }

        ret = senscord.senscord_stream_release_frame(stream.getValue(), frame.getValue());
        System.out.println("senscord_stream_release_frame ret = " + ret);
        if (ret != 0) {
            PrintError();
        }


        ret = senscord.senscord_stream_stop(stream.getValue());
        System.out.println("senscord_stream_stop ret = " + ret);
        if (ret != 0) {
            PrintError();
        }

        ret = senscord.senscord_stream_unregister_event_callback(stream.getValue(),
                SenscordCLibrary.Event.SENSCORD_EVENT_FRAME_DROPPED);
        System.out.println("senscord_stream_unregister_event_callback ret = " + ret);
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

        return;
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
