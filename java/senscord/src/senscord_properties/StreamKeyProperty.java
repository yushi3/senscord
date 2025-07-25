/*
 * SPDX-FileCopyrightText: 2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Native;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

/**
 * Property for the key of the stream.
 * 
 * @see "senscord::StreamKeyProperty"
 */
public class StreamKeyProperty extends Structure {
    public byte[] key;

    public static class ByReference extends StreamKeyProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("key");
    }

    public StreamKeyProperty() {
        super();
        key = new byte[64];
    }

    /**
     * Get stream key.
     * 
     * @return stream key
     */
    public String GetKey() {
        return Native.toString(key);
    }
}
