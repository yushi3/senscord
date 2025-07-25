/*
 * SPDX-FileCopyrightText: 2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_types;

import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

/**
 * Handling internal parameters of calibration.
 * 
 * @see "senscord::IntrinsicCalibrationParameter"
 */
public class IntrinsicCalibrationParameter extends Structure {
    public static class ByReference extends IntrinsicCalibrationParameter
            implements Structure.ByReference {
    }

    public float cx;
    public float cy;
    public float fx;
    public float fy;
    public float s;

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("cx", "cy", "fx", "fy", "s");
    }

    /**
     * Get x axis coordinate of the optical center point.
     * 
     * @return x axis coordinate
     */
    public float GetCoordinateX() {
        return cx;
    }

    /**
     * Get y axis coordinate of the optical center point.
     * 
     * @return y axis coordinate
     */
    public float GetCoordinateY() {
        return cy;
    }

    /**
     * Get focal of length x axis.
     * 
     * @return focal of length x axis
     */
    public float GetFocalX() {
        return fx;
    }

    /**
     * Get focal of length y axis.
     * 
     * @return focal of length y axis
     */
    public float GetFocalY() {
        return fy;
    }

    /**
     * Get skewness.
     * 
     * @return skewness
     */
    public float GetSkew() {
        return s;
    }

    /**
     * Set x axis coordinate of the optical center point.
     * 
     * @param cx x axis coordinate
     */
    public void SetCoordinateX(float cx) {
        this.cx = cx;
    }

    /**
     * Set x axis coordinate of the optical center point.
     * 
     * @param cy y axis coordinate
     */
    public void SetCoordinateY(float cy) {
        this.cy = cy;
    }

    /**
     * Set focal of length x axis.
     * 
     * @param fx focal of length x axis
     */
    public void SetFocalX(float fx) {
        this.fx = fx;
    }

    /**
     * Set focal of length y axis.
     * 
     * @param fy focal of length y axis
     */
    public void SetFocalY(float fy) {
        this.fy = fy;
    }

    /**
     * Set skewness.
     * 
     * @param s skewness
     */
    public void SetSkew(float s) {
        this.s = s;
    }

}
