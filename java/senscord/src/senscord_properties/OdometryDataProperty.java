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
 * Position Data Property.
 * 
 * @see "senscord::OdometryDataProperty"
 */
public class OdometryDataProperty extends Structure {
    public int coordinate_system;

    public static class ByReference extends OdometryDataProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("coordinate_system");
    }

    /**
     * Types of coordinate system.
     * 
     * @see "senscord::CoordinateSystem"
     */
    public static interface CoordinateSystem {
        /**
         * World coordinate system.
         */
        public static final int SENSCORD_WORLD_COORDINATWE = 0;

        /**
         * Local coordinate system.
         */
        public static final int SENSCORD_LOCAL_COORDINATWE = 1;

        /**
         * Camera coordinate system.
         */
        public static final int SENSCORD_CAMERA_COORDINATWE = 2;
    }

    /**
     * Get coordinate system value.
     * 
     * @return coordinate system value
     */
    public int GetCoordinateSystem() {
        return coordinate_system;
    }

    /**
     * Get coordinate system String value.
     * 
     * @param value coordinate system value
     * @return coordinate system String value
     */
    public String GetCoordinateSystemString(int value) {
        String ret = "";

        switch (value) {
            case CoordinateSystem.SENSCORD_WORLD_COORDINATWE:
                ret = "WORLD";
                break;
            case CoordinateSystem.SENSCORD_LOCAL_COORDINATWE:
                ret = "LOCAL";
                break;
            case CoordinateSystem.SENSCORD_CAMERA_COORDINATWE:
                ret = "CAMERA";
                break;
            default:
                break;
        }

        return ret;
    }

    /**
     * Set coordinate system value.
     * 
     * @param coordinate_system coordinate system value
     */
    public void SetCoordinateSystem(int coordinate_system) {
        this.coordinate_system = coordinate_system;
    }
}
