/*
 * SPDX-FileCopyrightText: 2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Native;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

/**
 * Set the functions used in the sensor.
 * 
 * @see "senscord::ImageSensorFunctionProperty"
 */
public class ImageSensorFunctionProperty extends Structure {
    public byte auto_exposure;
    public byte auto_white_balance;
    public int brightness;
    public int iso_sensitivity;
    public int exposure_time;
    public byte[] exposure_metering;
    public float gamma_value;
    public int gain_value;
    public int hue;
    public int saturation;
    public int sharpness;
    public int white_balance;

    public static class ByReference extends ImageSensorFunctionProperty
            implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("auto_exposure", "auto_white_balance", "brightness", "iso_sensitivity",
                "exposure_time", "exposure_metering", "gamma_value", "gain_value", "hue",
                "saturation", "sharpness", "white_balance");
    }

    /**
     * Exposure metering modes.
     */
    public static interface Metering {
        /**
         * None(Disabled).
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

    public ImageSensorFunctionProperty() {
        super();
        exposure_metering = new byte[64];
    }

    /**
     * Get auto exposure value.
     * 
     * @return auto exposure value
     */
    public byte GetAutoExposure() {
        return auto_exposure;
    }

    /**
     * Get auto white balance value.
     * 
     * @return white balance value
     */
    public byte GetAutoWhiteBalance() {
        return auto_white_balance;
    }

    /**
     * Get Brightness value.
     * 
     * @return brightness value
     */
    public int GetBrightess() {
        return brightness;
    }

    /**
     * Get ISO sensitivity value.
     * 
     * @return ISO sensitivity value.
     */
    public int GetIsoSensitivity() {
        return iso_sensitivity;
    }

    /**
     * Get Exposer time.
     * 
     * @return exposure time
     */
    public int GetExposureTime() {
        return exposure_time;
    }

    /**
     * Get exposure metering.
     * 
     * @return exposure metering
     */
    public String GetExposureMetering() {
        return Native.toString(exposure_metering);
    }

    /**
     * Get gamma value.
     * 
     * @return gamma value
     */
    public float GetGammaValue() {
        return gamma_value;
    }

    /**
     * Get gain value.
     * 
     * @return gain value
     */
    public int GetGainValue() {
        return gain_value;
    }

    /**
     * Get hue value.
     * 
     * @return hue value
     */
    public int GetHue() {
        return hue;
    }

    /**
     * Get saturation value.
     * 
     * @return saturation value
     */
    public int GetSaturation() {
        return saturation;
    }

    /**
     * Get sharpness value.
     * 
     * @return sharpness value
     */
    public int GetSharpness() {
        return sharpness;
    }

    /**
     * Get white balance.
     * 
     * @return white balance
     */
    public int GetWhiteBalance() {
        return white_balance;
    }

    /**
     * Set auto exposure value.
     * 
     * @param auto_exposure aute exposure value
     */
    public void SetAutoExposure(byte auto_exposure) {
        this.auto_exposure = auto_exposure;
    }

    /**
     * Set auto white balance value.
     * 
     * @param auto_white_balance white balance value
     */
    public void SetAutoWhiteBalance(byte auto_white_balance) {
        this.auto_white_balance = auto_white_balance;
    }

    /**
     * Set brightness value.
     * 
     * @param brightness brightness value
     */
    public void SetBrightess(int brightness) {
        this.brightness = brightness;
    }

    /**
     * Set ISO sensitivity value.
     * 
     * @param iso_sensitivity ISO sensitivity value.
     */
    public void SetIsoSensitivity(int iso_sensitivity) {
        this.iso_sensitivity = iso_sensitivity;
    }

    /**
     * Set exposure time.
     * 
     * @param exposure_time exposure time
     */
    public void SetExposureTime(int exposure_time) {
        this.exposure_time = exposure_time;
    }

    /**
     * Set exposure metering value.
     * 
     * @param exposure_metering exposure metering value
     */
    public void SetExposureMetering(String exposure_metering) {
        Arrays.fill(this.exposure_metering, (byte) 0);
        System.arraycopy(exposure_metering.getBytes(), 0, this.exposure_metering, 0,
                exposure_metering.length());
    }

    /**
     * Set gamma value.
     * 
     * @param gamma_value gamma value
     */
    public void SetGammaValue(float gamma_value) {
        this.gamma_value = gamma_value;
    }

    /**
     * Set gain value.
     * 
     * @param gain_value gain value.
     */
    public void SetGainValue(int gain_value) {
        this.gain_value = gain_value;
    }

    /**
     * Set hue value.
     * 
     * @param hue hue value.
     */
    public void SetHue(int hue) {
        this.hue = hue;
    }

    /**
     * Set saturation value.
     * 
     * @param saturation saturation value
     */
    public void SetSaturation(int saturation) {
        this.saturation = saturation;
    }

    /**
     * Set sharpness value.
     * 
     * @param sharpness sharpness value
     */
    public void SetSharpness(int sharpness) {
        this.sharpness = sharpness;
    }

    /**
     * Set white balance value.
     * 
     * @param white_balance white balance value
     */
    public void SetWhiteBalance(int white_balance) {
        this.white_balance = white_balance;
    }
}
