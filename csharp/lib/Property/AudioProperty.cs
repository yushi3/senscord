/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Structure containing information about the audio raw data.
    /// </summary>
    [MessagePackObject]
    public class AudioProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "audio_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Audio format.
        /// </summary>
        [Key("format")]
        public string Format { get; set; } = string.Empty;

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Key}({this.Format})";
        }
    }

    /// <summary>
    /// Audio format.
    /// </summary>
    public static partial class AudioFormat
    {
        /// <summary>
        /// Linear PCM.
        /// </summary>
        public static readonly string LINEAR_PCM = "audio_lpcm";
    }
}
