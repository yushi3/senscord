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
 * Error status.
 */
public class Status extends Structure {
    public int level;
    public int cause;
    public String message;
    public String block;
    public String trace;

    public static class ByValue extends Status implements Structure.ByValue {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("level", "cause", "message", "block", "trace");
    }

    /**
     * Level of error.
     * 
     * @see "senscord::Status::Level"
     */
    public static interface ErrorLevel {
        public static final int SENSCORD_LEVEL_UNDEFINED = 0;
        public static final int SENSCORD_LEVEL_FAIL = 1;
        public static final int SENSCORD_LEVEL_FATAL = 2;
    }

    /**
     * Cause of error.
     * 
     * @see "senscord::Status::Cause"
     */
    public static interface ErrorCause {
        public static final int SENSCORD_ERROR_NONE = 0;
        public static final int SENSCORD_ERROR_NOT_FOUND = 1;
        public static final int SENSCORD_ERROR_INVALID_ARGUMENT = 2;
        public static final int SENSCORD_ERROR_RESOURCE_EXHAUSTED = 3;
        public static final int SENSCORD_ERROR_PERMISSION_DENIED = 4;
        public static final int SENSCORD_ERROR_BUSY = 5;
        public static final int SENSCORD_ERROR_TIMEOUT = 6;
        public static final int SENSCORD_ERROR_CANCELED = 7;
        public static final int SENSCORD_ERROR_ABORTED = 8;
        public static final int SENSCORD_ERROR_ALREADY_EXIT = 9;
        public static final int SENSCORD_ERROR_INVALID_OPERATION = 10;
        public static final int SENSCORD_ERROR_OUT_OF_RANGE = 11;
        public static final int SENSCORD_ERROR_DATA_LOSS = 12;
        public static final int SENSCORD_ERROR_HARDWARE_ERROR = 13;
        public static final int SENSCORD_ERROR_NOT_SUPPORTED = 14;
        public static final int SENSCORD_ERROR_UNKNOWN = 15;
    }

    /**
     * Get error level.
     * 
     * @return error level
     */
    public int GetLevel() {
        return level;
    }

    /**
     * Get error level String value.
     * 
     * @param level error level
     * @return error level
     */
    public String GetLevelString(int level) {

        String ret = "";

        switch (level) {
            case ErrorLevel.SENSCORD_LEVEL_UNDEFINED:
                ret = "UNDEFINED";
                break;
            case ErrorLevel.SENSCORD_LEVEL_FAIL:
                ret = "FAIL";
                break;
            case ErrorLevel.SENSCORD_LEVEL_FATAL:
                ret = "FATAL";
                break;
            default:
                break;
        }
        return ret;
    }

    /**
     * Get error cause.
     * 
     * @return error cause
     */
    public int GetCause() {
        return cause;
    }

    /**
     * Get error cause String value.
     * 
     * @param cause error cause
     * @return error cause String value
     */
    public String GetCauseString(int cause) {
        String ret = "";

        switch (cause) {
            case ErrorCause.SENSCORD_ERROR_NONE:
                ret = "NONE";
                break;
            case ErrorCause.SENSCORD_ERROR_NOT_FOUND:
                ret = "NOT_FOUND";
                break;
            case ErrorCause.SENSCORD_ERROR_INVALID_ARGUMENT:
                ret = "INVALID_ARGUMENT";
                break;
            case ErrorCause.SENSCORD_ERROR_RESOURCE_EXHAUSTED:
                ret = "RESOURCE_EXHAUSTED";
                break;
            case ErrorCause.SENSCORD_ERROR_PERMISSION_DENIED:
                ret = "PERMISSION_DENIED";
                break;
            case ErrorCause.SENSCORD_ERROR_BUSY:
                ret = "BUSY";
                break;
            case ErrorCause.SENSCORD_ERROR_TIMEOUT:
                ret = "TIMEOUT";
                break;
            case ErrorCause.SENSCORD_ERROR_CANCELED:
                ret = "CANCELED";
                break;
            case ErrorCause.SENSCORD_ERROR_ABORTED:
                ret = "ABORTED";
                break;
            case ErrorCause.SENSCORD_ERROR_ALREADY_EXIT:
                ret = "ALREADY_EXIT";
                break;
            case ErrorCause.SENSCORD_ERROR_INVALID_OPERATION:
                ret = "INVALID_OPERATION";
                break;
            case ErrorCause.SENSCORD_ERROR_OUT_OF_RANGE:
                ret = "OUT_OF_RANGE";
                break;
            case ErrorCause.SENSCORD_ERROR_DATA_LOSS:
                ret = "DATA_LOSS";
                break;
            case ErrorCause.SENSCORD_ERROR_HARDWARE_ERROR:
                ret = "HARDWARE_ERROR";
                break;
            case ErrorCause.SENSCORD_ERROR_NOT_SUPPORTED:
                ret = "NOT_SUPPORTED";
                break;
            case ErrorCause.SENSCORD_ERROR_UNKNOWN:
                ret = "UNKNOWN";
                break;
            default:
                break;
        }

        return ret;
    }

    /**
     * Get error message.
     * 
     * @return error message
     */
    public String GetMessase() {
        return message;
    }

    /**
     * Get error block.
     * 
     * @return error block
     */
    public String GetBlock() {
        return block;
    }

    /**
     * Get trace information.
     * 
     * @return trace information
     */
    public String GetTrace() {
        return trace;
    }
}
