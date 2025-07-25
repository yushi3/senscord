/*
 * SPDX-FileCopyrightText: 2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

/**
 * Grid Size Property.
 * 
 * @see "senscord::GridSize"
 */
public class GridSizeProperty extends Structure {
    public float x;
    public float y;
    public float z;
    public int unit;

    public static class ByReference extends GridSizeProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("x", "y", "z", "unit");
    }

    /**
     * Units of grid.
     * 
     * @see "senscord::GridUnit"
     */
    public static interface GridUnit {
        /**
         * pixel.
         */
        public static final int Pixel = 0;

        /**
         * meter.
         */
        public static final int Meter = 1;
    }

    /**
     * Get X of GridSize.
     * 
     * @return x of GridSize
     */
    public float GetX() {
        return x;
    }

    /**
     * Get Y of GridSize.
     * 
     * @return y of GridSize
     */
    public float GetY() {
        return y;
    }

    /**
     * Get Z of GridSize.
     * 
     * @return z of GridSize
     */
    public float GetZ() {
        return z;
    }

    /**
     * Get unit of GridSize.
     * 
     * @return unit of GridSize
     */
    public int GetUnit() {
        return unit;
    }

    /**
     * Get GridUnit String value.
     * 
     * @param value GridUnit value
     * @return GridUnit String value
     */
    public String GetUnitString(int value) {
        String ret = "";

        switch (value) {
            case GridUnit.Pixel:
                ret = "Pixel";
                break;
            case GridUnit.Meter:
                ret = "Meter";
                break;
            default:
                break;
        }

        return ret;
    }

    /**
     * Set x of GridSize.
     * 
     * @param x x of GridSize
     */
    public void SetX(float x) {
        this.x = x;
    }

    /**
     * Set y of GridSize.
     * 
     * @param y y of GridSize
     */
    public void SetY(float y) {
        this.y = y;
    }

    /**
     * Set z of GridSize.
     * 
     * @param z z of GridSize.
     */
    public void SetZ(float z) {
        this.z = z;
    }

    /**
     * Set GridUnit from {@link GridUnit} .
     * 
     * @param unit unit of GridSize
     */
    public void SetUnit(int unit) {
        this.unit = unit;
    }
}
