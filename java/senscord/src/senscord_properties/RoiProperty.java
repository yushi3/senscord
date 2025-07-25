/*
 * SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

/**
 * Property for ROI.
 * 
 * @see "senscord::RoiProperty"
 */
public class RoiProperty extends Structure {
    public int left;
    public int top;
    public int width;
    public int height;

    public static class ByReference extends RoiProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("left", "top", "width", "height");
    }

    /**
     * Get left.
     *
     * @return left
     */
    public int GetLeft() {
        return left;
    }

    /**
     * Get top.
     *
     * @return top
     */
    public int GetTop() {
        return top;
    }

    /**
     * Get width.
     *
     * @return width
     */
    public int GetWidth() {
        return width;
    }

    /**
     * Get Height.
     *
     * @return Height
     */
    public int GetHeight() {
        return height;
    }

    /**
     * Set left.
     *
     * @param left Left
     */
    public void SetLeft(int left) {
        this.left = left;
    }

    /**
     * Set top.
     *
     * @param top Top
     */
    public void SetTop(int top) {
        this.top = top;
    }

    /**
     * Set width.
     *
     * @param width Width
     */
    public void SetWidth(int width) {
        this.width = width;
    }

    /**
     * Set Height.
     *
     * @param height Height
     */
    public void SetHeight(int height) {
        this.height = height;
    }
}
