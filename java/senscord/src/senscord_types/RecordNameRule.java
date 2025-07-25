/*
 * SPDX-FileCopyrightText: 2021-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_types;

import com.sun.jna.Native;
import com.sun.jna.Structure;

import java.util.Arrays;
import java.util.List;

/**
 * Information about the record for directory name rule.
 */

public class RecordNameRule extends Structure {
    public byte[] directory_type;
    public byte[] format;

    public static class ByReference extends RecordNameRule implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("directory_type", "format");
    }

    /**
     * Constructor.
     */
    public RecordNameRule() {
        super();
        directory_type = new byte[64];
        format = new byte[128];
    }

    /**
     * Record directory type.
     */
    public static interface DirectoryType {
        public static final String SENSCORD_RECORD_DIRECTORY_TOP = "top";
    }

    /**
     * Get directory type.
     * 
     * @return directory type
     */
    public String GetDirectoryType() {
        return Native.toString(directory_type);
    }

    /**
     * Set directory type.
     * 
     * @param directory_type directory type
     */
    public void SetDirectoryType(String directory_type) {
        Arrays.fill(this.directory_type, (byte) 0);
        System.arraycopy(
            directory_type.getBytes(), 0,
            this.directory_type, 0, directory_type.length());
    }

    /**
     * Get format.
     * 
     * @return format.
     */
    public String GetFormat() {
        return Native.toString(format);
    }

    /**
     * Set format.
     * 
     * @param format format.
     */
    public void SetFormat(String format) {
        Arrays.fill(this.format, (byte) 0);
        System.arraycopy(
            format.getBytes(), 0,
            this.format, 0, format.length());
    }
}
