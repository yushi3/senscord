/*
 * SPDX-FileCopyrightText: 2021-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using System.Collections.Generic;
using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Object tracking data.
    /// </summary>
    [MessagePackObject]
    public class ObjectTrackingData
    {
        /// <summary>
        /// Raw data type.
        /// </summary>
        [IgnoreMember]
        public string Type { get; } = RawDataType.ObjectTracking;

        /// <summary>
        /// ObjectTracking data.
        /// </summary>
        [Key("data")]
        public List<TrackedObjectInformation> Data { get; set; } =
            new List<TrackedObjectInformation>();

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
    /// Tracked object information.
    /// </summary>
    [MessagePackObject]
    public class TrackedObjectInformation
    {
        /// <summary>
        /// Track id of tracked object.
        /// </summary>
        [Key("track_id")]
        public int TrackId { get; set; }

        /// <summary>
        /// Class id of tracked object.
        /// </summary>
        [Key("class_id")]
        public int ClassId { get; set; }

        /// <summary>
        /// Score of tracked object.
        /// </summary>
        [Key("score")]
        public float Score { get; set; }

        /// <summary>
        ///  Velocity (x,y).
        /// </summary>
        [Key("velocity")]
        public Vector2<float> Velocity { get; set; } = new Vector2<float>();

        /// <summary>
        ///  Position (x,y).
        /// </summary>
        [Key("position")]
        public Vector2<int> Position { get; set; } = new Vector2<int>();

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
                "{0}({1}, {2}, {3}, {4}, {5}, {6})",
                this.GetType().FullName, this.TrackId, this.ClassId,
                this.Score, this.Velocity, this.Position, this.Box);
        }
    }
}
