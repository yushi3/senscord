/*
 * SPDX-FileCopyrightText: 2019-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;

import java.util.Arrays;
import java.util.List;

/**
 * Version information.
 * 
 * @see "senscord::VersionProperty"
 */
public class VersionProperty extends Structure {
    public byte[] name;
    public int major;
    public int minor;
    public int patch;
    public byte[] description;

    public static class ByReference extends VersionProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("name", "major", "minor", "patch", "description");
    }

    /**
     * Constructor.
     */
    public VersionProperty() {
        super();
        name = new byte[256];
        description = new byte[256];
    }

    /**
     * Constructor.
     * @param p JNA Pointer.
     */
    public VersionProperty(Pointer p) {
        super();
        name = new byte[256];
        description = new byte[256];
        useMemory(p);
        read();
    }

    /**
     * Get name.
     * @return name.
     */
    public String GetName() {
        return Native.toString(name);
    }

    /**
     * Get major version.
     * @return major version.
     */
    public int GetMajor() {
        return major;
    }

    /**
     * Get minor version.
     * @return minor version.
     */
    public int GetMinor() {
        return minor;
    }

    /**
     * Get patch version.
     * @return patch version.
     */
    public int GetPatch() {
        return patch;
    }

    /**
     * Get version description.
     * @return version description.
     */
    public String GetDescription() {
        return Native.toString(description);
    }

    /**
     * Set name.
     * @param name Maximum length 256
     */
    public void SetName(String name) {
        Arrays.fill(this.name, (byte) 0);
        System.arraycopy(name.getBytes(), 0, this.name, 0, name.length());
    }

    /**
     * Set major version.
     * @param major version
     */
    public void SetMajor(int major) {
        this.major = major;
    }

    /**
     * Set minor version.
     * @param minor version
     */
    public void SetMinor(int minor) {
        this.minor = minor;
    }

    /**
     * Set patch version.
     * @param patch version
     */
    public void SetPatch(int patch) {
        this.patch = patch;
    }

    /**
     * Set description.
     * @param description Maximum length 256
     */
    public void SetDescription(String description) {
        Arrays.fill(this.description, (byte) 0);
        System.arraycopy(description.getBytes(), 0, this.description, 0, description.length());
    }
}
