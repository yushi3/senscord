/*
 * SPDX-FileCopyrightText: 2021-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Structure;

import java.util.Arrays;
import java.util.List;

import senscord_types.DataRateElement;

/**
 * Handling Data rate properties.
 *
 * @see "senscord::DataRateProperty"
 */
public class DataRateProperty extends Structure {
    public int count;
    public DataRateElement[] elements;

    public static class ByReference extends DataRateProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("count", "elements");
    }

    /**
     * Get data rate element count.
     *
     * @return count
     */
    public int GetCount() {
        return count;
    }

    /**
     * Get data rate elements.
     *
     * @return elements
     */
    public DataRateElement[] GetElements() {
        return elements;
    }

    /**
     * Set data rate element count.
     *
     * @param count element count
     */
    public void SetCount(int count) {
        this.count = count;
    }

    /**
     * Set data rate elements.
     *
     * @param elements data rate elements
     */
    public void SetElements(DataRateElement[] elements) {
        this.elements = elements;
    }
}
