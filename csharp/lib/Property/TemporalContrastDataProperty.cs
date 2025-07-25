/*
 * SPDX-FileCopyrightText: 2020-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Property for TemporalContrastStream.
    /// </summary>
    [MessagePackObject]
    public class TemporalContrastDataProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "pixel_polarity_data_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// The trigger type.
        /// </summary>
        [Key("trigger_type")]
        public TemporalContrastTriggerType TriggerType { get; set; }

        /// <summary>
        /// event count used when trigger type is TriggerTypeEvent.
        /// </summary>
        [Key("event_count")]
        public int EventCount { get; set; }

        /// <summary>
        /// The time used for image frame generation [usec].
        /// </summary>
        [Key("accumulation_time")]
        public int AccumulationTime { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return string.Format(
                "{0}(trigger_type={1}, event_count={2}, accumulation_time={3})",
                this.Key, this.TriggerType, this.EventCount, this.AccumulationTime);
        }
    }

    /// <summary>
    /// Trigger types for TemporalContrastStream.
    /// </summary>
    public enum TemporalContrastTriggerType
    {
        /// <summary>
        /// Frame generation will be triggered based on time (frame rate).
        /// </summary>
        TriggerTypeTime,

        /// <summary>
        /// Frame generation will be triggered based on the number of events.
        /// </summary>
        TriggerTypeEvent,
    }

    /// <summary>
    /// Property for PixelPolarityStream
    /// will be replaced by TemporalContrastDataProperty.
    /// </summary>
    [MessagePackObject]
    public class PixelPolarityDataProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "pixel_polarity_data_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// The trigger type.
        /// </summary>
        [Key("trigger_type")]
        public PixelPolarityTriggerType TriggerType { get; set; }

        /// <summary>
        /// event count used when trigger type is TriggerTypeEvent.
        /// </summary>
        [Key("event_count")]
        public int EventCount { get; set; }

        /// <summary>
        /// The time used for image frame generation [usec].
        /// </summary>
        [Key("accumulation_time")]
        public int AccumulationTime { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return string.Format(
                "{0}(trigger_type={1}, event_count={2}, accumulation_time={3})",
                this.Key, this.TriggerType, this.EventCount, this.AccumulationTime);
        }
    }

    /// <summary>
    /// Trigger types for PixelPolarityStream
    /// will be replaced by TemporalContrastTriggerType.
    /// </summary>
    public enum PixelPolarityTriggerType
    {
        /// <summary>
        /// Frame generation will be triggered based on time (frame rate).
        /// </summary>
        TriggerTypeTime,

        /// <summary>
        /// Frame generation will be triggered based on the number of events.
        /// </summary>
        TriggerTypeEvent,
    }
}
