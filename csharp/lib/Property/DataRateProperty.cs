/*
 * SPDX-FileCopyrightText: 2021-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using System.Collections.Generic;
using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Structure for handling Data rate properties.
    /// </summary>
    [MessagePackObject]
    public class DataRateProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "data_rate_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Elements.
        /// </summary>
        [Key("elements")]
        public List<DataRateElement> Elements { get; set; } =
            new List<DataRateElement>();

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Key}({string.Join(", ", this.Elements)})";
        }
    }

    /// <summary>
    /// Structure for Data rate element.
    /// </summary>
    [MessagePackObject]
    public class DataRateElement
    {
        /// <summary>
        /// Data rate size.
        /// </summary>
        [Key("size")]
        public float Size { get; set; }

        /// <summary>
        /// Data rate name.
        /// </summary>
        [Key("name")]
        public string Name { get; set; } = string.Empty;

        /// <summary>
        /// Data rate unit.
        /// </summary>
        [Key("unit")]
        public string Unit { get; set; } = string.Empty;

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"size={this.Size}, name={this.Name}, unit={this.Unit}";
        }
    }
}
