/*
 * SPDX-FileCopyrightText: 2020-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using System.Collections.Generic;
using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Property for temperature.
    /// </summary>
    [MessagePackObject]
    public class TemperatureProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "temperature_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Information for each temperature sensor (Key = Sensor id).
        /// </summary>
        [Key("temperatures")]
        public Dictionary<int, TemperatureInfo> Temperatures { get; set; } =
            new Dictionary<int, TemperatureInfo>();

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Key}({string.Join(", ", this.Temperatures)})";
        }
    }

    /// <summary>
    /// temperature information.
    /// </summary>
    [MessagePackObject]
    public class TemperatureInfo
    {
        /// <summary>
        /// temperature.
        /// </summary>
        [Key("temperature")]
        public float Temperature { get; set; }

        /// <summary>
        /// Temperature description.
        /// </summary>
        [Key("description")]
        public string Description { get; set; } = string.Empty;

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return string.Join(", ", this.Temperature, this.Description);
        }
    }
}