/*
 * SPDX-FileCopyrightText: 2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_types;

import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

/**
 * Tracked Object Information.
 *
 * @see "senscord::TrackedObjectInformation"
 */
public class TrackedObjectInformation extends Structure {
    public int track_id;
    public int class_id;
    public float score;
    public Vector2Float velocity;
    public Vector2Int position;
    public RectangleRegionParameter box;

    public static class ByReference extends TrackedObjectInformation implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("track_id", "class_id", "score", "velocity", "position", "box");
    }

    /**
     * Get track_id.
     *
     * @return track_id
     */
    public int GetTrackId() {
        return track_id;
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
     * Get velocity.
     * 
     * @return velocity
     */
    public Vector2Float GetVelocity() {
        return velocity;
    }

    /**
     * Get position.
     * 
     * @return position
     */
    public Vector2Int GetPosition() {
        return position;
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
