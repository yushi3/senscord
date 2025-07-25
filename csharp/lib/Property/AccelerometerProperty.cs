/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Set the acceleration range.
    /// </summary>
    [MessagePackObject]
    public class AccelerometerRangeProperty : Scalar<float>, IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "accelerometer_range_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;
    }

    /// <summary>
    /// Property used for calibration of acceleration data.
    /// </summary>
    [MessagePackObject]
    public class AccelerationCalibProperty : AxisMisalignment, IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "acceleration_calib_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;
    }
}
