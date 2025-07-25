/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Structure for handling Depth data properties.
    /// </summary>
    [MessagePackObject]
    public class DepthProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "depth_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Scale of the depth value, in metres.
        /// By multiplying this value, the depth value is converted to metres.
        /// </summary>
        [Key("scale")]
        public float Scale { get; set; }

        /// <summary>
        /// Minimum depth value of the sensor.
        /// </summary>
        [Key("depth_min_range")]
        public float MinRange { get; set; }

        /// <summary>
        /// Maximum depth value of the sensor.
        /// </summary>
        [Key("depth_max_range")]
        public float MaxRange { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Key}(scale={this.Scale}, min={this.MinRange}, max={this.MaxRange})";
        }
    }
}
