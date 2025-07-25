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
 * Stream status.
 * 
 * @see "senscord::StreamState"
 */
public class StreamStateProperty extends Structure {
    public int state;

    public static class ByReference extends StreamStateProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("state");
    }

    /**
     * Stream status.
     * 
     * @see "senscord::StreamState"
     */
    public static interface StreamState {
        /**
         * Undefined state.
         */
        public static final int SENSCORD_STREAM_STATE_UNDEFINED = 0;

        /**
         * Opened but not start.
         */
        public static final int SENSCORD_STREAM_STATE_READY = 1;

        /**
         * Started.
         */
        public static final int SENSCORD_STREAM_STATE_RUNNING = 2;

    }

    /**
     * Get stream state.
     * 
     * @return stream state
     */
    public int GetState() {
        return state;
    }

    /**
     * Get stream state of String type from state value.
     * 
     * @param state state value
     * @return stream state string
     */
    public String GetStateString(int state) {
        String ret = "";

        switch (state) {
            case StreamState.SENSCORD_STREAM_STATE_UNDEFINED:
                ret = "UNDEFINED";
                break;
            case StreamState.SENSCORD_STREAM_STATE_READY:
                ret = "READY";
                break;
            case StreamState.SENSCORD_STREAM_STATE_RUNNING:
                ret = "RUNNING";
                break;
            default:
                break;
        }
        return ret;
    }

}
