/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Property for the current buffering frames.
    /// </summary>
    [MessagePackObject]
    public class CurrentFrameNumProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "current_frame_num_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Arrived number.
        /// </summary>
        [Key("arrived_number")]
        public int ArrivedNumber { get; set; }

        /// <summary>
        /// Received number.
        /// </summary>
        [Key("received_number")]
        public int ReceivedNumber { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Key}(arrived={this.ArrivedNumber}, received={this.ReceivedNumber})";
        }
    }
}
