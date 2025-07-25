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
 * User data informations.
 * 
 * @see "senscord::Frame::UserData"
 */
public class UserData extends Structure {
    public Pointer address;
    public int size;

    public static class ByReference extends UserData implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("address", "size");
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
     * Set virtual address.
     * 
     * @param address virtual address
     */
    public void SetAddress(Pointer address) {
        this.address = address;
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
     * Set data size.
     * 
     * @param size data size
     */
    public void SetSize(int size) {
        this.size = size;
    }

}
