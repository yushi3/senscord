/*
 * SPDX-FileCopyrightText: 2019-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_types;

import com.sun.jna.Native;
import com.sun.jna.Structure;

import java.util.Arrays;
import java.util.List;

/**
 * Channel information.
 * 
 * @see "senscord::ChannelInfo"
 */
public class ChannelInfo extends Structure {
    public int channel_id;
    public byte[] raw_data_type;
    public byte[] description;

    public static class ByReference extends ChannelInfo implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("channel_id", "raw_data_type", "description");
    }

    /**
     * Constructor.
     */
    public ChannelInfo() {
        super();
        raw_data_type = new byte[64];
        description = new byte[128];

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
     * Get raw data type.
     * 
     * @return raw data type
     */
    public String GetRawDataTypeString() {
        return Native.toString(raw_data_type);
    }

    /**
     * Get description.
     * 
     * @return description
     */
    public String GetDescriptionString() {
        return Native.toString(description);
    }

}
