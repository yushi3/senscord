/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using System.Collections.Generic;
using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Object detection data.
    /// </summary>
    [MessagePackObject]
    public class ObjectDetectionData
    {
        /// <summary>
        /// Raw data type.
        /// </summary>
        [IgnoreMember]
        public string Type { get; } = RawDataType.ObjectDetection;

        /// <summary>
        /// ObjectDetection data.
        /// </summary>
        [Key("data")]
        public List<DetectedObjectInformation> Data { get; set; } =
            new List<DetectedObjectInformation>();

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Type}({string.Join(", ", this.Data)})";
        }
    }

    /// <summary>
    /// Detected object information.
    /// </summary>
    [MessagePackObject]
    public class DetectedObjectInformation
    {
        /// <summary>
        /// Class id of detected object.
        /// </summary>
        [Key("class_id")]
        public int ClassId { get; set; }

        /// <summary>
        /// Score of detected object.
        /// </summary>
        [Key("score")]
        public float Score { get; set; }

        /// <summary>
        /// Detected object area.
        /// </summary>
        [Key("box")]
        public RectangleRegionParameter Box { get; set; } =
            new RectangleRegionParameter();

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return string.Format(
                "{0}({1}, {2}, {3})",
                this.GetType().FullName, this.ClassId, this.Score, this.Box);
        }
    }
}
