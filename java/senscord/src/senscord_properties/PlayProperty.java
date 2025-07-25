/*
 * SPDX-FileCopyrightText: 2019-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Native;
import com.sun.jna.Structure;

import java.util.Arrays;
import java.util.List;

/**
 * PlayProperty.
 *
 * @see "senscord::kPlayPropertyKey"
 */

public class PlayProperty extends Structure {

    public static interface PlaySpeed {
        /** Sending based on framerate. */
        public static final int SENSCORD_PLAY_SPEED_BASED_ON_FRAMERATE = 0;

        /**
         * Sending without framerate.
         *
         * @deprecated replaces "BASED_ON_FRAMERATE" in player.
         */
        public static final int SENSCORD_PLAY_SPEED_BEST_EFFORT = 1;
    }

    /** Length of the file path string. */
    private static final int SENSCORD_FILE_PATH_LENGTH = 256;

    public byte[] target_path;
    public int start_offset;
    public int count;
    public int speed;
    public PlayModeProperty mode;

    public static class ByReference extends PlayProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("target_path", "start_offset", "count", "speed", "mode");
    }

    public PlayProperty() {
        super();
        target_path = new byte[SENSCORD_FILE_PATH_LENGTH];
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
        System.arraycopy(target_path.getBytes(), 0, this.target_path, 0, target_path.length());
    }

    /**
     * Get start offset.
     *
     * @return start_offset
     */
    public int GetStartOffset() {
        return start_offset;
    }

    /**
     * Set start offset.
     *
     * @param start_offset Offset of starting
     */
    public void SetStartOffset(int start_offset) {
        this.start_offset = start_offset;
    }

    /**
     * Get count.
     *
     * @return count
     */
    public int GetCount() {
        return count;
    }

    /**
     * Set count.
     *
     * @param count playing count from start_offset
     */
    public void SetCount(int count) {
        this.count = count;
    }

    /**
     * Get speed.
     *
     * @return speed
     */
    public int GetSpeed() {
        return speed;
    }

    /**
     * Set speed.
     *
     * @param speed playback speed
     */
    public void SetSpeed(int speed) {
        this.speed = speed;
    }

    /**
     * Get mode.
     *
     * @return mode repeat mode of playback
     */
    public PlayModeProperty GetMode() {
        return mode;
    }

    /**
     * Set mode.
     *
     * @param mode repeat mode of playback
     */
    public void SetMode(PlayModeProperty mode) {
        this.mode = mode;
    }

}
