/*
 * SPDX-FileCopyrightText: 2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_types;

import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

/**
 * Detected Object Information.
 *
 * @see "senscord::DetectedObjectInformation"
 */
public class DetectedObjectInformation extends Structure {
    public int class_id;
    public float score;
    public RectangleRegionParameter box;

    public static class ByReference extends DetectedObjectInformation implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("class_id", "score", "box");
    }

    /**
     * Get class_id.
     *
     * @return class_id
     */
    public int GetClassId() {
        return class_id;
    }

    /**
     * Get score.
     *
     * @return score
     */
    public float GetScore() {
        return score;
    }

    /**
     * Get RectangleRegionParameter.
     *
     * @return box
     */
    public RectangleRegionParameter GetBox() {
        return box;
    }
}
