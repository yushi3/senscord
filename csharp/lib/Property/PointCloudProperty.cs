/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Point cloud Property.
    /// If the cloud is unordered, height is 1 and width is the length of
    /// the point cloud.
    /// </summary>
    [MessagePackObject]
    public class PointCloudProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "point_cloud_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Width of the point cloud.
        /// </summary>
        [Key("width")]
        public int Width { get; set; }

        /// <summary>
        /// Height of the point cloud.
        /// </summary>
        [Key("height")]
        public int Height { get; set; }

        /// <summary>
        /// The format of a pixel.
        /// </summary>
        [Key("pixel_format")]
        public string PixelFormat { get; set; } = string.Empty;

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Key}({this.Width}x{this.Height}, PixelFormat={this.PixelFormat})";
        }
    }
}
