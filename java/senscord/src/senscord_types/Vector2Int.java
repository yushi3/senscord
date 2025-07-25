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
 * Vector2. (int)
 * 
 * @see "senscord::Vector2"
 */
public class Vector2Int extends Structure {
    public int x;
    public int y;

    public static class ByReference extends Vector2Int implements Structure.ByReference {
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
    public int GetX() {
        return x;
    }

    /**
     * Get y axis.
     * 
     * @return y axis value
     */
    public int GetY() {
        return y;
    }

    /**
     * Set x axis.
     * 
     * @param x x axis value
     */
    public void SetX(int x) {
        this.x = x;
    }

    /**
     * Set y axis.
     * 
     * @param y y axis value
     */
    public void SetY(int y) {
        this.y = y;
    }
}
