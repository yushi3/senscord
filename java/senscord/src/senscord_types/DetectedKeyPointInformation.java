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
 * Detected Key Point Information.
 *
 * @see "senscord::DetectedKeyPointInformation"
 */
public class DetectedKeyPointInformation extends Structure {
    public int class_id;
    public float score;
    public int count;
    public KeyPoint.ByReference key_points;

    public static class ByReference extends DetectedKeyPointInformation implements Structure.ByReference {
    }

    // @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("class_id", "score", "count", "key_points");
    }

    public DetectedKeyPointInformation() {
        this.key_points = new KeyPoint.ByReference();
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
     * Get key_point.
     *
     * @return key_point
     */
    public KeyPoint[] GetKeyPoints() {
        KeyPoint[] retData;
        if ((count == 0) || (key_points == null)) {
            retData = new KeyPoint[1];
            retData[0] = new KeyPoint();
            retData[0].key_point_id = 0;
            retData[0].score = 0;
            retData[0].point.x = 0;
            retData[0].point.y = 0;
            retData[0].point.z = 0;
        } else {
            retData = (KeyPoint[]) key_points.toArray(count);
        }
        return retData;
    }

    /**
     * Get size of key_points.
     *
     * @return size
     */
    public int GetCount() {
        return this.count;
    }

}
