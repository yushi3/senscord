/*
 * SPDX-FileCopyrightText: 2021-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_types;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;

/**
 * An inner class for TemporalContrastStream data structure.
 *
 * @see "senscord::TemporalContrastEventsTimeslice"
 */
public class TemporalContrastEventsTimeslice {
    final int TEMPORAL_CONTRAST_TIMESLICE_HEADER_SIZE = 24;

    public long timestamp;
    public int count;
    public ArrayList<TemporalContrastEvent> events;

    /**
     * Constructor.
     */
    public TemporalContrastEventsTimeslice() {
        this.timestamp = 0;
        this.count = 0;
        this.events = new ArrayList<TemporalContrastEvent>();
    }

    /**
     * Get timestamp.
     *
     * @return timestamp
     */
    public long GetTimestamp() {
        return timestamp;
    }

    /**
     * Get count.
     *
     * @return count
     */
    public int GetCount() {
        return count;
    }

    /**
     * Get events.
     *
     * @return events
     */
    public ArrayList<TemporalContrastEvent> GetEvents() {
        return events;
    }

    /**
     * Set data from ByteBuffer object.
     *
     * @param byte_buffer byte buffer
     * @return the position after data conversion
     */
    public int SetFromByteBuffer(ByteBuffer byte_buffer) {
        if (byte_buffer.capacity() < TEMPORAL_CONTRAST_TIMESLICE_HEADER_SIZE) {
            throw new IndexOutOfBoundsException(
                          "Timeslice index buffer overrun at EventsTimeslice header.");
        }

        int diff = 0;
        byte_buffer.order(ByteOrder.LITTLE_ENDIAN);
        this.timestamp = byte_buffer.getLong();
        this.count = byte_buffer.getInt();
        byte_buffer.position(byte_buffer.position() + 12);  // reserve + skip pointer
        for (int i = 0; i < this.count; i++) {
            TemporalContrastEvent event = new TemporalContrastEvent();
            diff = event.SetFromByteBuffer(byte_buffer.slice());
            this.events.add(event);
            byte_buffer.position(byte_buffer.position() + diff);
        }
        return byte_buffer.position();
    }
}
