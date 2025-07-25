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
 * Property for the current buffering frames.
 * 
 * @see "senscord::CurrentFrameNumProperty"
 */
public class CurrentFrameNumProperty extends Structure {
    public int arrived_number;
    public int received_number;

    public static class ByReference extends CurrentFrameNumProperty
            implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("arrived_number", "received_number");
    }

    /**
     * Get arrived frame number.
     * 
     * @return arrived number
     */
    public int GetArrivedNumber() {
        return arrived_number;
    }


    /**
     * Get received frame number.
     * 
     * @return received frame number
     */
    public int GetReceivedNumber() {
        return received_number;
    }

}
