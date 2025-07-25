/*
 * SPDX-FileCopyrightText: 2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_types;

import com.sun.jna.Native;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

/**
 * Recorder format.
 */
public class RecorderFormat extends Structure {
    public byte[] name;

    public static class ByReference extends RecorderFormat implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("name");
    }

    public RecorderFormat() {
        super();
        name = new byte[64];
    }

    public static interface Format {
        public static final String SENSCORD_RECORDING_FORMAT_RAW = "raw";
        public static final String SENSCORD_RECORDING_FORMAT_COMPOSITE_RAW = "composite_raw";
        public static final String SENSCORD_RECORDING_FORMAT_SKV = "skv";
    }

    /**
     * Get recorder format.
     * 
     * @return recorder format
     */
    public String GetName() {
        return Native.toString(name);
    }

    /**
     * Set recorder format.
     * 
     * @param name recorder format name
     */
    public void SetName(String name) {
        Arrays.fill(this.name, (byte) 0);
        System.arraycopy(name.getBytes(), 0, this.name, 0, name.length());
    }

}
