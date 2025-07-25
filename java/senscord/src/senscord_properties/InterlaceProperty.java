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
 * Channel's property for interlace.
 * 
 * @see "senscord::InterlaceProperty"
 */
public class InterlaceProperty extends Structure {
    public int field;

    public static class ByReference extends InterlaceProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("field");
    }

    /**
     * The field types of interlace.
     * 
     * @see "senscord::InterlaceField"
     */
    public static interface Feild {
        /**
         * Top field.
         */
        public static final int SENSCORD_INTERLACE_FIELD_TOP = 0;

        /**
         * Bottom field.
         */
        public static final int SENSCORD_INTERLACE_FIELD_BOTTOM = 1;

    }

    /**
     * Get contained field type.
     * 
     * @return contained field type
     */
    public float GetField() {
        return field;
    }

    /**
     * Set contained field type.
     * 
     * @param field contained field type
     */
    public void SetField(int field) {
        this.field = field;
    }
}
