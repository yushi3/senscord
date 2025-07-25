/*
 * SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_types;

import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

/**
 * 3x4 matrix.
 *
 * @see "senscord::Matrix3x4"
 */
public class Matrix3x4Float extends Structure {
    public float[] element;

    public static class ByReference extends Matrix3x4Float implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("element");
    }


    public Matrix3x4Float() {
        element = new float[12];
    }

    /**
     * Get matrix element.
     *
     * @return element
     */
    public float[] GetElement() {
        return element;
    }

    /**
     * Set matrix element.
     *
     * @param element matrix element
     */
    public void SetElement(float[] element) {
        this.element = element;
    }
}
