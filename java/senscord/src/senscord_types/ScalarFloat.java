/*
 * SPDX-FileCopyrightText: 2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_types;

import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

/**
 * Scalar. (float)
 * 
 * @see "senscord::Scalar"
 */
public class ScalarFloat extends Structure {
    public float value;

    public static class ByReference extends ScalarFloat implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("value");
    }

    /**
     * Get value.
     * 
     * @return value
     */
    public float GetValue() {
        return value;
    }

    /**
     * Set value.
     * 
     * @param value calue
     */
    public void SetValue(float value) {
        this.value = value;
    }
}
