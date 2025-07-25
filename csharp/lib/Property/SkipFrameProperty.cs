/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Structure for setting the skip rate of the frame.
    /// </summary>
    [MessagePackObject]
    public class SkipFrameProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "skip_frame_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// If 'rate = 1' is specified, frames are not skipped.
        /// If 'rate = N' (N is 2 or more) is specified, the frame is skipped and
        /// the frame rate drops to 1 / N.
        /// </summary>
        [Key("rate")]
        public int Rate { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Key}({this.Rate})";
        }
    }
}
