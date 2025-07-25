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
 * Temperature information.
 * @see "senscord::TemperatureInfo"
 */
public class TemperatureInfo extends Structure {
    public int sensor_id;
    public float temperature;
    public String description;

    public static class ByReference extends TemperatureInfo implements Structure.ByReference {
    }

    @Override
    protected List<String> getFieldOrder() {
        return Arrays.asList("sensor_id", "temperature", "description");
    }

    /**
     * Get sensor_id.
     * @return sensor_id
     */
    public int GetSensorId() {
        return sensor_id;
    }

    /**
     * Get temperature.
     * @return temperature
     */
    public float GetTemperature() {
        return temperature;
    }

    /**
     * Get description.
     * @return description
     */
    public String GetDescription() {
        return description;
    }

    /**
     * Set sensor_id.
     * @param sensor_id Sensor ID
     */
    public void SetSensorId(int sensor_id) {
        this.sensor_id = sensor_id;
    }

    /**
     * Set temperature.
     * @param temperature Temperature data
     */
    public void SetTemperature(float temperature) {
        this.temperature = temperature;
    }

    /**
     * Set temperature description.
     * @param description Description of sensor
     */
    public void SetDescription(String description) {
        this.description = description;
    }
}
