/*
 * SPDX-FileCopyrightText: 2019-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Native;
import com.sun.jna.Structure;

import java.util.Arrays;
import java.util.List;

/**
 * Handle properties of raw data of confidence.
 * 
 * @see "senscord::ConfidenceProperty"
 */
public class ConfidenceProperty extends Structure {
    public int width;
    public int height;
    public int stride_bytes;
    public byte[] pixel_format;

    public static class ByReference extends ConfidenceProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("width", "height", "stride_bytes", "pixel_format");
    }

    public static interface PixelFormat {
        /**
         * 1-bit positive confidence.
         */
        public static final String SENSCORD_PIXEL_FORMAT_C1P = "confidence_c1p";

        /**
         * 1-bit negative confidence.
         */
        public static final String SENSCORD_PIXEL_FORMAT_C1N = "confidence_c1n";

        /**
         * 16bit confidence.
         */
        public static final String SENSCORD_PIXEL_FORMAT_C16 = "confidence_c16";

        /**
         * 32bit float confidence.
         */
        public static final String SENSCORD_PIXEL_FORMAT_C32F = "confidence_c32f";
    }

    public ConfidenceProperty() {
        super();
        pixel_format = new byte[64];
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

    /**
     * Get image width.
     * 
     * @return width image width
     */
    public int GetWidth() {
        return this.width;
    }

    /**
     * Set image height.
     * 
     * @return height image height
     */
    public int GetHeight() {
        return this.height;
    }

    /**
     * Set image stride.
     * 
     * @return stride_bytes image stride
     */
    public int GetStrideBytes() {
        return this.stride_bytes;
    }

    /**
     * Set pixel format.
     * 
     * @return pixel_format image pixel format. Maximum length is 64. {@link PixelFormat}
     */
    public String GetPixelFormat() {
        return Native.toString(this.pixel_format);
    }


}
