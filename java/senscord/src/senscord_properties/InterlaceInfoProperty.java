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
 * Property for interlace informations.
 * 
 * @see "senscord::InterlaceInfoProperty"
 */
public class InterlaceInfoProperty extends Structure {
    public int order;

    public static class ByReference extends InterlaceInfoProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("order");
    }

    /**
     * The order of interlace.
     * 
     * @see "senscord::InterlaceOrder"
     */
    public static interface Order {
        /**
         * Top first.
         */
        public static final int SENSCORD_INTERLACE_ORDER_TOP_FIRST = 0;

        /**
         * Bottom first.
         */
        public static final int SENSCORD_INTERLACE_ORDER_BOTTOM_FIRST = 1;

    }

    /**
     * Get interlace order.
     * 
     * @return interlace order
     */
    public float GetOrder() {
        return order;
    }

    /**
     * Set interlace order.
     * 
     * @param order order of field
     */
    public void SetOrder(int order) {
        this.order = order;
    }
}
