/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using System.Collections.Generic;
using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Property for channel information.
    /// </summary>
    [MessagePackObject]
    public class ChannelInfoProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "channel_info_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Channel information list (Key=Channel ID).
        /// </summary>
        [Key("channels")]
        public Dictionary<long, ChannelInfo> Channels { get; set; } =
            new Dictionary<long, ChannelInfo>();

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Key}({string.Join(", ", this.Channels)})";
        }
    }

    /// <summary>
    /// Channel information.
    /// </summary>
    [MessagePackObject]
    public class ChannelInfo
    {
        /// <summary>
        /// Type of raw data.
        /// </summary>
        [Key("raw_data_type")]
        public string RawDataType { get; set; } = string.Empty;

        /// <summary>
        /// Channel description.
        /// </summary>
        [Key("description")]
        public string Description { get; set; } = string.Empty;

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.RawDataType}: {this.Description}";
        }
    }
}
