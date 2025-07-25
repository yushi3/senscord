/*
 * SPDX-FileCopyrightText: 2019-2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Native;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

/**
 * Point cloud Property.
 * 
 * @see "senscord::PointCloudProperty"
 */
public class PointCloudProperty extends Structure {
    public int height;
    public int width;
    public byte[] pixel_format;

    public static class ByReference extends PointCloudProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("height", "width", "pixel_format");
    }

    public PointCloudProperty() {
        super();
        pixel_format = new byte[64];
    }

    public static interface PixelFormat {
        /**
         * 16 bit (x, y, depth).
         */
        public static final String SENSCORD_PIXEL_FORMAT_XYZ16 = "point_cloud_xyz16";

        /**
         * 16 bit (x, y, depth, rgb).
         */
        public static final String SENSCORD_PIXEL_FORMAT_XYZRGB16 = "point_cloud_xyzrgb16";

        /**
         * 32 bit (x, y, depth).
         */
        public static final String SENSCORD_PIXEL_FORMAT_XYZ32 = "point_cloud_xyz32";

        /**
         * 32 bit (x, y, depth, rgb).
         */
        public static final String SENSCORD_PIXEL_FORMAT_XYZRGB32 = "point_cloud_xyzrgb32";

        /**
         * 32 bit (x, y, depth).
         */
        public static final String SENSCORD_PIXEL_FORMAT_XYZ32U = "point_cloud_xyz32u";

        /**
         * 32 bit (x, y, depth, rgb).
         */
        public static final String SENSCORD_PIXEL_FORMAT_XYZRGB32U = "point_cloud_xyzrgb32u";

        /**
         * 16 bit (x, y, depth).
         */
        public static final String SENSCORD_PIXEL_FORMAT_XYZ16U = "point_cloud_xyz16u";

        /**
         * 16 bit (x, y, depth, rgb).
         */
        public static final String SENSCORD_PIXEL_FORMAT_XYZRGB16U = "point_cloud_xyzrgb16u";

        /**
         * 32 bit float (x, y, depth).
         */
        public static final String SENSCORD_PIXEL_FORMAT_XYZ32F = "point_cloud_xyz32f";

        /**
         * 32 bit float (x, y, depth, rgb).
         */
        public static final String SENSCORD_PIXEL_FORMAT_XYZ32RGBF = "point_cloud_xyzrgb32f";

        /**
         * 16bit (x, y, depth) planar array.
         */
        public static final String SENSCORD_PIXEL_FORMAT_XYZ16U_PLANAR =
                "point_cloud_xyz16u_planar";

        /**
         * 32 bit float (x, y, depth) planar array.
         */
        public static final String SENSCORD_PIXEL_FORMAT_XYZ32F_PLANAR =
                "point_cloud_xyz32f_planar";
    }

    /**
     * Get height of PointCloud.
     * 
     * @return height of PointCloud
     */
    public int GetHeight() {
        return height;
    }

    /**
     * Get width of PointCloud.
     * 
     * @return width of PointCloud
     */
    public int GetWidth() {
        return width;
    }

    /**
     * Get pixel format of PointCloud.
     * 
     * @return pixel format
     */
    public String GetPixelFormat() {
        return Native.toString(pixel_format);
    }

    /**
     * Set height of PointCloud.
     * 
     * @param height height of PointCloud
     */
    public void SetHeight(int height) {
        this.height = height;
    }

    /**
     * Set width of PointCloud.
     * 
     * @param width width of PointCloud
     */
    public void SetWidth(int width) {
        this.width = width;
    }

    /**
     * Set pixel format of PointCloud.
     * 
     * @param pixel_format pixlel format of PointCloud
     */
    public void SetPixelFormat(String pixel_format) {
        Arrays.fill(this.pixel_format, (byte) 0);
        System.arraycopy(pixel_format.getBytes(), 0, this.pixel_format, 0, pixel_format.length());
    }

}
