/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Structure used to acquire field angle of camera.
    /// </summary>
    [MessagePackObject]
    public class LensProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "lens_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// The horizontal viewing angle of the lens.
        /// </summary>
        [Key("horizontal_field_of_view")]
        public float HorizontalFov { get; set; }

        /// <summary>
        /// The vertical viewing angle of the lens.
        /// </summary>
        [Key("vertical_field_of_view")]
        public float VerticalFov { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Key}(horizontal={this.HorizontalFov}, vertical={this.VerticalFov})";
        }
    }
}
