/*
 * SPDX-FileCopyrightText: 2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Property for coordinate system information.
    /// </summary>
    [MessagePackObject]
    public class CoordinateSystemProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "coordinate_system_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// System handed for CoordinateSystem.
        /// </summary>
        [Key("handed")]
        public SystemHanded Handed { get; set; }

        /// <summary>
        /// Up axis for CoordinateSystem.
        /// </summary>
        [Key("up_axis")]
        public UpAxis UpAxis { get; set; }

        /// <summary>
        /// Forward axis for CoordinateSystem.
        /// </summary>
        [Key("forward_axis")]
        public ForwardAxis ForwardAxis { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return string.Format(
                "{0}(handed={1}, up_axis={2}, forward_axis={3}",
                this.Key, this.Handed, this.UpAxis, this.ForwardAxis);
        }
    }

    /// <summary>
    /// Used for system handed.
    /// </summary>
    public enum SystemHanded
    {
        /// <summary>
        /// Left-handed system.
        /// </summary>
        Left,

        /// <summary>
        /// Right-handed system.
        /// </summary>
        Right,
    }

    /// <summary>
    /// Used for up axis.
    /// </summary>
    public enum UpAxis
    {
        /// <summary>
        /// Up axis undefined.
        /// </summary>
        Undefined,

        /// <summary>
        /// X axis up.
        /// </summary>
        PlusX,

        /// <summary>
        /// Y axis up.
        /// </summary>
        PlusY,

        /// <summary>
        /// Z axis up.
        /// </summary>
        PlusZ,

        /// <summary>
        /// X axis down.
        /// </summary>
        MinusX,

        /// <summary>
        /// Y axis down.
        /// </summary>
        MinusY,

        /// <summary>
        /// Z axis down.
        /// </summary>
        MinusZ,
    }

    /// <summary>
    /// Used for forward axis.
    /// </summary>
    public enum ForwardAxis
    {
        /// <summary>
        /// Forward axis undefined.
        /// </summary>
        Undefined,

        /// <summary>
        /// X axis up.
        /// </summary>
        PlusX,

        /// <summary>
        /// Y axis up.
        /// </summary>
        PlusY,

        /// <summary>
        /// Z axis up.
        /// </summary>
        PlusZ,

        /// <summary>
        /// X axis down.
        /// </summary>
        MinusX,

        /// <summary>
        /// Y axis down.
        /// </summary>
        MinusY,

        /// <summary>
        /// Z axis down.
        /// </summary>
        MinusZ,
    }
}
