/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Data for rotation attitude.
    /// </summary>
    [MessagePackObject]
    public class RotationData
    {
        /// <summary>
        /// Raw data type.
        /// </summary>
        [IgnoreMember]
        public string Type { get; } = RawDataType.Rotation;

        /// <summary>
        /// Roll angle.
        /// </summary>
        [Key("roll")]
        public float roll { get; set; }

        /// <summary>
        /// Pitch angle.
        /// </summary>
        [Key("pitch")]
        public float pitch { get; set; }

        /// <summary>
        /// Yaw angle.
        /// </summary>
        [Key("yaw")]
        public float yaw { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Type}(roll={this.roll}, pitch={this.pitch}, yaw={this.yaw})";
        }
    }
}
