/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Set the magnetometer range.
    /// </summary>
    [MessagePackObject]
    public class MagnetometerRangeProperty : Scalar<float>, IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "magnetometer_range_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;
    }

    /// <summary>
    /// Set the range of magnetometer for each xyz.
    /// </summary>
    [MessagePackObject]
    public class MagnetometerRange3Property : Vector3<float>, IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "magnetometer_range3_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;
    }

    /// <summary>
    /// Property used for calibration of magnetic field data.
    /// </summary>
    [MessagePackObject]
    public class MagneticFieldCalibProperty : AxisMisalignment, IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "magnetic_field_calib_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;
    }

    /// <summary>
    /// Property for calibration magnetic north.
    /// </summary>
    [MessagePackObject]
    public class MagneticNorthCalibProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "magnetic_north_calib_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Declination.
        /// </summary>
        [Key("declination")]
        public float Declination { get; set; }

        /// <summary>
        /// Inclination.
        /// </summary>
        [Key("inclination")]
        public float Inclination { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Key}(declination={this.Declination}, inclination={this.Inclination})";
        }
    }
}
