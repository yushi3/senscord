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
 * Calibration parameters of a single camera.
 * 
 * @see "senscord::CameraCalibrationParameters"
 */
public class CameraCalibrationParameter extends Structure {
    public int channel_id;
    public IntrinsicCalibrationParameter intrinsic = new IntrinsicCalibrationParameter();
    public DistortionCalibrationParameter distortion = new DistortionCalibrationParameter();
    public ExtrinsicCalibrationParameter extrinsic = new ExtrinsicCalibrationParameter();

    public static class ByReference extends CameraCalibrationParameter
            implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("channel_id", "intrinsic", "distortion", "extrinsic");
    }

    /**
     * Get channel ID.
     * 
     * @return channel ID
     */
    public int GetChannelId() {
        return channel_id;
    }

    /**
     * Get camera internal parameter.
     * 
     * @return camera internal parameter
     */
    public IntrinsicCalibrationParameter GetIntrinsicCalibrationParameter() {
        return intrinsic;
    }

    /**
     * Get distortion connection coefficient.
     * 
     * @return distortion connection coefficient
     */
    public DistortionCalibrationParameter GetDistortionCalibrationParameter() {
        return distortion;
    }

    /**
     * Get extrinsic parameter.
     * 
     * @return extrinsic parameter
     */
    public ExtrinsicCalibrationParameter GetExtrinsicCalibrationParameter() {
        return extrinsic;
    }

}
