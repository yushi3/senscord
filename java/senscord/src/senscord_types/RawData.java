/*
 * SPDX-FileCopyrightText: 2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_types;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

/**
 * Raw data informations.
 * 
 * @see "senscord::Channel::RawData"
 */
public class RawData extends Structure {
    public static class ByReference extends RawData implements Structure.ByReference {
    }

    public Pointer address;
    public int size;
    public String type;
    public long timestamp;

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("address", "size", "type", "timestamp");
    }

    /**
     * Get virtual address.
     * 
     * @return virtual address
     */
    public Pointer GetAddress() {
        return address;
    }

    /**
     * Get data size.
     * 
     * @return data size
     */
    public int GetSize() {
        return size;
    }

    /**
     * Get data type.
     * 
     * @return data type
     */
    public String GetType() {
        return type;
    }

    /**
     * nanoseconds time stamp captured by the device.
     * 
     * @return time stamp
     */
    public long GetTimestamp() {
        return timestamp;
    }

}
