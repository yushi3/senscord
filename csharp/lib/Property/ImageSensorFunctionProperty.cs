/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Structures used to set the functions used in the sensor.
    /// </summary>
    [MessagePackObject]
    public class ImageSensorFunctionProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "image_sensor_function_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Setting auto exposure function.
        /// </summary>
        [Key("auto_exposure")]
        public bool AutoExposure { get; set; }

        /// <summary>
        /// Setting automatic white balance function.
        /// </summary>
        [Key("auto_white_balance")]
        public bool AutoWhiteBalance { get; set; }

        /// <summary>
        /// Brightness value.
        /// </summary>
        [Key("brightness")]
        public int Brightness { get; set; }

        /// <summary>
        /// ISO sensitivity. (100,200,400,800,1600,...)
        /// </summary>
        [Key("iso_sensitivity")]
        public int IsoSensitivity { get; set; }

        /// <summary>
        /// Time of exposure [100usec].
        /// </summary>
        [Key("exposure_time")]
        public int ExposureTime { get; set; }

        /// <summary>
        /// Exposure metering mode.
        /// </summary>
        [Key("exposure_metering")]
        public string ExposureMetering { get; set; } = string.Empty;

        /// <summary>
        /// Gamma correction value.
        /// </summary>
        [Key("gamma_value")]
        public float GammaValue { get; set; }

        /// <summary>
        /// Gain value.
        /// </summary>
        [Key("gain_value")]
        public int GainValue { get; set; }

        /// <summary>
        /// Hue value.
        /// </summary>
        [Key("hue")]
        public int Hue { get; set; }

        /// <summary>
        /// Saturation value.
        /// </summary>
        [Key("saturation")]
        public int Saturation { get; set; }

        /// <summary>
        /// Sharpness value.
        /// </summary>
        [Key("sharpness")]
        public int Sharpness { get; set; }

        /// <summary>
        /// White balance value.
        /// </summary>
        [Key("white_balance")]
        public int WhiteBalance { get; set; }
    }

    /// <summary>
    /// Structure for acquiring functions supported by Component.
    /// For each function of the image sensor, set the counter corresponding
    /// to Component as a boolean value.
    /// </summary>
    [MessagePackObject]
    public class ImageSensorFunctionSupportedProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "image_sensor_function_supported_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Auto exposure supported.
        /// </summary>
        [Key("auto_exposure_supported")]
        public bool AutoExposureSupported { get; set; }

        /// <summary>
        /// Auto white balance supported.
        /// </summary>
        [Key("auto_white_balance_supported")]
        public bool AutoWhiteBalanceSupported { get; set; }

        /// <summary>
        /// Brightness supported.
        /// </summary>
        [Key("brightness_supported")]
        public bool BrightnessSupported { get; set; }

        /// <summary>
        /// Iso sensitivity supported.
        /// </summary>
        [Key("iso_sensitivity_supported")]
        public bool IsoSensitivitySupported { get; set; }

        /// <summary>
        /// Exposure time supported.
        /// </summary>
        [Key("exposure_time_supported")]
        public bool ExposureTimeSupported { get; set; }

        /// <summary>
        /// Exposure metering supported.
        /// </summary>
        [Key("exposure_metering_supported")]
        public bool ExposureMeteringSupported { get; set; }

        /// <summary>
        /// Gamma value supported.
        /// </summary>
        [Key("gamma_value_supported")]
        public bool GammaValueSupported { get; set; }

        /// <summary>
        /// Gain value supported.
        /// </summary>
        [Key("gain_value_supported")]
        public bool GainValueSupported { get; set; }

        /// <summary>
        /// Hue supported.
        /// </summary>
        [Key("hue_supported")]
        public bool HueSupported { get; set; }

        /// <summary>
        /// Saturation supported.
        /// </summary>
        [Key("saturation_supported")]
        public bool SaturationSupported { get; set; }

        /// <summary>
        /// Sharpness supported.
        /// </summary>
        [Key("sharpness_supported")]
        public bool SharpnessSupported { get; set; }

        /// <summary>
        /// White balance supported.
        /// </summary>
        [Key("white_balance_supported")]
        public bool WhiteBalanceSupported { get; set; }
    }
}
