/*
 * SPDX-FileCopyrightText: 2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

/**
 * PProperty showing the information of coordinate system.
 * 
 * @see "senscord::CoordinateSystemProperty"
 */
public class CoordinateSystemProperty extends Structure {
    public int handed;
    public int up_axis;
    public int forward_axis;

    public static class ByReference extends CoordinateSystemProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("handed", "up_axis", "forward_axis");
    }

    /**
     * System handed for CoordinateSystem.
     * 
     * @see "senscord::SystemHanded"
     */
    public static interface SystemHanded {
        /**
         * Left-handed system.
         */
        public static final int SENSCORD_SYSTEM_HANDED_LEFT = 0;

        /**
         * Right-handed system.
         */
        public static final int SENSCORD_SYSTEM_HANDED_RIGHT = 1;
    }

    /**
     * Up axis for CoordinateSystem.
     * 
     * @see "senscord::UpAxis"
     */
    public static interface UpAxis {
        /**
         * UpAxis undefined.
         */
        public static final int SENSCORD_UP_AXIS_UNDEFINED = 0;

        /**
         * X axis up.
         */
        public static final int SENSCORD_UP_AXIS_PLUS_X = 1;

        /**
         * Y axis up.
         */
        public static final int SENSCORD_UP_AXIS_PLUS_Y = 2;

        /**
         * Z axis up.
         */
        public static final int SENSCORD_UP_AXIS_PLUS_Z = 3;

        /**
         * X axis down.
         */
        public static final int SENSCORD_UP_AXIS_MINUS_X = 4;

        /**
         * Y axis down.
         */
        public static final int SENSCORD_UP_AXIS_MINUS_Y = 5;

        /**
         * Z axis down.
         */
        public static final int SENSCORD_UP_AXIS_MINUS_Z = 6;
    }

    /**
     * Forward axis for CoordinateSystem.
     * 
     * @see "senscord::ForwardAxis"
     */
    public static interface ForwardAxis {
        /**
         * ForwardAxis undefined.
         */
        public static final int SENSCORD_FORWARD_AXIS_UNDEFINED = 0;

        /**
         * X axis up.
         */
        public static final int SENSCORD_FORWARD_AXIS_PLUS_X = 1;

        /**
         * Y axis up.
         */
        public static final int SENSCORD_FORWARD_AXIS_PLUS_Y = 2;

        /**
         * Z axis up.
         */
        public static final int SENSCORD_FORWARD_AXIS_PLUS_Z = 3;

        /**
         * X axis down.
         */
        public static final int SENSCORD_FORWARD_AXIS_MINUS_X = 4;

        /**
         * Y axis down.
         */
        public static final int SENSCORD_FORWARD_AXIS_MINUS_Y = 5;

        /**
         * Z axis down.
         */
        public static final int SENSCORD_FORWARD_AXIS_MINUS_Z = 6;
    }


    /**
     * Get System handed.
     * 
     * @return handed from {@link SystemHanded}
     */
    public int GetSystemHanded() {
        return handed;
    }

    /**
     * Get up axis.
     * 
     * @return up axis from {@link UpAxis}
     */
    public int GetUpAxis() {
        return up_axis;
    }

    /**
     * Get forward axis.
     * 
     * @return forward axis from {@link ForwardAxis}
     */
    public int GetForwardAxis() {
        return forward_axis;
    }

    /**
     * Get system handed String value.
     * 
     * @param handed system handed value
     * @return system handed String value
     */
    public String GetSystemHandedString(int handed) {
        String ret = "";

        switch (handed) {
            case SystemHanded.SENSCORD_SYSTEM_HANDED_LEFT:
                ret = "SYSTEM_HANDED_LEFT";
                break;
            case SystemHanded.SENSCORD_SYSTEM_HANDED_RIGHT:
                ret = "SYSTEM_HANDED_RIGHT";
                break;
            default:
                break;
        }

        return ret;
    }

    /**
     * Get up axis String value.
     * 
     * @param up_axis up axis value
     * @return up_axis String value
     */
    public String GetUpAxisString(int up_axis) {
        String ret = "";

        switch (up_axis) {
            case UpAxis.SENSCORD_UP_AXIS_UNDEFINED:
                ret = "UP_AXIS_UNDEFINED";
                break;
            case UpAxis.SENSCORD_UP_AXIS_PLUS_X:
                ret = "UP_AXIS_PLUS_X";
                break;
            case UpAxis.SENSCORD_UP_AXIS_PLUS_Y:
                ret = "UP_AXIS_PLUS_Y";
                break;
            case UpAxis.SENSCORD_UP_AXIS_PLUS_Z:
                ret = "UP_AXIS_PLUS_Z";
                break;
            case UpAxis.SENSCORD_UP_AXIS_MINUS_X:
                ret = "UP_AXIS_MINUS_X";
                break;
            case UpAxis.SENSCORD_UP_AXIS_MINUS_Y:
                ret = "UP_AXIS_MINUS_Y";
                break;
            case UpAxis.SENSCORD_UP_AXIS_MINUS_Z:
                ret = "UP_AXIS_MINUS_Z";
                break;
            default:
                break;
        }

        return ret;
    }

    /**
     * Get forward axis String value.
     * 
     * @param forward_axis up axis value
     * @return forward_axis String value
     */
    public String GetForwardAxisString(int forward_axis) {
        String ret = "";

        switch (forward_axis) {
            case ForwardAxis.SENSCORD_FORWARD_AXIS_UNDEFINED:
                ret = "FORWARD_AXIS_UNDEFINED";
                break;
            case ForwardAxis.SENSCORD_FORWARD_AXIS_PLUS_X:
                ret = "FORWARD_AXIS_PLUS_X";
                break;
            case ForwardAxis.SENSCORD_FORWARD_AXIS_PLUS_Y:
                ret = "FORWARD_AXIS_PLUS_Y";
                break;
            case ForwardAxis.SENSCORD_FORWARD_AXIS_PLUS_Z:
                ret = "FORWARD_AXIS_PLUS_Z";
                break;
            case ForwardAxis.SENSCORD_FORWARD_AXIS_MINUS_X:
                ret = "FORWARD_AXIS_MINUS_X";
                break;
            case ForwardAxis.SENSCORD_FORWARD_AXIS_MINUS_Y:
                ret = "FORWARD_AXIS_MINUS_Y";
                break;
            case ForwardAxis.SENSCORD_FORWARD_AXIS_MINUS_Z:
                ret = "FORWARD_AXIS_MINUS_Z";
                break;
            default:
                break;
        }

        return ret;
    }

    /**
     * Set system handed from {@link SystemHanded}.
     * 
     * @param handed system handed value
     */
    public void SetSystemHanded(int handed) {
        this.handed = handed;
    }

    /**
     * Set up axis from {@link UpAxis}.
     * 
     * @param up_axis up axis value
     */
    public void SetUpAxis(int up_axis) {
        this.up_axis = up_axis;
    }

    /**
     * Set forward axis from {@link ForwardAxis}.
     * 
     * @param forward_axis up axis value
     */
    public void SetForwardAxis(int forward_axis) {
        this.forward_axis = forward_axis;
    }
}
