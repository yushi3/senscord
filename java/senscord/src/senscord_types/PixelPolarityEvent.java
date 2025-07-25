/*
 * SPDX-FileCopyrightText: 2020-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_types;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * An elemental class for PixelPolarityStream.
 * 
 * @deprecated replaced by "TemporalContrastEvent"
 * @see "senscord::PixelPolarityEvent"
 */
public class PixelPolarityEvent {
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
        byte_buffer.order(ByteOrder.LITTLE_ENDIAN);
        this.x = byte_buffer.getShort();
        this.y = byte_buffer.getShort();
        this.p = byte_buffer.get();
        byte_buffer.position(byte_buffer.position() + 1);  // reserve
        return byte_buffer.position();
    }
}
