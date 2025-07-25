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
 * Property for calibration magnetic north.
 * 
 * @see "senscord::MagneticNorthCalibProperty"
 */
public class MagneticNorthCalibProperty extends Structure {
    public float declination;
    public float inclination;

    public static class ByReference extends MagneticNorthCalibProperty
            implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("declination", "inclination");
    }

    /**
     * Get magnetic declination.
     * 
     * @return magnetic declination
     */
    public float GetDeclination() {
        return declination;
    }

    /**
     * Get magnetic inclination.
     * 
     * @return magnetic inclination
     */
    public float GetInclination() {
        return inclination;
    }

    /**
     * Set magnetic declination.
     * 
     * @param declination magnetic declination
     */
    public void SetDeclination(float declination) {
        this.declination = declination;
    }

    /**
     * Set magnetic inclination.
     * 
     * @param inclination magnetic inclination
     */
    public void SetInclination(float inclination) {
        this.inclination = inclination;
    }
}
