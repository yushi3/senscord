/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Set the image crop information.
    /// </summary>
    [MessagePackObject]
    public class ImageCropProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "image_crop_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Left.
        /// </summary>
        [Key("left")]
        public int Left { get; set; }

        /// <summary>
        /// Top.
        /// </summary>
        [Key("top")]
        public int Top { get; set; }

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
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return MessagePackSerializer.SerializeToJson(this);
        }
    }

    /// <summary>
    /// Property for bounds of the image crop.
    /// </summary>
    [MessagePackObject]
    public class ImageCropBoundsProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "image_crop_bounds_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Left.
        /// </summary>
        [Key("left")]
        public int Left { get; set; }

        /// <summary>
        /// Top.
        /// </summary>
        [Key("top")]
        public int Top { get; set; }

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
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return MessagePackSerializer.SerializeToJson(this);
        }
    }
}
