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
 * Setting frame rate.
 * Specify in the style of numerator / denominator. ex) 60fps : num = 60, denom = 1
 * 
 * @see "senscord::FrameRateProperty"
 */
public class FrameRateProperty extends Structure {
    public int num;
    public int denom;

    public static class ByReference extends FrameRateProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("num", "denom");
    }

    /**
     * Get frame rate numerator.
     * 
     * @return frame rate numerator
     */
    public int GetNum() {
        return num;
    }

    /**
     * Get frame rate denominator.
     * 
     * @return frame rate denominator
     */
    public int GetDenom() {
        return denom;
    }

    /**
     * Set frame rate numerator.
     * 
     * @param num frame rate numerator
     */
    public void SetNum(int num) {
        this.num = num;
    }

    /**
     * Set Frame rate denominator.
     * 
     * @param denom frame rate denominator
     */
    public void SetDenom(int denom) {
        this.denom = denom;
    }
}
