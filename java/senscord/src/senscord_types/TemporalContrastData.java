/*
 * SPDX-FileCopyrightText: 2021-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_types;

import com.sun.jna.Pointer;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;

import senscord.SenscordJavaAPI;

/**
 * The most outer class for TemporalContrastStream data structure.
 *
 * @see "senscord::TemporalContrastData"
 */
public class TemporalContrastData {
    final int TEMPORAL_CONTRAST_HEADER_SIZE = 16;

    public int count;
    public ArrayList<TemporalContrastEventsTimeslice> bundles;

    /**
     * Constructor.
     *
     * @param raw_data TemporalContrast RawData.
     */
    public TemporalContrastData(RawData.ByReference raw_data) {
        this.count = 0;
        this.bundles = new ArrayList<TemporalContrastEventsTimeslice>();

        if (raw_data == null) {
            throw new IllegalArgumentException("Argument is null.");
        }

        String expected_type =
            SenscordJavaAPI.SenscordCLibrary.RawDataType.SENSCORD_RAW_DATA_TYPE_TEMPORAL_CONTRAST;
        if (!expected_type.equals(raw_data.GetType())) {
            StringBuilder message = new StringBuilder();
            message.append("Invalid RawData type ");
            message.append(raw_data.GetType());
            message.append(".");
            throw new IllegalArgumentException(message.toString());
       }

       SetFromPointer(raw_data.GetAddress(), raw_data.GetSize());
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
    public ArrayList<TemporalContrastEventsTimeslice> GetBundles() {
        return bundles;
    }

    /**
     * Set data using the pointer for binary data.
     *
     * @param pointer data pointer
     * @param size data size
     */
    private void SetFromPointer(Pointer pointer, int size) {
        // Note: won't work with big endian
        if (pointer == null) {
            throw new IllegalArgumentException("RawData address is null.");
        }

        if (size < TEMPORAL_CONTRAST_HEADER_SIZE) {
            throw new IndexOutOfBoundsException(
                    "Buffer overrun at Temporal Contrast Data header.");
        }

        int diff = 0;
        int count = 0;
        ArrayList<TemporalContrastEventsTimeslice> bundles =
            new ArrayList<TemporalContrastEventsTimeslice>();

        ByteBuffer byte_buffer = pointer.getByteBuffer(0, size);
        byte_buffer.order(ByteOrder.LITTLE_ENDIAN);
        count = byte_buffer.getInt();
        byte_buffer.position(byte_buffer.position() + 12);  // reserve  + skip pointer
        for (int i = 0; i < count; i++) {
            TemporalContrastEventsTimeslice bundle = new TemporalContrastEventsTimeslice();
            diff = bundle.SetFromByteBuffer(byte_buffer.slice());
            bundles.add(bundle);
            byte_buffer.position(byte_buffer.position() + diff);
        }

        this.count = count;
        this.bundles = bundles;
        byte_buffer.clear();
        return;
    }
}
