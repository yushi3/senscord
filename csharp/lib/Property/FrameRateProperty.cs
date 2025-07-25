/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Structure for setting frame rate.
    /// Specify in the style of numerator / denominator.
    /// </summary>
    /// <example>
    /// 60fps : num = 60, denom = 1
    /// </example>
    [MessagePackObject]
    public class FrameRateProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "frame_rate_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Framerate numerator.
        /// </summary>
        [Key("num")]
        public int Num { get; set; }

        /// <summary>
        /// Framerate denominator.
        /// </summary>
        [Key("denom")]
        public int Denom { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Key}(num={this.Num}, denom={this.Denom})";
        }
    }
}
