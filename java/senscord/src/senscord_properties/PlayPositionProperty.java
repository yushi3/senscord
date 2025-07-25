/*
 * SPDX-FileCopyrightText: 2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;

/**
 * PlayPositionProperty.
 *
 * @see "senscord::kPlayPositionPropertyKey"
 */

public class PlayPositionProperty extends Structure {
    public int position;

    public static class ByReference extends PlayPositionProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("position");
    }

    public PlayPositionProperty() {
        super();
    }

    /**
     * Get playback position.
     *
     * @return position
     */
    public int GetPosition() {
        return position;
    }

    /**
     * Set playback position.
     *
     * @param position Playback position
     */
    public void SetPosition(int position) {
        this.position = position;
    }
}
