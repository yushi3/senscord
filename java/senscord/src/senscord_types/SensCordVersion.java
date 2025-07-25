/*
 * SPDX-FileCopyrightText: 2019-2020 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_types;

import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;
import senscord_properties.VersionProperty;

/**
 * SensCord version information.
 * @see "senscord::SensCordVersion"
 */
public class SensCordVersion extends Structure {
    public static class ByReference extends SensCordVersion implements Structure.ByReference {
    }

    public VersionProperty senscord_version;
    public VersionProperty project_version;
    public int stream_count;
    public Pointer stream_versions;
    public int destination_id;
    public int server_count;
    public Pointer server_versions;

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList(
            "senscord_version",
            "project_version",
            "stream_count",
            "stream_versions",
            "destination_id",
            "server_count",
            "server_versions");
    }

    /**
     * Constructor.
     */
    public SensCordVersion() {
        super();
    }

    /**
     * Constructor.
     * @param pointer JNA Pointer.
     */
    public SensCordVersion(Pointer pointer) {
        super(pointer);
        read();
    }

    /**
     * Get SensCord version.
     * @return SensCord version.
     */
    public VersionProperty GetSensCordVersion() {
        return senscord_version;
    }

    /**
     * Get project version.
     * @return project version.
     */
    public VersionProperty GetProjectVersion() {
        return project_version;
    }

    /**
     * Get stream count.
     * @return stream count.
     */
    public int GetStreamCount() {
        return stream_count;
    }

    /**
     * Get stream version.
     * @param index index of stream count.
     * @return stream version.
     */
    public StreamVersion GetStreamVersion(int index) {
        final int STREAM_SIZE = 608;
        Pointer p_stream = stream_versions.share(STREAM_SIZE * index);
        StreamVersion stream = new StreamVersion(p_stream);
        return stream;
    }
 
    /**
     * Get destination id.
     * @return destination id.
     */
    public int GetDestinationId() {
        return destination_id;
    }

    /**
     * Get stream count.
     * @return stream count.
     */
    public int GetServerCount() {
        return server_count;
    }

    /**
     * Get server version.
     * @param index index of server count.
     * @return server version.
     */
    public SensCordVersion GetServerVersion(int index) {
        final int SERVER_SIZE = 1080;
        Pointer p_server = server_versions.share(SERVER_SIZE * index);
        SensCordVersion version = new SensCordVersion(p_server);
        return version;
    }
}
