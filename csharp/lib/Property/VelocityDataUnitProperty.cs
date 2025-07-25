/*
 * SPDX-FileCopyrightText: 2021-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Property for obtaining unit of RawData.
    /// </summary>
    [MessagePackObject]
    public class VelocityDataUnitProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "velocity_data_unit_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Unit of data of velocity.
        /// </summary>
        [Key("velocity")]
        public VelocityUnit Velocity { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return string.Format(
                "{0}(velocity={1})",
                this.Key, this.Velocity);
        }
    }

    /// <summary>
    /// Units used for velocity.
    /// </summary>
    public enum VelocityUnit
    {
        /// <summary>
        /// sensor not supported.
        /// </summary>
        NotSupported,

        /// <summary>
        /// Unit:[m/s].
        /// </summary>
        MetrePerSecond,

        /// <summary>
        /// Unit:[pixel/s].
        /// </summary>
        PixelPerSecond,
    }
}
