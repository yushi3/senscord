/*
 * SPDX-FileCopyrightText: 2020-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Set the ROI information.
    /// </summary>
    [MessagePackObject]
    public class RoiProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "roi_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// The X coordinate of left top corner of ROI.
        /// </summary>
        [Key("left")]
        public int Left { get; set; }

        /// <summary>
        /// The Y coordinate of left top corner of ROI.
        /// </summary>
        [Key("top")]
        public int Top { get; set; }

        /// <summary>
        /// The width of the ROI.
        /// </summary>
        [Key("width")]
        public int Width { get; set; }

        /// <summary>
        /// The height of the ROI.
        /// </summary>
        [Key("height")]
        public int Height { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return string.Format(
                "{0}(left={1} top={2} width={3} height={4})",
                this.Key, this.Left, this.Top, this.Width, this.Height);
        }
    }
}
