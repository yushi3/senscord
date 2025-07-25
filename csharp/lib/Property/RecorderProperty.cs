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
    /// Property for the recording frames.
    /// </summary>
    [MessagePackObject]
    public class RecordProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "record_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// State of recording.
        /// If set to true, recording will start.
        /// Startable only in the stream running state.
        /// </summary>
        [Key("enabled")]
        public bool Enabled { get; set; }

        /// <summary>
        /// Top directory path of recording files.
        /// When to stop, this member is ignored.
        /// </summary>
        [Key("path")]
        public string Path { get; set; } = string.Empty;

        /// <summary>
        /// The count of record frames.
        /// </summary>
        [Key("count")]
        public int Count { get; set; }

        /// <summary>
        /// Format names of each channel ID.
        /// Frames of no specified channel ID will not be recorded.
        /// For get the available formats, use RecorderListProperty.
        /// When to stop, this member is ignored.
        /// </summary>
        [Key("formats")]
        public Dictionary<long, string> Formats { get; set; } =
            new Dictionary<long, string>();

        /// <summary>
        /// Number of the buffering of recording frame queue.
        /// If set zero means the number equals one.
        /// When to stop, this member is ignored.
        /// </summary>
        [Key("buffer_num")]
        public int BufferNum { get; set; }

        /// <summary>
        /// Directory naming rules.
        /// Key is the directory type, value is a format string.
        /// When to stop, this member is ignored.
        /// </summary>
        [Key("name_rules")]
        public Dictionary<string, string> NameRules { get; set; } =
            new Dictionary<string, string>();

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return string.Format(
                "{0}(enabled={1}, path={2}, {3}, {4}, num={5}, {6})",
                this.Key, this.Enabled, this.Path, this.Count, string.Join(", ", this.Formats),
                this.BufferNum, string.Join(", ", this.NameRules));
        }
    }

    /// <summary>
    /// Property for reference the available recording formats.
    /// For stream property getting only.
    /// </summary>
    [MessagePackObject]
    public class RecorderListProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "recorder_list_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// list of formats.
        /// </summary>
        [Key("formats")]
        public List<string> Formats { get; set; } = new List<string>();

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Key}({string.Join(", ", this.Formats)})";
        }
    }

    /// <summary>
    /// Standard recording formats.
    /// </summary>
    public static partial class RecordingFormat
    {
        /// <summary>
        /// Raw.
        /// </summary>
        public static readonly string Raw = "raw";

        /// <summary>
        /// CompositeRaw.
        /// </summary>
        public static readonly string CompositeRaw = "composite_raw";

        /// <summary>
        /// Skv.
        /// </summary>
        public static readonly string Skv = "skv";
    }

    /// <summary>
    /// Record directory types.
    /// </summary>
    public static partial class RecordDirectory
    {
        /// <summary>
        /// Top directory.
        /// </summary>
        public static readonly string Top = "top";
    }
}
