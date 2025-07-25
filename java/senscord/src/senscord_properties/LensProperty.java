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
 * Acquire field angle of camera.
 * 
 * @see "senscord::LensProperty"
 */

public class LensProperty extends Structure {
    public float horizontal_field_of_view;
    public float vertical_field_of_view;

    public static class ByReference extends LensProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("horizontal_field_of_view", "vertical_field_of_view");
    }

    /**
     * Get horizontal viewing angle of lens.
     * 
     * @return horizontal viewing angle of lends
     */
    public float GetHorizontal() {
        return horizontal_field_of_view;
    }


    /**
     * Get vertical viewing angle of lens.
     * 
     * @return vertical viewing angle of lens
     */
    public float GetVertical() {
        return vertical_field_of_view;
    }

    /**
     * Set horizontal viewing angle of lens.
     * 
     * @param horizontal_field_of_view horizontal viewing of lens
     */
    public void SetHorizontal(float horizontal_field_of_view) {
        this.horizontal_field_of_view = horizontal_field_of_view;
    }


    /**
     * Set vertical viewing angle of lens.
     * 
     * @param vertical_field_of_view The vertical viewing angle of the lens
     */
    public void SetVertical(float vertical_field_of_view) {
        this.vertical_field_of_view = vertical_field_of_view;
    }
}
