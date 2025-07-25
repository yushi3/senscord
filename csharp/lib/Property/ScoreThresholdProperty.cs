/*
 * SPDX-FileCopyrightText: 2021-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Structure for handling threshold of the score.
    /// </summary>
    [MessagePackObject]
    public class ScoreThresholdProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "score_threshold_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Threshold of the score.
        /// </summary>
        [Key("score_threshold")]
        public float ScoreThreshold { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Key}(score_threshold={this.ScoreThreshold})";
        }
    }
}
