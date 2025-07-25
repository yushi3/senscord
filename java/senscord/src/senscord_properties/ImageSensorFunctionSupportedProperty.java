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
 * Acquiring functions supported by Component.
 * For each function of the image sensor, set the counter corresponding to Component as a boolean
 * value.
 *
 * @see "senscord::ImageSensorFunctionSupportedProperty"
 */
public class ImageSensorFunctionSupportedProperty extends Structure {
    public byte auto_exposure_supported;
    public byte auto_white_balance_supported;
    public byte brightness_supported;
    public byte iso_sensitivity_supported;
    public byte exposure_time_supported;
    public byte exposure_metering_supported;
    public byte gamma_value_supported;
    public byte gain_value_supported;
    public byte hue_supported;
    public byte saturation_supported;
    public byte sharpness_supported;
    public byte white_balance_supported;

    public static class ByReference extends ImageSensorFunctionSupportedProperty
            implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("auto_exposure_supported", "auto_white_balance_supported",
                "brightness_supported", "iso_sensitivity_supported", "exposure_time_supported",
                "exposure_metering_supported", "gamma_value_supported", "gain_value_supported",
                "hue_supported", "saturation_supported", "sharpness_supported",
                "white_balance_supported");
    }

    /**
     * Get auto exposure supported or not.
     * 
     * @return auto exposure supported or not
     */
    public byte GetAutoExposureSupported() {
        return auto_exposure_supported;
    }

    /**
     * Get auto white balance supported or not.
     * 
     * @return auto exposure supported or not
     */
    public byte GetAutoWhiteBalanceSupported() {
        return auto_white_balance_supported;
    }

    /**
     * Get brightness supported or not.
     * 
     * @return brightness supported or not
     */
    public byte GetBrightessSupported() {
        return brightness_supported;
    }

    /**
     * Get ISO sensitivity supported or not.
     * 
     * @return ISO sensitivity supported or not
     */
    public byte GetIsoSensitivitySupported() {
        return iso_sensitivity_supported;
    }


    /**
     * Get exposure time supported or not.
     * 
     * @return exposure time supported or not
     */
    public byte GetExposureTimeSupported() {
        return exposure_time_supported;
    }

    /**
     * Get exposure metering supported or not.
     * 
     * @return exposure metering supported or not
     */
    public byte GetExposureMeteringSupported() {
        return exposure_metering_supported;
    }

    /**
     * Get gamma value supported or not.
     * 
     * @return gamma supported or not
     */
    public byte GetGammaValueSupported() {
        return gamma_value_supported;
    }

    /**
     * Get gain value supported or not.
     * 
     * @return gain value supported or not
     */
    public byte GetGainValueSupported() {
        return gain_value_supported;
    }

    /**
     * Get hue value supported or not.
     * 
     * @return hue value supported or not
     */
    public byte GetHueSupported() {
        return hue_supported;
    }

    /**
     * Get saturation supported or not.
     * 
     * @return saturation supported or not
     */
    public byte GetSaturationSupported() {
        return saturation_supported;
    }

    /**
     * Get sharpness supported or not.
     * 
     * @return sharpness supported or not
     */
    public byte GetSharpnessSupported() {
        return sharpness_supported;
    }

    /**
     * Get white balance supported or not.
     * 
     * @return white balance supported or not
     */
    public byte GetWhiteBalanceSupported() {
        return white_balance_supported;
    }

    /**
     * Set auto exposure supported or not.
     * 
     * @param auto_exposure_supported expose supported or not
     */
    public void SetAutoExposureSupported(byte auto_exposure_supported) {
        this.auto_exposure_supported = auto_exposure_supported;
    }

    /**
     * Set auto white balance supported or not.
     * 
     * @param auto_white_balance_supported auto white balance supported or not
     */
    public void SetAutoWhiteBalanceSupported(byte auto_white_balance_supported) {
        this.auto_white_balance_supported = auto_white_balance_supported;
    }

    /**
     * Set brightness supported or not.
     * 
     * @param brightness_supported brightness supported or not
     */
    public void SetBrightessSupported(byte brightness_supported) {
        this.brightness_supported = brightness_supported;
    }

    /**
     * Set ISO sensitivity supported or not.
     * 
     * @param iso_sensitivity_supported iso sensitivity supported or not
     */
    public void SetIsoSensitivitySupported(byte iso_sensitivity_supported) {
        this.iso_sensitivity_supported = iso_sensitivity_supported;
    }

    /**
     * Set exposure time supported or not.
     * 
     * @param exposure_time_supported exposure time supported or not
     */
    public void SetExposureTimeSupported(byte exposure_time_supported) {
        this.exposure_time_supported = exposure_time_supported;
    }

    /**
     * Set exposure metering supported or not.
     * 
     * @param exposure_metering_supported exposure metering supported or not
     */
    public void SetExposureMeteringSupported(byte exposure_metering_supported) {
        this.exposure_metering_supported = exposure_metering_supported;
    }

    /**
     * Set gamma value supported or not.
     * 
     * @param gamma_value_supported gamma value supported or not
     */
    public void SetGammaValueSupported(byte gamma_value_supported) {
        this.gamma_value_supported = gamma_value_supported;
    }

    /**
     * Set gain value supported or not.
     * 
     * @param gain_value_supported gain value supported or not
     */
    public void SetGainValueSupported(byte gain_value_supported) {
        this.gain_value_supported = gain_value_supported;
    }

    /**
     * Set hue supported or not.
     * 
     * @param hue_supported hue supported or not
     */
    public void SetHueSupported(byte hue_supported) {
        this.hue_supported = hue_supported;
    }

    /**
     * Set saturation supported or not.
     * 
     * @param saturation_supported saturation supported or not
     */
    public void SetSaturationSupported(byte saturation_supported) {
        this.saturation_supported = saturation_supported;
    }

    /**
     * Set sharpness supported or not.
     * 
     * @param sharpness_supported sharpness supported or not
     */
    public void SetSharpnessSupported(byte sharpness_supported) {
        this.sharpness_supported = sharpness_supported;
    }

    /**
     * Set white balance supported or not.
     * 
     * @param white_balance_supported white balance supported or not
     */
    public void SetWhiteBalanceSupported(byte white_balance_supported) {
        this.white_balance_supported = white_balance_supported;
    }
}
