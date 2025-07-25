/*
 * SPDX-FileCopyrightText: 2021-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Structure;

import java.util.Arrays;
import java.util.List;

/**
 * Structure for threshold for the score to be output.
 * 
 * @see "senscord::ScoreThresholdProperty"
 */
public class ScoreThresholdProperty extends Structure {
    public float score_threshold;

    public static class ByReference extends ScoreThresholdProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("score_threshold");
    }

    public ScoreThresholdProperty() {
        super();
    }

    /**
     * Get score threshold value.
     * 
     * @return score threshold value.
     */
    public float GetScoreThreshold() {
        return score_threshold;
    }

    /**
     * Set score threshold value.
     * 
     * @param score_threshold score threshold value
     */
    public void SetScoreThreshold(float score_threshold) {
        this.score_threshold = score_threshold;
    }
}
