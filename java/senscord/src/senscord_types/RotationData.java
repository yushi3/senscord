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
 * Data for rotating posture.
 * 
 * @see "senscord::RotationData"
 */
public class RotationData extends Structure {
    public static class ByReference extends RotationData implements Structure.ByReference {
    }

    public float roll;
    public float pitch;
    public float yaw;

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("roll", "pitch", "yaw");
    }

    /**
     * Get roll angle.
     * 
     * @return roll angle
     */
    public float GetRoll() {
        return roll;
    }

    /**
     * Get pitch angle.
     * 
     * @return pitch angle
     */
    public float GetPitch() {
        return pitch;
    }

    /**
     * Get yaw angle.
     * 
     * @return yaw angle
     */
    public float GetYaw() {
        return yaw;
    }

}
