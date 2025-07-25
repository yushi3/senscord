/*
 * SPDX-FileCopyrightText: 2019-2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_types;

import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;
import senscord_properties.VersionProperty;

/**
 * Stream version information.
 * @see "senscord::StreamVersion"
 */
public class StreamVersion extends Structure {
    public byte[] stream_key;
    public VersionProperty stream_version;
    public int linkage_count;
    public Pointer linkage_versions;
    public int destination_id;

    /**
     * Constructor.
     */
    public StreamVersion() {
        super();
        stream_key = new byte[64];
    }

    /**
     * Constructor.
     * @param pointer JNA Pointer.
     */
    public StreamVersion(Pointer pointer) {
        this();
        useMemory(pointer);
        read();
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList(
            "stream_key",
            "stream_version",
            "linkage_count",
            "linkage_versions",
            "destination_id");
    }

    /**
     * Get stream key.
     * @return stream key.
     */
    public String GetStreamKey() {
        return Native.toString(stream_key);
    }

    /**
     * Get stream version.
     * @return stream version.
     */
    public VersionProperty GetStreamVersion() {
        return stream_version;
    }

    /**
     * Get linkage count.
     * @return linkage count.
     */
    public int GetLinkageCount() {
        return linkage_count;
    }

    /**
     * Get linkage version.
     * @param index index of linkage count.
     * @return linkage version.
     */
    public VersionProperty GetLinkageVersion(int index) {
        final int VERSION_SIZE = 524;
        Pointer p_version = linkage_versions.share(VERSION_SIZE * index);
        VersionProperty version = new VersionProperty(p_version);
        return version;
    }

    /**
     * Get destination id.
     * @return destination id.
     */
    public int GetDestinationId() {
        return destination_id;
    }
}
