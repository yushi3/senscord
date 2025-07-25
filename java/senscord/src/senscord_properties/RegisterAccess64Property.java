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
 * Property of standard register 64 bit read/write access.
 * 
 * @see "senscord::RegisterAccess64Property"
 */
public class RegisterAccess64Property extends Structure {
    public int id;
    public long address;
    public long data;

    public static class ByReference extends RegisterAccess64Property
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
    public long GetData() {
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
    public void SetData(long data) {
        this.data = data;
    }
}
