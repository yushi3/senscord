/*
 * SPDX-FileCopyrightText: 2020-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_types;

import com.sun.jna.Pointer;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;

/**
 * The most outer class for PixelPolarityStream data structure.
 * 
 * @deprecated replaced by "TemporalContrastData"
 * @see "senscord::PixelPolarityData"
 */
public class PixelPolarityData {
    public int count;
    public ArrayList<PixelPolarityEventsBundle> bundles;

    /**
     * Constructor.
     */
    public PixelPolarityData() {
        this.count = 0;
        this.bundles = new ArrayList<PixelPolarityEventsBundle>();
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
    public ArrayList<PixelPolarityEventsBundle> GetBundles() {
        return bundles;
    }

    /**
     * Set data using the pointer for binary data.
     *
     * @param pointer data pointer
     * @param size data size
     */
    public void SetFromPointer(Pointer pointer, int size) {
        // Note: won't work with big endian
        if (pointer == null) {
            return;
        }

        int diff = 0;
        ByteBuffer byte_buffer = pointer.getByteBuffer(0, size);
        byte_buffer.order(ByteOrder.LITTLE_ENDIAN);
        this.count = byte_buffer.getInt();
        byte_buffer.position(byte_buffer.position() + 12);  // reserve  + skip pointer
        for (int i = 0; i < this.count; i++) {
            PixelPolarityEventsBundle bundle = new PixelPolarityEventsBundle();
            diff = bundle.SetFromByteBuffer(byte_buffer.slice());
            this.bundles.add(bundle);
            byte_buffer.position(byte_buffer.position() + diff);
        }
        byte_buffer.clear();
        return;
    }
}
