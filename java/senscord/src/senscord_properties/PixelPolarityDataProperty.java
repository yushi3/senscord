/*
 * SPDX-FileCopyrightText: 2020-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import java.util.Arrays;
import java.util.List;

import com.sun.jna.Structure;

/**
 * PixelPolarityDataProperty.
 * 
 * @deprecated replaced by "TemporalContrastDataProperty"
 * @see "senscord::PixelPolarityDataProperty"
 */
public class PixelPolarityDataProperty extends Structure {

    public static interface PixelPolarityTriggerType {
        public static final int SENSCORD_TRIGGR_TYPE_TIME = 0;
        public static final int SENSCORD_TRIGGR_TYPE_EVENT = 1;
    }

    public int trigger_type;
    public int event_count;
    public int accumulation_time;

    public static class ByReference extends PixelPolarityDataProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("trigger_type", "event_count", "accumulation_time");
    }

    public PixelPolarityDataProperty() {
        super();
    }

    /**
     * Get trigger type.
     *
     * @return the trigger type
     */
    public int GetTriggerType() {
        return trigger_type;
    }

    /**
     * Set trigger type.
     *
     * @param trigger_type the trigger type
     */
    public void SetTriggerType(int trigger_type) {
        this.trigger_type = trigger_type;
    }

    /**
     * Get event count.
     *
     * @return event_count
     */
    public int GetEventCount() {
        return event_count;
    }

    /**
     * Set event count.
     *
     * @param event_count the number of events
     */
    public void SetEventCount(int event_count) {
        this.event_count = event_count;
    }

    /**
     * Get accumulation time.
     *
     * @return accumulation_time
     */
    public int GetAccumulationTime() {
        return accumulation_time;
    }

    /**
     * Set accumulation time.
     *
     * @param accumulation_time the accumulation time
     */
    public void SetAccumulationTime(int accumulation_time) {
        this.accumulation_time = accumulation_time;
    }

}
