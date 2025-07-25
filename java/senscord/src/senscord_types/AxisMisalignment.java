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
 * Misalignment of the axis direction.
 * 
 * @see "senscord::AxisMisalignment"
 */
public class AxisMisalignment extends Structure {
    public Matrix3x3Float ms;
    public Vector3Float offset;

    public static class ByReference extends AxisMisalignment implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("ms", "offset");
    }

    /**
     * Get 3x3 Float Matrix.
     * 
     * @return {@link Matrix3x3Float} class
     */
    public Matrix3x3Float GetValueMatrix() {
        return ms;
    }

    /**
     * Get Vector3Float.
     * 
     * @return {@link Vector3Float} class
     */
    public Vector3Float GetOffset() {
        return offset;
    }
}
