/*
 * SPDX-FileCopyrightText: 2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;
import senscord_types.PoseQuaternionData;

/**
 * Handled by this Key, use senscord_pose_data_t structure.
 * 
 * @see "senscord::kInitialPosePropertyKey"
 */
public class InitialPoseProperty extends Structure {
    public PoseQuaternionData pose;

    public static class ByReference extends InitialPoseProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("pose");
    }

    /**
     * Get pose data.
     * 
     * @return pose data
     */
    public PoseQuaternionData GetPose() {
        return pose;
    }

    /**
     * Set pose data.
     * 
     * @param pose Pose data
     */
    public void SetPose(PoseQuaternionData pose) {
        this.pose = pose;
    }
}
