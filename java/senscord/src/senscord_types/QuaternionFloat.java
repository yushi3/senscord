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
 * Quaternion.
 * 
 * @see "senscord::Quaternion"
 */
public class QuaternionFloat extends Structure {
    public float x;
    public float y;
    public float z;
    public float w;

    public static class ByReference extends QuaternionFloat implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("x", "y", "z", "w");
    }

    /**
     * Get x element.
     * 
     * @return x element value
     */
    public float GetX() {
        return x;
    }

    /**
     * Get y element.
     * 
     * @return y element value
     */
    public float GetY() {
        return y;
    }

    /**
     * Get z element.
     * 
     * @return z element value
     */
    public float GetZ() {
        return z;
    }

    /**
     * Get w element.
     * 
     * @return w element value
     */
    public float GetW() {
        return w;
    }

    /**
     * Set x element.
     * 
     * @param x x element value
     */
    public void SetX(float x) {
        this.x = x;
    }

    /**
     * Set y element.
     * 
     * @param y y element value
     */
    public void SetY(float y) {
        this.y = y;
    }

    /**
     * Set z element.
     * 
     * @param z z element value
     */
    public void SetZ(float z) {
        this.z = z;
    }

    /**
     * Set w element.
     * 
     * @param w w element value
     */
    public void SetW(float w) {
        this.w = w;
    }
}
