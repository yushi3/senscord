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
 * Property for the property's preset.
 * 
 * @see "senscord::PresetProperty"
 */
public class PresetProperty extends Structure {
    public int id;

    public static class ByReference extends PresetProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("id");
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
     * Set preset ID.
     * 
     * @param id preset ID
     */
    public void SetId(int id) {
        this.id = id;
    }

}
