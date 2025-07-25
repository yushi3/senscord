/*
 * SPDX-FileCopyrightText: 2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import java.util.Arrays;
import java.util.List;
import com.sun.jna.Structure;

/**
 * Setting the skip rate of the frame.
 * If 'rate = 1' is specified, frames are not skipped. If 'rate = N' (N is 2 or more) is specified,
 * the frame is skipped and the frame rate drops to 1 / N.
 * 
 * @see "senscord::SkipFrameProperty"
 */

public class SkipFrameProperty extends Structure {
    public int rate;

    public static class ByReference extends SkipFrameProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("rate");
    }

    /**
     * Get skip rate.
     * 
     * @return skip rate
     */
    public int GetRate() {
        return rate;
    }

    /**
     * Set skip rate.
     * 
     * @param rate skip rate
     */
    public void SetRate(int rate) {
        this.rate = rate;
    }
}
