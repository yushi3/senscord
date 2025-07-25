/*
 * SPDX-FileCopyrightText: 2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;
import senscord_types.CameraCalibrationParameter;

/**
 * Property for camera calibration.
 * 
 * @see "senscord::CameraCalibrationProperty"
 */
public class CameraCalibrationProperty extends Structure {
    public int count;
    public CameraCalibrationParameter[] parameters;

    public static class ByReference extends CameraCalibrationProperty
            implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("count", "parameters");
    }

    public CameraCalibrationProperty() {
        parameters = new CameraCalibrationParameter[128];
    }

    /**
     * Get parameter count.
     * 
     * @return parameter count
     */
    public int GetCount() {
        return count;
    }

    /**
     * Get camera calibration parameter array.
     * 
     * @return camera calibration array. Maximum length is 128.
     */
    public CameraCalibrationParameter[] GetCameraCalibrationParameter() {
        return parameters;
    }
}
