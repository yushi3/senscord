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
 * 3x3 matrix.
 * 
 * @see "senscord::Matrix"
 */
public class Matrix3x3Float extends Structure {
    public float[] element;

    public static class ByReference extends Matrix3x3Float implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("element");
    }


    public Matrix3x3Float() {
        element = new float[9];
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
