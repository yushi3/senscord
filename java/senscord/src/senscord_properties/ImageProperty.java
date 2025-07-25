/*
 * SPDX-FileCopyrightText: 2019-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Native;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

/**
 * Handle properties of Raw data of Image and Depth data.
 *
 * @see "senscord::ImageProperty"
 */
public class ImageProperty extends Structure {
    public int width;
    public int height;
    public int stride_bytes;
    public byte[] pixel_format;

    public static class ByReference extends ImageProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("width", "height", "stride_bytes", "pixel_format");
    }

    /**
     * Image pixel format.
     */
    public static interface PixelFormat {
        /* RGB */

        /**
         * ARGB 4444.
         */
        public static final String SENSCORD_PIXEL_FORMAT_ARGB444 = "image_argb444";

        /**
         * XRGB 4444.
         */
        public static final String SENSCORD_PIXEL_FORMAT_XRGB444 = "image_xrgb444";

        /**
         * RGB 888.
         */
        public static final String SENSCORD_PIXEL_FORMAT_RGB24 = "image_rgb24";

        /**
         * ARGB 8888.
         */
        public static final String SENSCORD_PIXEL_FORMAT_ARGB32 = "image_argb32";

        /**
         * XRGB 4444.
         */
        public static final String SENSCORD_PIXEL_FORMAT_XRGB32 = "image_xrgb32";

        /**
         * BRG 888.
         */
        public static final String SENSCORD_PIXEL_FORMAT_BGR24 = "image_bgr24";

        /**
         * ABGR 8888.
         */
        public static final String SENSCORD_PIXEL_FORMAT_ABGR32 = "image_abgr32";

        /**
         * XBGR 8888.
         */
        public static final String SENSCORD_PIXEL_FORMAT_XBGR32 = "image_xbgr32";

        /* Planar RGB */

        /**
         * RGB 8-bit.
         */
        public static final String SENSCORD_PIXEL_FORMAT_RGB8_PLANAR = "image_rgb8_planar";

        /**
         * RGB 16-bit.
         */
        public static final String SENSCORD_PIXEL_FORMAT_RGB16_PLANAR = "image_rgb16_planar";

        /* Grayscale */

        /**
         * 8 -bit Greyscale.
         */
        public static final String SENSCORD_PIXEL_FORMAT_GREY = "image_grey";

        /**
         * 10 -bit Greyscale (on 16 bit).
         */
        public static final String SENSCORD_PIXEL_FORMAT_Y10 = "image_y10";

        /**
         * 12 -bit Greyscale (on 16 bit).
         */
        public static final String SENSCORD_PIXEL_FORMAT_Y12 = "image_y12";

        /**
         * 14 -bit Greyscale (on 16 bit).
         */
        public static final String SENSCORD_PIXEL_FORMAT_Y14 = "image_y14";

        /**
         * 16 -bit Greyscale.
         */
        public static final String SENSCORD_PIXEL_FORMAT_Y16 = "image_y16";

        /**
         * 20 -bit Greyscale (on 32bit).
         */
        public static final String SENSCORD_PIXEL_FORMAT_Y20 = "image_y20";

        /**
         * 24 -bit Greyscale (on 32bit).
         */
        public static final String SENSCORD_PIXEL_FORMAT_Y24 = "image_y24";

        /* YUV */

        /**
         * YUV444.
         */
        public static final String SENSCORD_PIXEL_FORMAT_YUV444 = "image_yuv444";

        /**
         * YUV420SP.
         */
        public static final String SENSCORD_PIXEL_FORMAT_NV12 = "image_nv12";

        /**
         * YUV422SP.
         */
        public static final String SENSCORD_PIXEL_FORMAT_NV16 = "image_nv16";

        /**
         * YUV420.
         */
        public static final String SENSCORD_PIXEL_FORMAT_YUV420 = "image_yuv420";

        /**
         * YUV422P.
         */
        public static final String SENSCORD_PIXEL_FORMAT_YUV422P = "image_yuv422p";

        /**
         * YUYV.
         */
        public static final String SENSCORD_PIXEL_FORMAT_YUYV = "image_yuyv";

        /**
         * UYVY.
         */
        public static final String SENSCORD_PIXEL_FORMAT_UYVY = "image_uyvy";

        /* Bayer */
        public static final String SENSCORD_PIXEL_FORMAT_SBGGR8 = "image_sbggr8";
        public static final String SENSCORD_PIXEL_FORMAT_SGBRG8 = "image_sgbrg8";
        public static final String SENSCORD_PIXEL_FORMAT_SGRBG8 = "image_sgrbg8";
        public static final String SENSCORD_PIXEL_FORMAT_SRGGB8 = "image_srggb8";

        public static final String SENSCORD_PIXEL_FORMAT_SBGGR10 = "image_sbggr10";
        public static final String SENSCORD_PIXEL_FORMAT_SGBRG10 = "image_sgbrg10";
        public static final String SENSCORD_PIXEL_FORMAT_SGRBG10 = "image_sgrbg10";
        public static final String SENSCORD_PIXEL_FORMAT_SRGGB10 = "image_srggb10";

        public static final String SENSCORD_PIXEL_FORMAT_SBGGR12 = "image_sbggr12";
        public static final String SENSCORD_PIXEL_FORMAT_SGBRG12 = "image_sgbrg12";
        public static final String SENSCORD_PIXEL_FORMAT_SGRBG12 = "image_sgrbg12";
        public static final String SENSCORD_PIXEL_FORMAT_SRGGB12 = "image_srggb12";

        /* Quad Bayer */
        public static final String SENSCORD_PIXEL_FORMAT_QUAD_SBGGR8 = "image_quad_sbggr8";
        public static final String SENSCORD_PIXEL_FORMAT_QUAD_SGBRG8 = "image_quad_sgbrg8";
        public static final String SENSCORD_PIXEL_FORMAT_QUAD_SGRBG8 = "image_quad_sgrbg8";
        public static final String SENSCORD_PIXEL_FORMAT_QUAD_SRGGB8 = "image_quad_srggb8";

        public static final String SENSCORD_PIXEL_FORMAT_QUAD_SBGGR10 = "image_quad_sbggr10";
        public static final String SENSCORD_PIXEL_FORMAT_QUAD_SGBRG10 = "image_quad_sgbrg10";
        public static final String SENSCORD_PIXEL_FORMAT_QUAD_SGRBG10 = "image_quad_sgrbg10";
        public static final String SENSCORD_PIXEL_FORMAT_QUAD_SRGGB10 = "image_quad_srggb10";

        public static final String SENSCORD_PIXEL_FORMAT_QUAD_SBGGR12 = "image_quad_sbggr12";
        public static final String SENSCORD_PIXEL_FORMAT_QUAD_SGBRG12 = "image_quad_sgbrg12";
        public static final String SENSCORD_PIXEL_FORMAT_QUAD_SGRBG12 = "image_quad_sgrbg12";
        public static final String SENSCORD_PIXEL_FORMAT_QUAD_SRGGB12 = "image_quad_srggb12";

        /* Polarization image */
        public static final String SENSCORD_PIXEL_FORMAT_POLAR_90_45_135_0_Y8 =
                "image_polar_90_45_135_0_y8";
        public static final String SENSCORD_PIXEL_FORMAT_POLAR_90_45_135_0_Y10 =
                "image_polar_90_45_135_0_y10";
        public static final String SENSCORD_PIXEL_FORMAT_POLAR_90_45_135_0_Y12 =
                "image_polar_90_45_135_0_y12";
        public static final String SENSCORD_PIXEL_FORMAT_POLAR_90_45_135_0_RGGB8 =
                "image_polar_90_45_135_0_rggb8";
        public static final String SENSCORD_PIXEL_FORMAT_POLAR_90_45_135_0_RGGB10 =
                "image_polar_90_45_135_0_rggb10";
        public static final String SENSCORD_PIXEL_FORMAT_POLAR_90_45_135_0_RGGB12 =
                "image_polar_90_45_135_0_rggb12";

        /* Compressed image */
        public static final String SENSCORD_PIXEL_FORMAT_JPEG = "image_jpeg";
        public static final String SENSCORD_PIXEL_FORMAT_H264 = "image_h264";
    }

    public ImageProperty() {
        super();
        pixel_format = new byte[64];
    }

    /**
     * Get image width.
     *
     * @return image width
     */
    public int GetWidth() {
        return width;
    }

    /**
     * Get image height.
     *
     * @return image height
     */
    public int GetHeight() {
        return height;
    }

    /**
     * Get image stride.
     *
     * @return image stride
     */
    public int GetStrideBytes() {
        return stride_bytes;
    }

    /**
     * Get pixel format.
     *
     * @return pixel format
     */
    public String GetPixelFormat() {
        return Native.toString(pixel_format);
    }

    /**
     * Set image width.
     *
     * @param width image width
     */
    public void SetWidth(int width) {
        this.width = width;
    }

    /**
     * Set image height.
     *
     * @param height image height
     */
    public void SetHeight(int height) {
        this.height = height;
    }

    /**
     * Set image stride.
     *
     * @param stride_bytes image stride
     */
    public void SetStrideBytes(int stride_bytes) {
        this.stride_bytes = stride_bytes;
    }

    /**
     * Set pixel format.
     *
     * @param pixel_format image pixel format. Maximum length is 64. {@link PixelFormat}
     */
    public void SetPixelFormat(String pixel_format) {
        Arrays.fill(this.pixel_format, (byte) 0);
        System.arraycopy(pixel_format.getBytes(), 0, this.pixel_format, 0, pixel_format.length());
    }
}
