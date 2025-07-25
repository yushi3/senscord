/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Property of color space type for YUV.
    /// </summary>
    [MessagePackObject]
    public class ColorSpaceProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "color_space_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Encoding.
        /// </summary>
        [Key("encoding")]
        public YCbCrEncoding Encoding { get; set; }

        /// <summary>
        /// Quantization.
        /// </summary>
        [Key("quantization")]
        public YCbCrQuantization Quantization { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Key}(encoding={this.Encoding}, quantization={this.Quantization})";
        }
    }

    /// <summary>
    /// Encoding types for YUV (YCbCr).
    /// </summary>
    public enum YCbCrEncoding
    {
        /// <summary>
        /// Undefined.
        /// </summary>
        Undefined,

        /// <summary>
        /// BT601.
        /// </summary>
        BT601,

        /// <summary>
        /// BT709.
        /// </summary>
        BT709,

        /// <summary>
        /// BT2020.
        /// </summary>
        BT2020,

        /// <summary>
        /// BT2100.
        /// </summary>
        BT2100,
    }

    /// <summary>
    /// Quantization types for YUV (YCbCr).
    /// </summary>
    public enum YCbCrQuantization
    {
        /// <summary>
        /// BT2020.
        /// </summary>
        Undefined,

        /// <summary>
        /// Full range (Y: 0-255,  C: 0-255).
        /// </summary>
        FullRange,

        /// <summary>
        /// Limited range (Y: 16-235, C: 16-240).
        /// </summary>
        LimitedRange,

        /// <summary>
        /// Super white.
        /// </summary>
        SuperWhite,
    }
}
