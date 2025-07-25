/*
 * SPDX-FileCopyrightText: 2021-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_types;

import com.sun.jna.Native;
import com.sun.jna.Structure;

import java.util.Arrays;
import java.util.List;

/**
 * Data rate element.
 *
 * @see "senscord::DataRateProperty"
 */
public class DataRateElement extends Structure {
    public float size;
    public byte[] name;
    public byte[] unit;

    public static class ByReference extends DataRateElement implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("size", "name", "unit");
    }

    /**
     * Constructor.
     */
    public DataRateElement() {
        super();
        name = new byte[64];
        unit = new byte[64];
    }

    /**
    * Get data rate size.
    *
    * @return size
    */
    public float GetSize() {
        return size;
    }

    /**
    * Get data rate name.
    *
    * @return name
    */
    public String GetName() {
        return Native.toString(name);
    }

    /**
    * Get data rate unit.
    *
    * @return unit
    */
    public String GetUnit() {
        return Native.toString(unit);
    }

    /**
    * Set data rate size.
    *
    * @param size size
    */
    public void SetSize(float size) {
        this.size = size;
    }

    /**
    * Set data rate name.
    *
    * @param name name
    */
    public void SetName(String name) {
        Arrays.fill(this.name, (byte) 0);
        System.arraycopy(name.getBytes(), 0, this.name, 0, name.length());
    }

    /**
    * Set data rate unit.
    *
    * @param unit unit
    */
    public void SetUnit(String unit) {
        Arrays.fill(this.unit, (byte) 0);
        System.arraycopy(unit.getBytes(), 0, this.unit, 0, unit.length());
    }
}
