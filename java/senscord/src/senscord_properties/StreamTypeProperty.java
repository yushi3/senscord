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
 * Property for the type of the stream.
 * 
 * @see "senscord::StreamTypeProperty"
 */
public class StreamTypeProperty extends Structure {
    public byte[] type;

    public static class ByReference extends StreamTypeProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("type");
    }

    public StreamTypeProperty() {
        super();
        type = new byte[64];
    }

    /**
     * Get stream type.
     * 
     * @return stream type
     */
    public String GetType() {
        return Native.toString(type);
    }

}
