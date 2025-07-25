/*
 * SPDX-FileCopyrightText: 2019-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

/**
 * Data format supported by SLAM stream.
 * 
 * @see "senscord::SlamDataSupportedProperty"
 */
public class SlamDataSupportedProperty extends Structure {
    public byte odometry_supported;
    public byte gridmap_supported;
    public byte pointcloud_supported;

    public static class ByReference extends SlamDataSupportedProperty
            implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("odometry_supported", "gridmap_supported", "pointcloud_supported");
    }


    /**
     * Get position and attitude data supported or not.
     * 
     * @return position and attitude supported or not
     */
    public byte GetOdometrySupported() {
        return odometry_supported;
    }

    /**
     * Get GridMap supported or not.
     * 
     * @return GridMap supported or not
     */
    public byte GetGridMapSupported() {
        return gridmap_supported;
    }

    /**
     * Get PointCloud supported or not.
     * 
     * @return PointCloud supported or not
     */
    public byte GetPointCloudSupported() {
        return pointcloud_supported;
    }

    /**
     * Set position and attitude data supported or not.
     * 
     * @param odometry_supported position and attitude data supported or not
     */
    public void SetOdometrySupported(byte odometry_supported) {
        this.odometry_supported = odometry_supported;
    }

    /**
     * Set GridMap supported or not.
     * 
     * @param gridmap_supported GridMap supported or not
     */
    public void SetGridMapSupported(byte gridmap_supported) {
        this.gridmap_supported = gridmap_supported;
    }

    /**
     * Set PointCloud supported or not.
     * 
     * @param pointcloud_supported PointCloud supported or not
     */
    public void SetPointCloudSupported(byte pointcloud_supported) {
        this.pointcloud_supported = pointcloud_supported;
    }
}
