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
 * Property for obtaining unit of RawData.
 * 
 * @see "senscord::ImuDataUnitProperty"
 */
public class ImuDataUnitProperty extends Structure {
    public int acceleration;
    public int angular_velocity;
    public int magnetic_field;
    public int orientation;

    public static class ByReference extends ImuDataUnitProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("acceleration", "angular_velocity", "magnetic_field", "orientation");
    }

    /**
     * Units used for acceleration.
     * 
     * @see "senscord::AccelerationUnit"
     */
    public static interface AccelerationUnit {
        /**
         * Not supported.
         */
        public static final int SENSCORD_ACCELERATION_NOT_SUPPORTED = 0;

        /**
         * Unit : [G].
         */
        public static final int SENSCORD_GRAVITATIONAL = 1;

        /**
         * Unit : [m/s^2].
         */
        public static final int SENSCORD_METER_PER_SECOND_SQUARED = 2;
    }

    /**
     * Units used for angular velocity.
     * 
     * @see "senscord::AngularVelocityUnit"
     */
    public static interface AngularVelocityUnit {
        /**
         * Not supported.
         */
        public static final int SENSCORD_ANGULAR_VELOCITY_NOT_SUPPORTED = 0;

        /**
         * Unit : [deg/s].
         */
        public static final int SENSCORD_DEGREE_PER_SECOND = 1;

        /**
         * Unit : [rad/s].
         */
        public static final int SENSCORD_RADIAN_PER_SECOND = 2;
    }

    public static interface MagneticFieldUnit {
        /**
         * Not supported.
         */
        public static final int SENSCORD_MAGNETIC_FIELD_NOT_SUPPORTED = 0;

        /**
         * Unit : [gauss].
         */
        public static final int SENSCORD_GAUSS = 1;

        /**
         * Unit : [uT].
         */
        public static final int SENSCORD_MICRO_TESLA = 2;
    }

    public static interface OrientationUnit {
        /**
         * Not supported.
         */
        public static final int SENSCORD_ORIENTATION_NOT_SUPPORTED = 0;

        /**
         * Unit : [deg].
         */
        public static final int SENSCORD_DEGREE = 1;

        /**
         * Unit : [rad].
         */
        public static final int SENSCORD_RADIAN = 2;
    }


    /**
     * Get acceleration unit.
     * 
     * @return acceleration unit from {@link AccelerationUnit}
     */
    public int GetAccelerationUnit() {
        return acceleration;
    }

    /**
     * Get angular velocity unit.
     * 
     * @return angular velocity unit from {@link AngularVelocityUnit}
     */
    public int GetAngularVelocityUnit() {
        return angular_velocity;
    }

    /**
     * Get magnetic field unit.
     * 
     * @return magnetic field unit from {@link MagneticFieldUnit}
     */
    public int GetMagneticFieldUnit() {
        return magnetic_field;
    }

    /**
     * Get orientation unit.
     * 
     * @return orientation unit from {@link OrientationUnit}
     */
    public int GetOrientationUnit() {
        return orientation;
    }

    /**
     * Get acceleration unit String value.
     * 
     * @param acceleration acceleration value
     * @return acceleration String value
     */
    public String GetAccelerationUnitString(int acceleration) {
        String ret = "";

        switch (acceleration) {
            case AccelerationUnit.SENSCORD_ACCELERATION_NOT_SUPPORTED:
                ret = "NOT_SUPPORTED";
                break;
            case AccelerationUnit.SENSCORD_GRAVITATIONAL:
                ret = "GRAVITATIONAL";
                break;
            case AccelerationUnit.SENSCORD_METER_PER_SECOND_SQUARED:
                ret = "METER_PER_SECOND";
                break;
            default:
                break;
        }

        return ret;
    }

    /**
     * Get angular velocity unit String value.
     * 
     * @param angular_velocity angular velocity value
     * @return angular velocity String value
     */
    public String GetAngularVelocityUnitString(int angular_velocity) {
        String ret = "";

        switch (angular_velocity) {
            case AngularVelocityUnit.SENSCORD_ANGULAR_VELOCITY_NOT_SUPPORTED:
                ret = "NOT_SUPPORTED";
                break;
            case AngularVelocityUnit.SENSCORD_DEGREE_PER_SECOND:
                ret = "DEGREE_PER_SECOND";
                break;
            case AngularVelocityUnit.SENSCORD_RADIAN_PER_SECOND:
                ret = "RADIAN_PER_SECOND";
                break;
            default:
                break;
        }

        return ret;
    }

    /**
     * Get magnetic field unit String value.
     * 
     * @param magnetic_field magnetic field value
     * @return magnetic filed String value
     */
    public String GetMagneticFieldUnitString(int magnetic_field) {
        String ret = "";

        switch (magnetic_field) {
            case MagneticFieldUnit.SENSCORD_MAGNETIC_FIELD_NOT_SUPPORTED:
                ret = "NOT_SUPPORTED";
                break;
            case MagneticFieldUnit.SENSCORD_GAUSS:
                ret = "GAUSS";
                break;
            case MagneticFieldUnit.SENSCORD_MICRO_TESLA:
                ret = "TESLA";
                break;
            default:
                break;
        }

        return ret;
    }

    /**
     * Get orientation unit String value.
     * 
     * @param orientation orientation field value.
     * @return orientation String value
     */
    public String GetOrientationUnitString(int orientation) {
        String ret = "";

        switch (orientation) {
            case OrientationUnit.SENSCORD_ORIENTATION_NOT_SUPPORTED:
                ret = "NOT_SUPPORTED";
                break;
            case OrientationUnit.SENSCORD_DEGREE:
                ret = "DEGREE";
                break;
            case OrientationUnit.SENSCORD_RADIAN:
                ret = "RADIAN";
                break;
            default:
                break;
        }

        return ret;
    }

    /**
     * Set acceleration unit from {@link AccelerationUnit}.
     * 
     * @param acceleration acceleration value
     */
    public void SetAccelerationUnit(int acceleration) {
        this.acceleration = acceleration;
    }

    /**
     * Set angular velocity unit from {@link AngularVelocityUnit}.
     * 
     * @param angular_velocity angular velocity value
     */
    public void SetAngularVelocityUnit(int angular_velocity) {
        this.angular_velocity = angular_velocity;
    }

    /**
     * Set magnetic field unit from {@link MagneticFieldUnit}.
     * 
     * @param magnetic_field magnetic field value
     */
    public void SetMagneticFieldUnit(int magnetic_field) {
        this.magnetic_field = magnetic_field;
    }

    /**
     * Set orientation unit from {@link OrientationUnit}.
     * 
     * @param orientation orientation value
     */
    public void SetOrientationUnit(int orientation) {
        this.orientation = orientation;
    }
}
