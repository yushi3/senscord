/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Channel's property for interlace.
    /// </summary>
    [MessagePackObject]
    public class InterlaceProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "interlace_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// contained field type.
        /// </summary>
        [Key("field")]
        public InterlaceField Field { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Key}({this.Field})";
        }
    }

    /// <summary>
    /// Property for interlace information.
    /// </summary>
    [MessagePackObject]
    public class InterlaceInfoProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "interlace_info_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// order of field.
        /// </summary>
        [Key("order")]
        public InterlaceOrder Order { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Key}({this.Order})";
        }
    }

    /// <summary>
    /// The field types of interlace.
    /// </summary>
    public enum InterlaceField
    {
        /// <summary>
        /// Top field.
        /// </summary>
        Top,

        /// <summary>
        /// Bottom field.
        /// </summary>
        Bottom,
    }

    /// <summary>
    /// The order of interlace.
    /// </summary>
    public enum InterlaceOrder
    {
        /// <summary>
        /// Top first.
        /// </summary>
        TopFirst,

        /// <summary>
        /// Bottom first.
        /// </summary>
        BottomFirst,
    }
}
