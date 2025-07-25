/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Structure for the image of the camera exposure.
    /// </summary>
    [MessagePackObject]
    public class WhiteBalanceProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "white_balance_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Mode of white balance.
        /// </summary>
        [Key("mode")]
        public string Mode { get; set; } = string.Empty;

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return string.Format($"{0}({1})", this.Key, this.Mode);
        }
    }

    /// <summary>
    /// White balance modes.
    /// </summary>
    public static partial class WhiteBalanceMode
    {
        /// <summary>
        /// Auto.
        /// </summary>
        public static readonly string Auto = "auto";

        /// <summary>
        /// Manual.
        /// </summary>
        public static readonly string Manual = "manual";
    }
}
