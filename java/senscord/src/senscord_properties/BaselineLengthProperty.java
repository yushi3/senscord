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
 * Structure for handling baseline length between cameras.
 * 
 * @see "senscord::BaselineLengthProperty"
 */
public class BaselineLengthProperty extends Structure {
    public float length_mm;

    public static class ByReference extends BaselineLengthProperty
            implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("length_mm");
    }

    /**
     * Get baseline length. Unit is millimeters.
     * 
     * @return baseline length
     */
    public float GetLength() {
        return length_mm;
    }

    /**
     * Set baseline length. Unit is millimeters.
     * 
     * @param length_mm baseline length
     */
    public void SetLength(float length_mm) {
        this.length_mm = length_mm;
    }
}
