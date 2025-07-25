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
 * Object Detection Data.
 *
 * @see "senscord::ObjectDetectionData"
 */
public class ObjectDetectionData extends Structure {

    public static class ByReference extends ObjectDetectionData implements Structure.ByReference {
    }

    public int count;
    public DetectedObjectInformation.ByReference data;

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("count", "data");
    }

    public ObjectDetectionData() {
        this.data = new DetectedObjectInformation.ByReference();
    }

    /**
     * Get data.
     *
     * @return data
     */
    public DetectedObjectInformation[] GetData() {
        DetectedObjectInformation[] retData;
        if ((count == 0) || (data == null)) {
            retData = new DetectedObjectInformation[1];
            retData[0] = new DetectedObjectInformation();
            retData[0].class_id = 0;
            retData[0].score = 0;
            retData[0].box.bottom = 0;
            retData[0].box.left = 0;
            retData[0].box.right = 0;
            retData[0].box.top = 0;
        } else {
            retData = (DetectedObjectInformation[]) data.toArray(count);
        }
        return retData;
    }

    /**
     * Get size of data.
     *
     * @return size
     */
    public int GetCount() {
        return this.count;
    }
}
