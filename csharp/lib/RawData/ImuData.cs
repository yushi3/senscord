/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Raw data type for Acceleration.
    /// </summary>
    [MessagePackObject]
    public class AccelerationData : Vector3<float>
    {
        /// <summary>
        /// Raw data type.
        /// </summary>
        [IgnoreMember]
        public string Type { get; } = RawDataType.Acceleration;

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Type}({this.x}, {this.y}, {this.z})";
        }
    }

    /// <summary>
    /// Raw data type for angular velocity.
    /// </summary>
    [MessagePackObject]
    public class AngularVelocityData : Vector3<float>
    {
        /// <summary>
        /// Raw data type.
        /// </summary>
        [IgnoreMember]
        public string Type { get; } = RawDataType.AngularVelocity;

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Type}({this.x}, {this.y}, {this.z})";
        }
    }

    /// <summary>
    /// Raw data type for magnetic field.
    /// </summary>
    [MessagePackObject]
    public class MagneticFieldData : Vector3<float>
    {
        /// <summary>
        /// Raw data type.
        /// </summary>
        [IgnoreMember]
        public string Type { get; } = RawDataType.MagneticField;

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Type}({this.x}, {this.y}, {this.z})";
        }
    }
}
