/*
 * SPDX-FileCopyrightText: 2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_types;

import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

/**
 * Information about the record for each channel.
 */

public class RecordInfo extends Structure {
    public int channel_id;
    public RecorderFormat format;

    public static class ByReference extends RecordInfo implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("channel_id", "format");
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
     * Set channel ID.
     * 
     * @param id channel ID
     */
    public void SetChannelId(int id) {
        this.channel_id = id;
    }

    /**
     * Get format.
     * 
     * @return format class.
     */
    public RecorderFormat GetFormat() {
        return this.format;
    }

    /**
     * Set format.
     * 
     * @param format format class.
     */
    public void SetFormat(RecorderFormat format) {
        this.format = format;
    }

}
