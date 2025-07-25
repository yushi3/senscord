/*
 * SPDX-FileCopyrightText: 2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import java.util.Arrays;
import java.util.List;
import com.sun.jna.Structure;


/**
 * Property for masking the channel.
 * 
 * @see "senscord::ChannelMaskProperty"
 */
public class ChannelMaskProperty extends Structure {
    public int count;
    public int[] channels;

    public static class ByReference extends ChannelMaskProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("count", "channels");
    }

    public ChannelMaskProperty() {
        super();
        channels = new int[128];
    }

    /**
     * Get masked channel count.
     * 
     * @return masked channel count
     */
    public int GetCount() {
        return count;
    }

    /**
     * Get channel masked channel ID.
     * 
     * @return masked channel ID.
     */
    public int[] GetChannels() {
        return channels;
    }
}
