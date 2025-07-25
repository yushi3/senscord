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

import senscord_types.RectangleRegionParameter;

/**
 * Structure for the image of the camera exposure.
 * 
 * @see "senscord::ExposureProperty"
 */
public class ExposureProperty extends Structure {
    public byte[] mode;
    public float ev_compensation;
    public int exposure_time;
    public int iso_sensitivity;
    public byte[] metering;
    public RectangleRegionParameter target_region;

    public static class ByReference extends ExposureProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("mode", "ev_compensation", "exposure_time", "iso_sensitivity",
                "metering", "target_region");
    }

    /**
     * Constructor.
     */
    public ExposureProperty() {
        super();
        mode = new byte[64];
        metering = new byte[64];
    }

    public static interface Mode {
        /**
         * Auto.
         */
        public static final String SENSCORD_EXPOSURE_MODE_AUTO = "auto";

        /**
         * Hold.
         */
        public static final String SENSCORD_EXPOSURE_MODE_HOLD = "hold";

        /**
         * Manual.
         */
        public static final String SENSCORD_EXPOSURE_MODE_MANUAL = "manual";

        /**
         * Gain fix.
         */
        public static final String SENSCORD_EXPOSURE_MODE_GAIN_FIX = "gainfix";

        /**
         * Time fix.
         */
        public static final String SENSCORD_EXPOSURE_MODE_TIME_FIX = "timefix";
    }

    public static interface Metering {
        /**
         * Mode.
         */
        public static final String SENSOCRD_EXPOSURE_METERING_NONE = "note";

        /**
         * Average.
         */
        public static final String SENSOCRD_EXPOSURE_METERING_AVERAGE = "average";

        /**
         * Center weighted.
         */
        public static final String SENSOCRD_EXPOSURE_METERING_CENTER_WEIGHTED = "center_weighted";

        /**
         * Spot.
         */
        public static final String SENSOCRD_EXPOSURE_METERING_SPOT = "spot";

        /**
         * Matrix.
         */
        public static final String SENSOCRD_EXPOSURE_METERING_MATRIX = "matrix";
    }

    /**
     * Get exposure mode from {@link Mode}.
     * 
     * @return exposure mode
     */
    public String GetMode() {
        return Native.toString(mode);
    }


    /**
     * Get EV compensation value.
     * 
     * @return EV compensation value.
     */
    public float GetEvCompensation() {
        return ev_compensation;
    }

    /**
     * Get exposure time.
     * 
     * @return exposure time
     */
    public int GetExposureTime() {
        return exposure_time;
    }

    /**
     * Get ISO sensitivity value.
     * 
     * @return ISO sensitivity value
     */
    public int GetIsoSensitivity() {
        return iso_sensitivity;
    }

    /**
     * Get exposure metering from {@link Metering}.
     * 
     * @return exposure metering metering
     */
    public String GetMetering() {
        return Native.toString(metering);
    }

    /**
     * Get rectangle region {@link RectangleRegionParameter}.
     * 
     * @return rectangle region
     */
    public RectangleRegionParameter GetTargetRegion() {
        return target_region;
    }

    /**
     * Set e exposure mode from {@link Mode}.
     * 
     * @param mode exposure mode
     */
    public void SetMode(String mode) {
        Arrays.fill(this.mode, (byte) 0);
        System.arraycopy(mode.getBytes(), 0, this.mode, 0, mode.length());
    }

    /**
     * Set EV compensation value.
     * 
     * @param ev_compensation EV compensation value
     */
    public void SetEvCompensation(float ev_compensation) {
        this.ev_compensation = ev_compensation;
    }

    /**
     * Set exposure time .
     * 
     * @param exposure_time exposure time
     */
    public void SetExposureTime(int exposure_time) {
        this.exposure_time = exposure_time;
    }

    /**
     * Set IOS sensitivity.
     * 
     * @param iso_sensitivity iso sensitivity
     */
    public void SetIsoSensitivity(int iso_sensitivity) {
        this.iso_sensitivity = iso_sensitivity;
    }

    /**
     * Set metering {@link Metering}.
     * 
     * @param metering exposure metering
     */
    public void SetMetering(String metering) {
        Arrays.fill(this.metering, (byte) 0);
        System.arraycopy(metering.getBytes(), 0, this.metering, 0, metering.length());
    }

    /**
     * Set rectangle region {@link RectangleRegionParameter}.
     * 
     * @param target_region rectangle region
     */
    public void SetTargetRegion(RectangleRegionParameter target_region) {
        this.target_region = target_region;
    }
}
