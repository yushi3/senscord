/*
 * SPDX-FileCopyrightText: 2019-2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_types;

import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

/**
 * Handling extrinsic parameters of calibration.
 * 
 * @see "senscord::ExtrinsicCalibrationParameter"
 */
public class ExtrinsicCalibrationParameter extends Structure {
    public static class ByReference extends ExtrinsicCalibrationParameter
            implements Structure.ByReference {
    }

    public float r11;
    public float r12;
    public float r13;
    public float r21;
    public float r22;
    public float r23;
    public float r31;
    public float r32;
    public float r33;
    public float t1;
    public float t2;
    public float t3;
    public Matrix3x4Float p;

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("r11", "r12", "r13", "r21", "r22", "r23", "r31", "r32", "r33", "t1",
                "t2", "t3", "p");
    }

    /**
     * Get extrinsic parameter r11.
     * 
     * @return extrinsic parameter r11
     */
    public float GetR11() {
        return r11;
    }

    /**
     * Get extrinsic parameter r12.
     * 
     * @return extrinsic parameter r12
     */
    public float GetR12() {
        return r12;
    }

    /**
     * Get extrinsic parameter r13.
     * 
     * @return extrinsic parameter r13
     */
    public float GetR13() {
        return r13;
    }

    /**
     * Get extrinsic parameter r21.
     * 
     * @return extrinsic parameter r21
     */
    public float GetR21() {
        return r21;
    }

    /**
     * Get extrinsic parameter r22.
     * 
     * @return extrinsic parameter r22
     */
    public float GetR22() {
        return r22;
    }

    /**
     * Get extrinsic parameter r23.
     * 
     * @return extrinsic parameter r23
     */
    public float GetR23() {
        return r23;
    }

    /**
     * Get extrinsic parameter r31.
     * 
     * @return extrinsic parameter r31
     */
    public float GetR31() {
        return r31;
    }

    /**
     * Get extrinsic parameter r32.
     * 
     * @return extrinsic parameter r32
     */
    public float GetR32() {
        return r32;
    }

    /**
     * Get extrinsic parameter r33.
     * 
     * @return extrinsic parameter r33
     */
    public float GetR33() {
        return r33;
    }

    /**
     * Get extrinsic parameter t1.
     * 
     * @return extrinsic parameter t1
     */
    public float GetT1() {
        return t1;
    }

    /**
     * Get extrinsic parameter t2.
     * 
     * @return extrinsic parameter t2
     */
    public float GetT2() {
        return t2;
    }

    /**
     * Get extrinsic parameter t3.
     * 
     * @return extrinsic parameter t3
     */
    public float GetT3() {
        return t3;
    }

    /**
     * Get extrinsic parameter p11-p34.
     *
     * @return extrinsic parameter p11-p34
     */
    public Matrix3x4Float GetP() {
        return p;
    }

    /**
     * Set extrinsic parameter r11.
     * 
     * @param r11 extrinsic parameter
     */
    public void SetR11(float r11) {
        this.r11 = r11;
    }

    /**
     * Set extrinsic parameter r12.
     * 
     * @param r12 extrinsic parameter
     */
    public void SetR12(float r12) {
        this.r12 = r12;
    }

    /**
     * Set extrinsic parameter r13.
     * 
     * @param r13 extrinsic parameter
     */
    public void SetR13(float r13) {
        this.r13 = r13;
    }

    /**
     * Set extrinsic parameter r21.
     * 
     * @param r21 extrinsic parameter
     */
    public void SetR21(float r21) {
        this.r21 = r21;
    }

    /**
     * Set extrinsic parameter r22.
     * 
     * @param r22 extrinsic parameter
     */
    public void SetR22(float r22) {
        this.r22 = r22;
    }

    /**
     * Set extrinsic parameter r23.
     * 
     * @param r23 extrinsic parameter
     */
    public void SetR23(float r23) {
        this.r23 = r23;
    }

    /**
     * Set extrinsic parameter r31.
     * 
     * @param r31 extrinsic parameter
     */
    public void SetR31(float r31) {
        this.r31 = r31;
    }

    /**
     * Set extrinsic parameter r32.
     * 
     * @param r32 extrinsic parameter
     */
    public void SetR32(float r32) {
        this.r32 = r32;
    }

    /**
     * Set extrinsic parameter r33.
     * 
     * @param r33 extrinsic parameter
     */
    public void SetR33(float r33) {
        this.r33 = r33;
    }

    /**
     * Set extrinsic parameter t1.
     * 
     * @param t1 extrinsic parameter
     */
    public void SetT1(float t1) {
        this.t1 = t1;
    }

    /**
     * Set extrinsic parameter t2.
     * 
     * @param t2 extrinsic parameter
     */
    public void SetT2(float t2) {
        this.t2 = t2;
    }

    /**
     * Set extrinsic parameter t3.
     * 
     * @param t3 extrinsic parameter
     */
    public void SetT3(float t3) {
        this.t3 = t3;
    }

    /**
     * Set extrinsic parameter p11-p34.
     *
     * @param p p11-p34 extrinsic parameter
     */
    public void SetP(Matrix3x4Float p) {
        this.p = p;
    }
}
