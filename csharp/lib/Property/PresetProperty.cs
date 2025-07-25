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
    /// Property for the list of property's preset IDs.
    /// </summary>
    [MessagePackObject]
    public class PresetListProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "preset_list_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Preset ID + description.
        /// </summary>
        [Key("presets")]
        public Dictionary<long, string> Presets { get; set; } =
            new Dictionary<long, string>();

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Key}({string.Join(", ", this.Presets)})";
        }
    }

    /// <summary>
    /// Property for the property's preset.
    /// </summary>
    [MessagePackObject]
    public class PresetProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "preset_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Preset ID.
        /// </summary>
        [Key("id")]
        public long Id { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Key}({this.Id})";
        }
    }
}
