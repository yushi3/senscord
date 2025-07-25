/*
 * SPDX-FileCopyrightText: 2019-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Structure;

import java.util.Arrays;
import java.util.List;

import senscord_types.PresetInfo;

/**
 * Property for the list of property's preset IDs.
 * 
 * @see "senscord::PresetListProperty"
 */
public class PresetListProperty extends Structure {
    public int count;
    public PresetInfo[] presets;

    public static class ByReference extends PresetListProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("count", "presets");
    }

    /**
     * Constructor.
     */
    public PresetListProperty() {
        super();
        int PRESET_INFO_SIZE = 64;
        presets = new PresetInfo[PRESET_INFO_SIZE];
        for (int i = 0; i < PRESET_INFO_SIZE; i++) {
            presets[i] = new PresetInfo();
        }
    }

    /**
     * Get preset count.
     * 
     * @return preset count
     */
    public int GetCount() {
        return count;
    }

    /**
     * Get PresetInfo class array.
     * 
     * @return PresetInfo class array
     */
    public PresetInfo[] GetPresetInfo() {
        return presets;
    }

    /**
     * Set preset count.
     * 
     * @param count preset count
     */
    public void SetCount(int count) {
        this.count = count;
    }

}
