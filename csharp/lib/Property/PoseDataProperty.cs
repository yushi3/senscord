/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

﻿using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Pose data property.
    /// </summary>
    [MessagePackObject]
    public class PoseDataProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "pose_data_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// format to pose data.
        /// </summary>
        [Key("data_format")]
        public string DataFormat { get; set; } = string.Empty;

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return string.Format(
                "{0}(data_format={1})",
                this.Key, this.DataFormat);
        }
    }

    /// <summary>
    /// Pose data format.
    /// </summary>
    public static partial class PoseDataFormat
    {
        /// <summary>
        /// Quaternion.
        /// </summary>
        public static readonly string Quaternion = "pose_data_quaternion";

        /// <summary>
        /// Matrix.
        /// </summary>
        public static readonly string Matrix = "pose_data_matrix";
    }
}
