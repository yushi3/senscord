/*
 * SPDX-FileCopyrightText: 2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;
import senscord_types.ChannelInfo;

/**
 * Property for channel information.
 * 
 * @see "senscord::ChannelInfoProperty"
 */
public class ChannelInfoProperty extends Structure {
    public int count;
    public ChannelInfo[] channel_info;

    public static class ByReference extends ChannelInfoProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("count", "channel_info");
    }

    public ChannelInfoProperty() {
        channel_info = new ChannelInfo[128];
    }

    /**
     * Get channel info count.
     * 
     * @return channel info count
     */
    public int GetCount() {
        return count;
    }

    /**
     * Get channel info class array.
     * 
     * @return channel info class array
     */
    public ChannelInfo[] GetChannelInfo() {
        return channel_info;
    }

}
