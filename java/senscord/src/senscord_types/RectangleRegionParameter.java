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
 * Structure for the region of plane for AE or RIO, etc.
 */

public class RectangleRegionParameter extends Structure {
    public int top;
    public int left;
    public int bottom;
    public int right;

    public static class ByReference extends RectangleRegionParameter implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("top", "left", "bottom", "right");
    }

    /**
     * Get top.
     * 
     * @return upper position of region from origin.
     */
    public int GetTop() {
        return top;
    }

    /**
     * Get left.
     * 
     * @return left position of region from origin.
     */
    public int GetLeft() {
        return left;
    }

    /**
     * Get bottom.
     * 
     * @return bottom position of region from origin.
     */
    public int GetBottom() {
        return bottom;
    }

    /**
     * Get right.
     * 
     * @return right position of region from origin.
     */
    public int GetRight() {
        return right;
    }

    /**
     * Set top.
     * 
     * @param top upper position of region from origin.
     */
    public void SetTop(int top) {
        this.top = top;
    }

    /**
     * Set left.
     * 
     * @param left left position of region from origin.
     */
    public void SetLeft(int left) {
        this.left = left;
    }

    /**
     * Set bottom.
     * 
     * @param bottom bottom position of region from origin.
     */
    public void SetBottom(int bottom) {
        this.bottom = bottom;
    }

    /**
     * Set right.
     * 
     * @param right right position of region from origin.
     */
    public void SetRight(int right) {
        this.right = right;
    }
}
