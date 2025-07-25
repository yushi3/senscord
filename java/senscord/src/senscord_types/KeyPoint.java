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
 * Key Point.
 *
 * @see "senscord::keyPoint"
 */
public class KeyPoint extends Structure {
    public int key_point_id;
    public float score;
    public Vector3Float point;

    public static class ByReference extends KeyPoint implements Structure.ByReference {
    }

    // @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("key_point_id", "score", "point");
    }

    /**
     * Get key_point_id.
     *
     * @return key_point_id
     */
    public int GetKeyPointId() {
        return key_point_id;
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
     * Get point.
     *
     * @return point
     */
    public Vector3Float GetPoint() {
        return point;
    }
}
