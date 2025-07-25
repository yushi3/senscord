/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Frame buffering setting.
    /// </summary>
    [MessagePackObject]
    public class FrameBufferingProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "frame_buffering_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Frame buffering number uses the config.
        /// </summary>
        public static readonly int BufferNumUseConfig = -2;

        /// <summary>
        /// Frame buffering number default.
        /// </summary>
        public static readonly int BufferNumDefault = -1;

        /// <summary>
        /// Frame buffering number of unlimited.
        /// </summary>
        public static readonly int BufferNumUnlimited = 0;

        /// <summary>
        /// Buffering enabling.
        /// </summary>
        [Key("buffering")]
        public Buffering Buffering { get; set; } = Buffering.Default;

        /// <summary>
        /// Max buffering frame number.
        /// </summary>
        [Key("num")]
        public int Num { get; set; } = BufferNumDefault;

        /// <summary>
        /// Buffering format.
        /// </summary>
        [Key("format")]
        public BufferingFormat Format { get; set; } = BufferingFormat.Default;

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Key}({this.Buffering}, num={this.Num}, {this.Format})";
        }
    }
}
