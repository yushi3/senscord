/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Set the gyrometer range.
    /// </summary>
    [MessagePackObject]
    public class GyrometerRangeProperty : Scalar<float>, IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "gyrometer_range_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;
    }

    /// <summary>
    /// Property used for calibration of angular velocity data.
    /// </summary>
    [MessagePackObject]
    public class AngularVelocityCalibProperty : AxisMisalignment, IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "angular_velocity_calib_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;
    }
}
