/*
 * SPDX-FileCopyrightText: 2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

/**
 * Property of standard register 8 bit read/write access.
 * 
 * @see "senscord::RegisterAccess8Property"
 */
public class RegisterAccess8Property extends Structure {
    public int id;
    public long address;
    public byte data;

    public static class ByReference extends RegisterAccess8Property
            implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("id", "address", "data");
    }

    /**
     * Get register ID.
     * 
     * @return register ID
     */
    public int GetId() {
        return id;
    }

    /**
     * Get target address.
     * 
     * @return target address
     */
    public long GetAddress() {
        return address;
    }

    /**
     * Get write or read data.
     * 
     * @return data
     */
    public byte GetData() {
        return data;
    }

    /**
     * Set register ID.
     * 
     * @param id register ID
     */
    public void SetId(int id) {
        this.id = id;
    }

    /**
     * Set target address.
     * 
     * @param address target address
     */
    public void SetAddress(long address) {
        this.address = address;
    }

    /**
     * Set write or read data.
     * 
     * @param data data
     */
    public void SetData(byte data) {
        this.data = data;
    }
}
