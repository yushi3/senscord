/*
 * SPDX-FileCopyrightText: 2019-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_types;

import com.sun.jna.Structure;

import java.util.Arrays;
import java.util.List;

import senscord_properties.FrameBufferingProperty;

/**
 * Open stream setting.
 * 
 * @see "senscord::OpenStreamSetting"
 */
public class OpenStreamSetting extends Structure {
    public static class ByReference extends OpenStreamSetting implements Structure.ByReference {
    }

    public FrameBufferingProperty buffering;
    public int arguments_count;
    public StreamArgument[] arguments;

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("buffering", "arguments_count", "arguments");
    }

    private final int STREAM_ARGUMENT_SIZE = 32;

    /**
     * Constuctor.
     */
    public OpenStreamSetting() {
        super();

        arguments = new StreamArgument[STREAM_ARGUMENT_SIZE];
        for (int i = 0; i < STREAM_ARGUMENT_SIZE; i++) {
            arguments[i] = new StreamArgument();
        }
    }

    /**
     * Get buffering property.
     * @return buffering property
     */
    public FrameBufferingProperty GetBuffuringProperty() {
        return buffering;
    }

    /**
     * Set buffering property.
     * @param buffering buffering property
     */
    public void SetBuffuringProperty(FrameBufferingProperty buffering) {
        this.buffering = buffering;
    }

    /**
     * Get stream arguments count.
     * @return stream arguments count
     */
    public int GetStreamArgumentsCount() {
        return arguments_count;
    }

    /**
     * Set stream arguments count.
     * @param arguments_count stream arguments count
     */
    public void SetStreamArgumentsCount(int arguments_count) {
        if (arguments_count > STREAM_ARGUMENT_SIZE) {
            throw new IllegalArgumentException("The arguments count exceeds max.");
        }
        this.arguments_count = arguments_count;
    }

    /**
     * Get stream arguments.
     * @return stream arguments
     */
    public StreamArgument[] GetStreamArguments() {
        return arguments;
    }
}
