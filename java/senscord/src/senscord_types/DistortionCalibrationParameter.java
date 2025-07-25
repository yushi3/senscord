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
 * Handling camera distortion coefficient.
 * 
 * @see "senscord::DistortionCalibrationParameter"
 */
public class DistortionCalibrationParameter extends Structure {
    public static class ByReference extends DistortionCalibrationParameter
            implements Structure.ByReference {
    }

    public float k1;
    public float k2;
    public float k3;
    public float k4;
    public float k5;
    public float k6;
    public float p1;
    public float p2;

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("k1", "k2", "k3", "k4", "k5", "k6", "p1", "p2");
    }

    /**
     * Get camera distortion coefficient k1.
     * 
     * @return camera distortion coefficient k1
     */
    public float GetK1() {
        return k1;
    }

    /**
     * Get camera distortion coefficient k2.
     * 
     * @return camera distortion coefficient k2
     */
    public float GetK2() {
        return k2;
    }

    /**
     * Get camera distortion coefficient k3.
     * 
     * @return camera distortion coefficient k3
     */
    public float GetK3() {
        return k3;
    }

    /**
     * Get camera distortion coefficient k4.
     * 
     * @return camera distortion coefficient k4
     */
    public float GetK4() {
        return k4;
    }

    /**
     * Get camera distortion coefficient k5.
     * 
     * @return camera distortion coefficient k5
     */
    public float GetK5() {
        return k5;
    }

    /**
     * Get camera distortion coefficient k6.
     * 
     * @return camera distortion coefficient k6
     */
    public float GetK6() {
        return k6;
    }

    /**
     * Get camera distortion coefficient p1.
     * 
     * @return camera distortion coefficient p1
     */
    public float GetP1() {
        return p1;
    }

    /**
     * Get camera distortion coefficient p2.
     * 
     * @return camera distortion coefficient p2
     */
    public float GetP2() {
        return p2;
    }

    /**
     * Set camera distortion coefficient k1.
     * 
     * @param k1 camera distortion coefficient k1
     */
    public void SetK1(float k1) {
        this.k1 = k1;
    }

    /**
     * Set camera distortion coefficient k2.
     * 
     * @param k2 camera distortion coefficient k2
     */
    public void SetK2(float k2) {
        this.k2 = k2;
    }

    /**
     * Set camera distortion coefficient k3.
     * 
     * @param k3 camera distortion coefficient k3
     */
    public void SetK3(float k3) {
        this.k3 = k3;
    }

    /**
     * Set camera distortion coefficient k4.
     * 
     * @param k4 camera distortion coefficient k4
     */
    public void SetK4(float k4) {
        this.k4 = k4;
    }

    /**
     * Set camera distortion coefficient k5.
     * 
     * @param k5 camera distortion coefficient k5
     */
    public void SetK5(float k5) {
        this.k5 = k5;
    }

    /**
     * Set camera distortion coefficient k6.
     * 
     * @param k6 camera distortion coefficient k6
     */
    public void SetK6(float k6) {
        this.k6 = k6;
    }

    /**
     * Set camera distortion coefficient p1.
     * 
     * @param p1 camera distortion coefficient p1
     */
    public void SetP1(float p1) {
        this.p1 = p1;
    }

    /**
     * Set camera distortion coefficient p2.
     * 
     * @param p2 camera distortion coefficient p2
     */
    public void SetP2(float p2) {
        this.p2 = p2;
    }
}
