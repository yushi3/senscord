/*
 * SPDX-FileCopyrightText: 2020-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Structure;

import java.util.Arrays;
import java.util.List;

import senscord_types.TemperatureInfo;

public class TemperatureProperty extends Structure {
    public int count;
    public TemperatureInfo[] temperature_info;

    public static class ByReference extends TemperatureProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("count", "temperature_info");
    }

    /**
     * Constructor.
     */
    public TemperatureProperty() {
        super();
        final int TEMPERATURE_INFO_NUM = 64;
        temperature_info = new TemperatureInfo[TEMPERATURE_INFO_NUM];
        for (int i = 0; i < TEMPERATURE_INFO_NUM; i++) {
            temperature_info[i] = new TemperatureInfo();
        }
    }

    /**
     * Get count.
     *
     * @return Count
     */
    public int GetCount() {
        return count;
    }

    /**
     * Get temperature info.
     *
     * @return temperature_info
     */
    public TemperatureInfo[] GetTemperatureInfo() {
        return temperature_info;
    }

    /**
     * Set count.
     *
     * @param count Count of the recorder formats
     */
    public void SetCount(int count) {
        this.count = count;
    }
}
