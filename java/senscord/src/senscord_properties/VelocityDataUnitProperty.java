/*
 * SPDX-FileCopyrightText: 2021 Sony Semiconductor Solutions Corporation
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
 * @see "senscord::VelocityDataUnitProperty"
 */
public class VelocityDataUnitProperty extends Structure {
    public int velocity;

    public static class ByReference extends VelocityDataUnitProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("velocity");
    }

    /**
     * Units used for velocity.
     * 
     * @see "senscord::VelocityUnit"
     */
    public static interface VelocityUnit {
        /**
         * Not supported.
         */
        public static final int SENSCORD_VELOCITY_NOT_SUPPORTED = 0;

        /**
         * Unit : [m/s].
         */
        public static final int SENSCORD_METER_PER_SECOND = 1;

        /**
         * Unit : [pixel/s].
         */
        public static final int SENSCORD_PIXEL_PER_SECOND = 2;
    }

    /**
     * Get velocity unit.
     * 
     * @return velocity unit from {@link VelocityUnit}
     */
    public int GetVelocityUnit() {
        return velocity;
    }

    /**
     * Get velocity unit String value.
     * 
     * @param velocity velocity value
     * @return velocity String value
     */
    public String GetVelocityUnitString(int velocity) {
        String ret = "";

        switch (velocity) {
            case VelocityUnit.SENSCORD_VELOCITY_NOT_SUPPORTED:
                ret = "NOT_SUPPORTED";
                break;
            case VelocityUnit.SENSCORD_METER_PER_SECOND:
                ret = "METER_PER_SECOND";
                break;
            case VelocityUnit.SENSCORD_PIXEL_PER_SECOND:
                ret = "PIXEL_PER_SECOND";
                break;
            default:
                break;
        }

        return ret;
    }

    /**
     * Set velocity unit from {@link VelocityUnit}.
     * 
     * @param velocity velocity value
     */
    public void SetVelocityUnit(int velocity) {
        this.velocity = velocity;
    }
}
