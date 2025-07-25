/*
 * SPDX-FileCopyrightText: 2019-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Structure;

import java.util.Arrays;
import java.util.List;

import senscord_types.RecorderFormat;

/**
 * RecordProperty.
 * 
 * @see "senscord::kRecordPropertyKey"
 */

public class RecorderListProperty extends Structure {
    public int count;
    public RecorderFormat[] format;

    public static class ByReference extends RecorderListProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("count", "format");
    }

    /**
     * Constructor.
     */
    public RecorderListProperty() {
        super();
        final int RECORDER_FORMAT_NUM = 64;
        format = new RecorderFormat[RECORDER_FORMAT_NUM];
        for (int i = 0; i < RECORDER_FORMAT_NUM; i++) {
            format[i] = new RecorderFormat();
        }

    }

    /**
     * Get count.
     * 
     * @return Count of the recorder formats
     */
    public int GetCount() {
        return count;
    }

    /**
     * Set count.
     * 
     * @param count Count of the recorder formats
     */
    public void SetCount(int count) {
        this.count = count;
    }

    /**
     * Get RecorderFormat.
     * 
     * @return RecorderFormat
     */
    public RecorderFormat[] GetRecorderFormat() {
        return format;
    }

}
