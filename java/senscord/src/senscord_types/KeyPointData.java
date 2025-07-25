/*
 * SPDX-FileCopyrightText: 2020-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_types;

import com.sun.jna.Structure;

import java.util.Arrays;
import java.util.List;

/**
 * Key Point data.
 *
 * @see "senscord::KeyPointData"
 */
public class KeyPointData extends Structure {
    public int count;
    public DetectedKeyPointInformation.ByReference data;

    public static class ByReference extends KeyPointData implements Structure.ByReference {
    }

    // @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("count", "data");
    }

    public KeyPointData() {
        this.data = new DetectedKeyPointInformation.ByReference();
    }

    /**
     * Get keyPoint Information.
     *
     * @return key_point_information
     */
    public DetectedKeyPointInformation[] GetData() {
        DetectedKeyPointInformation[] retData;
        if ((count == 0) || (data == null)) {
            retData = new DetectedKeyPointInformation[1];
            retData[0] = new DetectedKeyPointInformation.ByReference();
        } else {
            retData = (DetectedKeyPointInformation[]) data.toArray(count);
        }
        return retData;
    }

    /**
     * Get size of keyPoint Information.
     *
     * @return key_point_information
     */
    public int GetCount() {
        return this.count;
    }
}
