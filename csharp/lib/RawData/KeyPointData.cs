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
    /// Key point data.
    /// </summary>
    [MessagePackObject]
    public class KeyPointData
    {
        /// <summary>
        /// Raw data type.
        /// </summary>
        [IgnoreMember]
        public string Type { get; } = RawDataType.KeyPoint;

        /// <summary>
        /// ObjectDetection data.
        /// </summary>
        [Key("data")]
        public List<DetectedKeyPointInformation> Data { get; set; } =
            new List<DetectedKeyPointInformation>();

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
    public class DetectedKeyPointInformation
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
        /// Detected points.
        /// </summary>
        [Key("key_points")]
        public List<KeyPoint> KeyPoints { get; set; } =
            new List<KeyPoint>();

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return string.Format(
                "{0}({1}, {2}, {3})",
                this.GetType().FullName, this.ClassId, this.Score,
                string.Join(", ", this.KeyPoints));
        }
    }

    /// <summary>
    /// Key point.
    /// </summary>
    [MessagePackObject]
    public class KeyPoint
    {
        /// <summary>
        /// Key point id of detected object.
        /// </summary>
        [Key("key_point_id")]
        public int KeyPointId { get; set; }

        /// <summary>
        /// Score of detected object.
        /// </summary>
        [Key("score")]
        public float Score { get; set; }

        /// <summary>
        /// Detected object area.
        /// </summary>
        [Key("point")]
        public Vector3<float> Point { get; set; } =
            new Vector3<float>();

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return string.Format(
                "{0}({1}, {2}, {3})",
                this.GetType().FullName, this.KeyPointId, this.Score, this.Point);
        }
    }
}
