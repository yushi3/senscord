/*
 * SPDX-FileCopyrightText: 2019-2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Native;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

/**
 * Pose Data Property.
 * 
 * @see "senscord::PoseDataProperty"
 */
public class PoseDataProperty extends Structure {
    public byte[] data_format;

    public static class ByReference extends PoseDataProperty implements Structure.ByReference {
    }

    public static interface PoseDataFormat {
        public static final String Quaternion = "pose_data_quaternion";
        public static final String Matrix = "pose_data_matrix";
    }

    public PoseDataProperty() {
        super();
        data_format = new byte[64];
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("data_format");
    }

    /**
     * Get pose data format.
     * 
     * @return data_format pose data format
     */
    public String GetDataFormat() {
        return Native.toString(data_format);
    }

    /**
     * Set pose data format.
     * 
     * @param data_format pose data format
     */
    public void SetDataFormat(String data_format) {
        Arrays.fill(this.data_format, (byte) 0);
        System.arraycopy(data_format.getBytes(), 0, this.data_format, 0, data_format.length());
    }
}
