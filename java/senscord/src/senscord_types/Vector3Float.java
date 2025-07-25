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
 * Vector3. (float)
 * 
 * @see "senscord::Vector3"
 */
public class Vector3Float extends Structure {
    public float x;
    public float y;
    public float z;

    public static class ByReference extends Vector3Float implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("x", "y", "z");
    }

    /**
     * Get x axis.
     * 
     * @return x axis value
     */
    public float GetX() {
        return x;
    }

    /**
     * Get y axis.
     * 
     * @return y axis value
     */
    public float GetY() {
        return y;
    }

    /**
     * Get z axis.
     * 
     * @return z axis value
     */
    public float GetZ() {
        return z;
    }

    /**
     * Set x axis.
     * 
     * @param x x axis value
     */
    public void SetX(float x) {
        this.x = x;
    }

    /**
     * Set y axis.
     * 
     * @param y y axis value
     */
    public void SetY(float y) {
        this.y = y;
    }

    /**
     * Set z axis.
     * 
     * @param z z axis value
     */
    public void SetZ(float z) {
        this.z = z;
    }
}
