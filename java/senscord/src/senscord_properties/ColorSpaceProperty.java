/*
 * SPDX-FileCopyrightText: 2019-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Structure;

import java.util.Arrays;
import java.util.List;

/**
 * Property of color space type for YUV.
 * 
 * @see "senscord::ColorSpaceProperty"
 */
public class ColorSpaceProperty extends Structure {
    public int encoding;
    public int quantization;

    public static class ByReference extends ColorSpaceProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("encoding", "quantization");
    }

    /**
     * Encoding types for YUV (YCbCr).
     * 
     * @see "senscord::YCbCrEncoding"
     */
    public static interface Encoding {
        public static final int SENSCORD_YCBCR_ENCODING_UNDEFINED = 0;
        public static final int SENSCORD_YCBCR_ENCODING_BT601 = 1;
        public static final int SENSCORD_YCBCR_ENCODING_BT709 = 2;
        public static final int SENSCORD_YCBCR_ENCODING_BT2020 = 3;
        public static final int SENSCORD_YCBCR_ENCODING_BT2100 = 4;
    }

    /**
     * Quantization types for YUV (YCbCr).
     * 
     * @see "senscord::YCbCrQuantization"
     */
    public static interface Quantization {
        public static final int SENSCORD_YCBCR_QUANTIZATION_UNDEFINED = 0;

        /**
         * Y: 0 - 255, C: 0 - 255.
         */
        public static final int SENSCORD_YCBCR_QUANTIZATION_FULL_RANGE = 1;

        /**
         * Y: 16 - 255, C: 16 - 240.
         */
        public static final int SENSCORD_YCBCR_QUANTIZATION_KIMITED_RANGE = 2;
        public static final int SENSCORD_YCBCR_QUANTIZATION_SUPER_WHITE = 3;
    }

    /**
     * Get color space encoding.
     * 
     * @return color space encoding
     */
    public int GetEncoding() {
        return encoding;
    }

    /**
     * Get color space quantization.
     * 
     * @return color space encoding
     */
    public int GetQuantization() {
        return quantization;
    }

    /**
     * Set color space encoding.
     * 
     * @param encoding color space encoding. {@link Encoding}
     */
    public void SetEncoding(int encoding) {
        this.encoding = encoding;
    }

    /**
     * Set color space quantization.
     * 
     * @param quantization color space quantization {@link Quantization}}
     */
    public void SetQuantization(int quantization) {
        this.quantization = quantization;
    }
}
