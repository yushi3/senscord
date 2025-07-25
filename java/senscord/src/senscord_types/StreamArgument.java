/*
 * SPDX-FileCopyrightText: 2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_types;

import java.util.Arrays;
import java.util.List;

import com.sun.jna.Native;
import com.sun.jna.Structure;

/**
 * Stream argument element.
 */
public class StreamArgument extends Structure {
    public static class ByReference extends StreamArgument implements Structure.ByReference {
    }

    public byte[] name;
    public byte[] value;

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("name", "value");
    }

    /**
     * Constuctor.
     */
    public StreamArgument() {
        super();
        final int STREAM_ARGUMENT_NAME_LENGTH = 32;
        final int STREAM_ARGUMENT_VALUE_LENGTH = 256;
        name = new byte[STREAM_ARGUMENT_NAME_LENGTH];
        value = new byte[STREAM_ARGUMENT_VALUE_LENGTH];
    }

    /**
     * Get name.
     * @return name
     */
    public String GetName() {
        return Native.toString(name);
    }

    /**
     * Get value.
     * @return value
     */
    public String GetValue() {
        return Native.toString(value);
    }

    /**
     * Set name.
     * @param name name
     */
    public void SetName(String name) {
        Arrays.fill(this.name, (byte) 0);
        System.arraycopy(name.getBytes(), 0, this.name, 0, name.length());
    }

    /**
     * Set value.
     * @param value value
     */
    public void SetValue(String value) {
        Arrays.fill(this.value, (byte) 0);
        System.arraycopy(value.getBytes(), 0, this.value, 0, value.length());
    }
}
