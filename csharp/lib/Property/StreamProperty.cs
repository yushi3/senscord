/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Property for the type of the stream.
    /// </summary>
    [MessagePackObject]
    public class StreamTypeProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "stream_type_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Stream type.
        /// </summary>
        [Key("type")]
        public string Type { get; set; } = string.Empty;

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Key}({this.Type})";
        }
    }

    /// <summary>
    /// Property for the key of the stream.
    /// </summary>
    [MessagePackObject]
    public class StreamKeyProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "stream_key_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Stream key.
        /// </summary>
        [Key("stream_key")]
        public string StreamKey { get; set; } = string.Empty;

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Key}({this.StreamKey})";
        }
    }

    /// <summary>
    /// Property for the current state of the stream.
    /// </summary>
    [MessagePackObject]
    public class StreamStateProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "stream_state_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Stream state.
        /// </summary>
        [Key("state")]
        public StreamState State { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Key}({this.State})";
        }
    }

    /// <summary>
    /// Stream state definitions.
    /// </summary>
    public enum StreamState
    {
        /// <summary>
        /// Undefined state.
        /// </summary>
        Undefined = 0,

        /// <summary>
        /// Opened but not started.
        /// </summary>
        Ready,

        /// <summary>
        /// Started.
        /// </summary>
        Running,
    }

    /// <summary>
    /// Stream type.
    /// </summary>
    public static partial class StreamType
    {
        /// <summary>
        /// Image.
        /// </summary>
        public static readonly string Image = "image";

        /// <summary>
        /// Depth.
        /// </summary>
        public static readonly string Depth = "depth";

        /// <summary>
        /// Imu.
        /// </summary>
        public static readonly string Imu = "imu";

        /// <summary>
        /// SLAM.
        /// </summary>
        public static readonly string Slam = "slam";

        /// <summary>
        /// Object detection.
        /// </summary>
        public static readonly string ObjectDetection = "object_detection";

        /// <summary>
        /// Key point.
        /// </summary>
        public static readonly string KeyPoint = "key_point";

        /// <summary>
        /// Temporal contrast.
        /// </summary>
        public static readonly string TemporalContrast = "pixel_polarity";

        /// <summary>
        /// Pixel polarity.
        /// </summary>
        public static readonly string PixelPolarity = "pixel_polarity";

        /// <summary>
        /// Object tracking.
        /// </summary>
        public static readonly string ObjectTracking = "object_tracking";

        /// <summary>
        /// Audio.
        /// </summary>
        public static readonly string AudioPcm = "audio";
    }
}
