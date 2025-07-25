/*
 * SPDX-FileCopyrightText: 2021-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_types;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * An elemental class for TemporalContrastStream.
 *
 * @see "senscord::TemporalContrastEvent"
 */
public class TemporalContrastEvent {
    final int TIMESLICE_EVENT_SIZE = 6;

    public short x;
    public short y;
    public byte p;

    /**
     * Temporal contrast types for p.
     * 
     * @see "senscord::TemporalContrast"
     */
    public static interface TemporalContrast {
        /**
         * Negative event.
         */
        public static final int SENSCORD_TEMPORAL_CONTRAST_NEGATIVE = -1;

        /**
         * Event is none.
         */
        public static final int SENSCORD_TEMPORAL_CONTRAST_NONE = 0;

        /**
         * Positive event.
         */
        public static final int SENSCORD_TEMPORAL_CONTRAST_POSITIVE = 1;
    }

    /**
     * Constructor.
     */
    public TemporalContrastEvent() {
        this.x = 0;
        this.y = 0;
        this.p = 0;
    }

    /**
     * Get x.
     *
     * @return x
     */
    public short GetX() {
        return x;
    }

    /**
     * Get y.
     *
     * @return y
     */
    public short GetY() {
        return y;
    }

    /**
     * Get p.
     *
     * @return p
     */
    public byte GetP() {
        return p;
    }

    /**
     * Set data from ByteBuffer object.
     *
     * @param byte_buffer byte buffer
     * @return the position after data conversion
     */
    public int SetFromByteBuffer(ByteBuffer byte_buffer) {
        if (byte_buffer.capacity() < TIMESLICE_EVENT_SIZE) {
            throw new IndexOutOfBoundsException(
                          "Timeslice index buffer overrun at TimesliceEvent header.");
        }
        byte_buffer.order(ByteOrder.LITTLE_ENDIAN);
        this.x = byte_buffer.getShort();
        this.y = byte_buffer.getShort();
        this.p = byte_buffer.get();
        byte_buffer.position(byte_buffer.position() + 1);  // reserve
        return byte_buffer.position();
    }
}
