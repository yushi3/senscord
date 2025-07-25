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
 * Pose Quaternion data.
 * 
 * @see "senscord::PoseData"
 */
public class PoseQuaternionData extends Structure {
    public static class ByReference extends PoseQuaternionData implements Structure.ByReference {
    }

    public Vector3Float position;
    public QuaternionFloat orientation;

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("position", "orientation");
    }

    /**
     * Get position.
     * 
     * @return position
     */
    public Vector3Float GetPosition() {
        return position;
    }

    /**
     * Get orientation.
     * 
     * @return orientation
     */
    public QuaternionFloat GetOrientation() {
        return orientation;
    }

    /**
     * Set position.
     * 
     * @param position position of PoseQuaternionData
     */
    public void SetPosition(Vector3Float position) {
        this.position = position;
    }

    /**
     * Set orientation.
     * 
     * @param orientation orientation of PoseQuaternionData
     */
    public void SetOrientation(QuaternionFloat orientation) {
        this.orientation = orientation;
    }
}
