/*
 * SPDX-FileCopyrightText: 2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

/**
 * Handling Depth data properties.
 * 
 * @see "senscord::DepthProperty"
 */
public class DepthProperty extends Structure {
    public float scale;
    public float depth_min_range;
    public float depth_max_range;

    public static class ByReference extends DepthProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("scale", "depth_min_range", "depth_max_range");
    }

    /**
     * Depth pixel format.
     */
    public static interface PixelFormat {
        /**
         * 16 bit Z-Depth.
         */
        public static final String SENSCORD_PIXEL_FORMAT_Z16 = "depth_z16";

        /**
         * 32 bit float Z-Depth.
         */
        public static final String SENSCORD_PIXEL_FORMAT_Z32F = "depth_z32f";

        /**
         * 16 bit Disparity.
         */
        public static final String SENSCORD_PIXEL_FORMAT_D16 = "depth_d16";
    }

    /**
     * Get scale in meters. By multiplying this value to depth value can convert to meter unit.
     * 
     * @return scale
     */
    public float GetScale() {
        return scale;
    }

    /**
     * Get minimum depth data of the sensor.
     * 
     * @return minimum data
     */
    public float GetDepthMinRange() {
        return depth_min_range;
    }

    /**
     * Get maximum depth data of the sensor.
     * 
     * @return maximum data
     */
    public float GetDepthMaxRange() {
        return depth_max_range;
    }

    /**
     * Set scale.
     * 
     * @param scale Scale of the depth value, in metres
     */
    public void SetScale(float scale) {
        this.scale = scale;
    }

    /**
     * Set minimum depth data of the sensor.
     * 
     * @param depth_min_range minimum data
     */
    public void SetDepthMinRange(float depth_min_range) {
        this.depth_min_range = depth_min_range;
    }

    /**
     * Set maximum depth data of the sensor.
     * 
     * @param depth_max_range maximum data
     */
    public void SetDepthMaxRange(float depth_max_range) {
        this.depth_max_range = depth_max_range;
    }

}
