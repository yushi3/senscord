/*
 * SPDX-FileCopyrightText: 2019-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_properties;

import com.sun.jna.Native;
import com.sun.jna.Structure;

import java.util.Arrays;
import java.util.List;

import senscord_types.RecordInfo;
import senscord_types.RecordNameRule;

/**
 * RecordProperty.
 * 
 * @see "senscord::kRecordPropertyKey"
 */

public class RecordProperty extends Structure {
    public byte enabled;
    public byte[] path;
    public int count;
    public int info_count;
    public RecordInfo[] info_array;
    public int buffer_num;
    public int name_rules_count;
    public RecordNameRule[] name_rules;

    public static class ByReference extends RecordProperty implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList(
            "enabled", "path", "count", "info_count", "info_array",
            "buffer_num", "name_rules_count", "name_rules");
    }

    /**
     * Constructor.
     */
    public RecordProperty() {
        super();
        path = new byte[256];
        int RECORD_INFO_SIZE = 128;
        info_array = new RecordInfo[RECORD_INFO_SIZE];
        for (int i = 0; i < RECORD_INFO_SIZE; i++) {
            info_array[i] = new RecordInfo();
        }

        int NAME_RULES_SIZE = 8;
        name_rules = new RecordNameRule[NAME_RULES_SIZE];
        for (int i = 0; i < NAME_RULES_SIZE; ++i) {
            name_rules[i] = new RecordNameRule();
        }
    }

    /**
     * Get enabled flag.
     * 
     * @return enabled flag
     */
    public int GetEnabled() {
        return enabled;
    }

    /**
     * Set enabled flag.
     * 
     * @param enabled flag
     */
    public void SetEnabled(byte enabled) {
        this.enabled = enabled;
    }

    /**
     * Get file path.
     * 
     * @return file path
     */
    public String GetPath() {
        return Native.toString(path);
    }

    /**
     * Set file path.
     * 
     * @param path file path
     */
    public void SetPath(String path) {
        Arrays.fill(this.path, (byte) 0);
        System.arraycopy(path.getBytes(), 0, this.path, 0, path.length());
    }

    /**
     * Get the count of record frames.
     * 
     * @return The count of record frames
     */
    public int GetCount() {
        return count;
    }

    /**
     * Set the count of record frames.
     * 
     * @param count record frame count
     */
    public void SetCount(int count) {
        this.count = count;
    }

    /**
     * Get info count.
     * 
     * @return info_count
     */
    public int GetInfoCount() {
        return info_count;
    }

    /**
     * Set info count.
     * 
     * @param info_count info count
     */
    public void SetInfoCount(int info_count) {
        this.info_count = info_count;
    }

    /**
     * Get RecordInfo.
     * 
     * @return RecordInfo
     */
    public RecordInfo[] GetRecordInfo() {
        return info_array;
    }

    /**
     * Get buffer num.
     * 
     * @return buffer num
     */
    public int GetBufferNum() {
        return buffer_num;
    }

    /**
     * Set buffer num.
     * 
     * @param num buffer num
     */
    public void SetBufferNum(int num) {
        this.buffer_num = num;
    }

    /**
     * Get name rules count.
     * 
     * @return name_rules_count
     */
    public int GetNameRulesCount() {
        return name_rules_count;
    }

    /**
     * Set name rules count.
     * 
     * @param name_rules_count name rules count
     */
    public void SetNameRulesCount(int name_rules_count) {
        this.name_rules_count = name_rules_count;
    }

    /**
     * Get NameRules.
     * 
     * @return RecordNameRule
     */
    public RecordNameRule[] GetNameRules() {
        return name_rules;
    }
}
