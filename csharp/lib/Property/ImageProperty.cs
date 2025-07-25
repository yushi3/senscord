/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Structures that handle properties of Raw data of Image and Depth data.
    /// </summary>
    [MessagePackObject]
    public class ImageProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "image_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Width.
        /// </summary>
        [Key("width")]
        public int Width { get; set; }

        /// <summary>
        /// Height.
        /// </summary>
        [Key("height")]
        public int Height { get; set; }

        /// <summary>
        /// Stride bytes.
        /// </summary>
        [Key("stride_bytes")]
        public int StrideBytes { get; set; }

        /// <summary>
        /// Pixel format.
        /// </summary>
        [Key("pixel_format")]
        public string PixelFormat { get; set; } = string.Empty;

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return string.Format(
                "{0}({1}x{2}, StrideBytes={3}, PixelFormat={4})",
                this.Key, this.Width, this.Height, this.StrideBytes, this.PixelFormat);
        }
    }
}
