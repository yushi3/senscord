/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Property for player playback parameters.
    /// </summary>
    [MessagePackObject]
    public class PlayProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "play_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Path of the recorded data.
        /// </summary>
        [Key("target_path")]
        public string TargetPath { get; set; } = string.Empty;

        /// <summary>
        /// Playback start offset.
        /// </summary>
        [Key("start_offset")]
        public int StartOffset { get; set; }

        /// <summary>
        /// Number of frames to play.
        /// </summary>
        [Key("count")]
        public int Count { get; set; }

        /// <summary>
        /// Frame playback speed.
        /// </summary>
        [Key("speed")]
        public PlaySpeed Speed { get; set; }

        /// <summary>
        /// Playback mode.
        /// </summary>
        [Key("mode")]
        public PlayModeProperty Mode { get; set; } = new PlayModeProperty();

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
    /// Property for player playback repeat mode.
    /// </summary>
    [MessagePackObject]
    public class PlayModeProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "play_mode_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Specify repeat playback.
        /// </summary>
        [Key("repeat")]
        public bool Repeat { get; set; }

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
    /// Property for player playback pause mode.
    /// </summary>
    [MessagePackObject]
    public class PlayPauseProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "play_pause_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Specify pause playback.
        /// </summary>
        [Key("pause")]
        public bool Pause { get; set; }

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
    /// Property that indicates the playback position in the player function.
    /// </summary>
    [MessagePackObject]
    public class PlayPositionProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "play_position_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Playback position.
        /// </summary>
        [Key("position")]
        public int Position { get; set; }

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
    /// Frame playback speed in the player.
    /// </summary>
    public enum PlaySpeed
    {
        /// <summary>
        /// Sending based on framerate.
        /// </summary>
        BasedOnFramerate = 0,

        /// <summary>
        /// Sending without framerate.
        /// </summary>
        BestEffort,
    }

    /// <summary>
    /// Property for playback file information.
    /// </summary>
    [MessagePackObject]
    public class PlayFileInfoProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "play_file_info_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Directory path of recorded.
        /// </summary>
        [Key("target_path")]
        public string TargetPath { get; set; } = string.Empty;

        /// <summary>
        /// Recorded date and time.
        /// </summary>
        [Key("record_date")]
        public string RecordDate { get; set; } = string.Empty;

        /// <summary>
        /// Stream key to recorded.
        /// </summary>
        [Key("stream_key")]
        public string StreamKey { get; set; } = string.Empty;

        /// <summary>
        /// Stream type to recorded.
        /// </summary>
        [Key("stream_type")]
        public string StreamType { get; set; } = string.Empty;

        /// <summary>
        /// Number of Frames recorded.
        /// </summary>
        [Key("frame_count")]
        public int FrameCount { get; set; }

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
