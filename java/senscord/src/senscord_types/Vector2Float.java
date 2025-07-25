/*
 * SPDX-FileCopyrightText: 2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_types;

import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

/**
 * Vector2. (float)
 * 
 * @see "senscord::Vector2"
 */
public class Vector2Float extends Structure {
    public float x;
    public float y;

    public static class ByReference extends Vector2Float implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("x", "y");
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
}
