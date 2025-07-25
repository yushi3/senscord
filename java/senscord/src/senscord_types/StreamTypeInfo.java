/*
 * SPDX-FileCopyrightText: 2019-2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_types;

import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

/**
 * The information of stream key.
 * 
 * @see "senscord::StreamTypeInfo"
 */
public class StreamTypeInfo extends Structure {
    public static class ByReference extends StreamTypeInfo implements Structure.ByReference {
    }

    public String key;
    public String type;
    public String id;

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("key", "type", "id");
    }

    /**
     * Get stream key.
     * 
     * @return stream key
     */
    public String GetKey() {
        return key;
    }

    /**
     * Get stream type.
     * 
     * @return stream type
     */
    public String GetType() {
        return type;
    }

    /**
     * Get id.
     *
     * @return id
     */
    public String GetId() {
        return id;
    }
}
