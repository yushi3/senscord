/*
 * SPDX-FileCopyrightText: 2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

/**
 * PlayPauseProperty.
 *
 * @see "senscord::kPlayPausePropertyKey"
 */

public class PlayPauseProperty extends Structure {
    public byte pause;

    public static class ByReference extends PlayPauseProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("pause");
    }

    public PlayPauseProperty() {
        super();
    }

    /**
     * Get pause mode.
     *
     * @return pause
     */
    public int GetPause() {
        return pause;
    }

    /**
     * Set pause mode.
     *
     * @param pause pause mode of playback
     */
    public void SetPause(byte pause) {
        this.pause = pause;
    }
}
