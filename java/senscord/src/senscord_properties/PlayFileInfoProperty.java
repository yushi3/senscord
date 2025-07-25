/*
 * SPDX-FileCopyrightText: 2021-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Native;
import com.sun.jna.Structure;

import java.util.Arrays;
import java.util.List;

/**
 * PlayFileInfoProperty.
 *
 * @see "senscord::kPlayFileInfoProperty"
 */

public class PlayFileInfoProperty extends Structure {

    /** Length of the file path string. */
    private static final int SENSCORD_FILE_PATH_LENGTH = 256;
    /** Length of the record date string. */
    private static final int SENSCORD_RECORD_DATE_LENGTH = 32;
    /** Length of the stream key string. */
    private static final int SENSCORD_STREAM_KEY_LENGTH = 64;
    /** Length of the raw data type string. */
    private static final int SENSCORD_RAWDATA_TYPE_LENGTH = 64;

    public byte[] target_path;
    public byte[] record_date;
    public byte[] stream_key;
    public byte[] stream_type;
    public int frame_count;

    public static class ByReference extends PlayFileInfoProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList(
            "target_path", "record_date", "stream_key", "stream_type",
            "frame_count");
    }

    /**
     * Constructor.
     */
    public PlayFileInfoProperty() {
        super();
        target_path = new byte[SENSCORD_FILE_PATH_LENGTH];
        record_date = new byte[SENSCORD_RECORD_DATE_LENGTH];
        stream_key = new byte[SENSCORD_STREAM_KEY_LENGTH];
        stream_type = new byte[SENSCORD_RAWDATA_TYPE_LENGTH];
    }

    /**
     * Get target path.
     *
     * @return target_path
     */
    public String GetTargetPath() {
        return Native.toString(target_path);
    }

    /**
     * Set target path.
     *
     * @param target_path target path
     */
    public void SetTargetPath(String target_path) {
        Arrays.fill(this.target_path, (byte) 0);
        System.arraycopy(target_path.getBytes(), 0,
            this.target_path, 0, target_path.length());
    }

    /**
     * Get record date.
     *
     * @return record_date
     */
    public String GetRecordDate() {
        return Native.toString(record_date);
    }

    /**
     * Set record date.
     *
     * @param record_date Record date
     */
    public void SetRecordDate(String record_date) {
        Arrays.fill(this.record_date, (byte) 0);
        System.arraycopy(record_date.getBytes(), 0,
            this.record_date, 0, record_date.length());
    }

    /**
     * Get stream key.
     *
     * @return stream_key
     */
    public String GetStreamKey() {
        return Native.toString(stream_key);
    }

    /**
     * Set stream key.
     *
     * @param stream_key stream key
     */
    public void SetStreamKey(String stream_key) {
        Arrays.fill(this.stream_key, (byte) 0);
        System.arraycopy(stream_key.getBytes(), 0,
            this.stream_key, 0, stream_key.length());
    }

    /**
     * Get stream type.
     *
     * @return stream_type
     */
    public String GetStreamType() {
        return Native.toString(stream_type);
    }

    /**
     * Set stream type.
     *
     * @param stream_type stream type
     */
    public void SetStreamType(String stream_type) {
        Arrays.fill(this.stream_type, (byte) 0);
        System.arraycopy(stream_type.getBytes(), 0,
            this.stream_type, 0, stream_type.length());
    }

    /**
     * Get frame count.
     *
     * @return frame count
     */
    public int GetFrameCount() {
        return this.frame_count;
    }

    /**
     * Set frame count.
     *
     * @param frame_count frame count
     */
    public void SetFrameCount(int frame_count) {
        this.frame_count = frame_count;
    }
}
