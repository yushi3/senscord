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
 * PlayModeProperty.
 *
 * @see "senscord::kPlayModePropertyKey"
 */

public class PlayModeProperty extends Structure {
    public byte repeat;

    public static class ByReference extends PlayModeProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("repeat");
    }

    public PlayModeProperty() {
        super();
    }

    /**
     * Get repeat mode.
     *
     * @return repeat
     */
    public int GetRepeat() {
        return repeat;
    }

    /**
     * Set repeat mode.
     *
     * @param repeat repeat mode of playback
     */
    public void SetRepeat(byte repeat) {
        this.repeat = repeat;
    }

}
