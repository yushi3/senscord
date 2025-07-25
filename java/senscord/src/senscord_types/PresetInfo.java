/*
 * SPDX-FileCopyrightText: 2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_types;

import com.sun.jna.Native;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

/**
 * Structure that represents the preset information.
 */
public class PresetInfo extends Structure {
    public static class ByReference extends PresetInfo implements Structure.ByReference {
    }

    public int id;
    public byte[] description;

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("id", "description");
    }

    public PresetInfo() {
        super();
        description = new byte[128];
    }

    /**
     * Get preset ID.
     * 
     * @return preset ID
     */
    public int GetId() {
        return id;
    }

    /**
     * Get preset description.
     * 
     * @return preset description
     */
    public String GetDescription() {
        return Native.toString(description);
    }

    /**
     * Set preset ID.
     * 
     * @param id preset ID.
     */
    public void SetId(int id) {
        this.id = id;
    }

    /**
     * Set description.
     * 
     * @param description preset description Maximum length is 128
     */
    public void SetDescription(String description) {
        Arrays.fill(this.description, (byte) 0);
        System.arraycopy(description.getBytes(), 0, this.description, 0, description.length());
    }
}
