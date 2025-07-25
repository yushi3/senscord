/*
 * SPDX-FileCopyrightText: 2021-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord_types;

import com.sun.jna.Structure;

import java.util.Arrays;
import java.util.List;

/**
 * Object TrackingData data.
 *
 * @see "senscord::ObjectTrackingData"
 */
public class ObjectTrackingData extends Structure {
    public int count;
    public TrackedObjectInformation.ByReference data;

    public static class ByReference extends ObjectTrackingData implements Structure.ByReference {
    }

    // @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("count", "data");
    }

    public ObjectTrackingData() {
        this.data = new TrackedObjectInformation.ByReference();
    }

    /**
     * Get Object Tracking Information.
     *
     * @return object_tracking_information
     */
    public TrackedObjectInformation[] GetData() {
        TrackedObjectInformation[] retData;
        if ((count == 0) || (data == null)) {
            retData = new TrackedObjectInformation[1];
            retData[0] = new TrackedObjectInformation.ByReference();
        } else {
            retData = (TrackedObjectInformation[]) data.toArray(count);
        }
        return retData;
    }

    /**
     * Get size of Object Tracking Information.
     *
     * @return object_tracking_information
     */
    public int GetCount() {
        return this.count;
    }
}
