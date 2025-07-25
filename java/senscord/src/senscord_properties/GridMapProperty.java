/*
 * SPDX-FileCopyrightText: 2019-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Native;
import com.sun.jna.Structure;

import java.util.Arrays;
import java.util.List;

/**
 * Grid Data Property.
 * 
 * @see "senscord::GridMapProperty"
 */
public class GridMapProperty extends Structure {
    public int grid_num_x;
    public int grid_num_y;
    public int grid_num_z;
    public byte[] pixel_format;
    public GridSizeProperty grid_size;

    public static class ByReference extends GridMapProperty implements Structure.ByReference {
    }

    public GridMapProperty() {
        super();
        pixel_format = new byte[64];
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("grid_num_x", "grid_num_y", "grid_num_z", "pixel_format", "grid_size");
    }

    /**
     * GridMap pixel format.
     */
    public static interface PixelFormat {
        /**
         * GridMap 1p1n.
         */
        public static final String SENSCORD_PIXEL_FORMAT_GRID_MAP_1P1N = "grid_map_1p1n";
    }

    /**
     * Get grid_num_x of GridMap.
     * 
     * @return grid_num_x of GridMap
     */
    public int GetGridNumX() {
        return grid_num_x;
    }

    /**
     * Get grid_num_y of GridMap.
     * 
     * @return grid_num_y of GridMap
     */
    public int GetGridNumY() {
        return grid_num_y;
    }

    /**
     * Get grid_num_z of GridMap.
     * 
     * @return grid_num_z of GridMap
     */
    public float GetGridNumZ() {
        return grid_num_z;
    }

    /**
     * Get pixel_format of GridMap.
     * 
     * @return pixel_format of GridMap
     */
    public String GetPixelFormat() {
        return Native.toString(pixel_format);
    }

    /**
     * Get grid_size of GridMap.
     * 
     * @return grid_size of GridMap
     */
    public GridSizeProperty GetGridSize() {
        return  grid_size;
    }

    /**
     * Set grid_num_x of GridMap.
     * 
     * @param grid_num_x grid_num_x of GridMap
     */
    public void SetGridNumX(int grid_num_x) {
        this.grid_num_x = grid_num_x;
    }

    /**
     * Set grid_num_y of GridMap.
     * 
     * @param grid_num_y grid_num_x of GridMap
     */
    public void SetGridNumY(int grid_num_y) {
        this.grid_num_y = grid_num_y;
    }

    /**
     * Set grid_num_z of GridMap.
     * 
     * @param grid_num_z grid_num_x of GridMap
     */
    public void SetGridNumZ(int grid_num_z) {
        this.grid_num_z = grid_num_z;
    }

    /**
     * Set pixel_format of GridMap.
     * 
     * @param pixel_format image pixel format of GridMap. Maximum length is 64. {@link PixelFormat}
     */
    public void SetPixelFormat(String pixel_format) {
        Arrays.fill(this.pixel_format, (byte) 0);
        System.arraycopy(pixel_format.getBytes(), 0, this.pixel_format, 0, pixel_format.length());
    }

    /**
     * Get grid_size of GridMap.
     * 
     * @param grid_size grid size of GridMap
     */
    public void SetGridSize(GridSizeProperty grid_size) {
        this.grid_size = grid_size;
    }
}
