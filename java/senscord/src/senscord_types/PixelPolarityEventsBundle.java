/*
 * SPDX-FileCopyrightText: 2020-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_types;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;

/**
 * An inner class for PixelPolarityStream data structure.
 * 
 * @deprecated replaced by "TemporalContrastEventsTimeslice"
 * @see "senscord::PixelPolarityEventsBundle"
 */
public class PixelPolarityEventsBundle {
    public long timestamp;
    public int count;
    public ArrayList<PixelPolarityEvent> events;

    /**
     * Constructor.
     */
    public PixelPolarityEventsBundle() {
        this.timestamp = 0;
        this.count = 0;
        this.events = new ArrayList<PixelPolarityEvent>();
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
    public ArrayList<PixelPolarityEvent> GetEvents() {
        return events;
    }

    /**
     * Set data from ByteBuffer object.
     *
     * @param byte_buffer byte buffer
     * @return the position after data conversion
     */
    public int SetFromByteBuffer(ByteBuffer byte_buffer) {
        int diff = 0;
        byte_buffer.order(ByteOrder.LITTLE_ENDIAN);
        this.timestamp = byte_buffer.getLong();
        this.count = byte_buffer.getInt();
        byte_buffer.position(byte_buffer.position() + 12);  // reserve + skip pointer
        for (int i = 0; i < this.count; i++) {
            PixelPolarityEvent event = new PixelPolarityEvent();
            diff = event.SetFromByteBuffer(byte_buffer.slice());
            this.events.add(event);
            byte_buffer.position(byte_buffer.position() + diff);
        }
        return byte_buffer.position();
    }
}
