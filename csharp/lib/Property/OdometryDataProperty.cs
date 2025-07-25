/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Position Data Property.
    /// </summary>
    [MessagePackObject]
    public class OdometryDataProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "odometry_data_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Coordinate system.
        /// </summary>
        [Key("coordinate_system")]
        public CoordinateSystem CoordinateSystem { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Key}({this.CoordinateSystem})";
        }
    }

    /// <summary>
    /// Types of coordinate system.
    /// </summary>
    public enum CoordinateSystem
    {
        /// <summary>
        /// World coordinate system.
        /// </summary>
        World,

        /// <summary>
        /// Local coordinate system.
        /// </summary>
        Local,

        /// <summary>
        /// Camera coordinate system.
        /// </summary>
        Camera,
    }
}
