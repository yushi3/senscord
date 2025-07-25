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
 * Pose Matrix data.
 * 
 * @see "senscord::PoseMatrixData"
 */
public class PoseMatrixData extends Structure {
    public Vector3Float position;
    public Matrix3x3Float rotation;

    public static class ByReference extends PoseMatrixData implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("position", "rotation");
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
     * Get rotation.
     * 
     * @return rotation
     */
    public Matrix3x3Float GetRotation() {
        return rotation;
    }

    /**
     * Set position.
     * 
     * @param position position of PoseMatrixData
     */
    public void SetPosition(Vector3Float position) {
        this.position = position;
    }

    /**
     * Set rotation.
     * 
     * @param rotation rotation of PoseMatrixData
     */
    public void SetRotation(Matrix3x3Float rotation) {
        this.rotation = rotation;
    }
}
