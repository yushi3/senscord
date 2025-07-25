/*
 * SPDX-FileCopyrightText: 2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Native;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

/**
 * WhiteBalanceProperty.
 * 
 * @see "senscord::WhiteBalanceProperty"
 */
public class WhiteBalanceProperty extends Structure {
    public byte[] mode;

    public static class ByReference extends WhiteBalanceProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("mode");
    }

    public WhiteBalanceProperty() {
        super();
        mode = new byte[64];
    }

    /**
     * Exposure metering modes.
     */
    /* Metering mode : None (Disable) */
    public static interface Mode {
        /**
         * Auto.
         */
        public static final String SENSCORD_WHITE_BALANCE_AUTO = "auto";

        /**
         * Manual.
         */
        public static final String SENSCORD_WHITE_BALANCE_MANUAL = "manual";

    }

    /**
     * Get white balance mode.
     * 
     * @return white balance mode from {@link Mode}
     */
    public String GetMode() {
        return Native.toString(mode);
    }

    /**
     * Set white balance mode.
     * 
     * @param mode white balance mode form {@link Mode}
     */
    public void SetMode(String mode) {
        Arrays.fill(this.mode, (byte) 0);
        System.arraycopy(mode.getBytes(), 0, this.mode, 0, mode.length());
    }

}
