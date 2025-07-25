/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Property for obtaining unit of RawData.
    /// </summary>
    [MessagePackObject]
    public class ImuDataUnitProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "imu_data_unit_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Unit of data of acceleration.
        /// </summary>
        [Key("acceleration")]
        public AccelerationUnit Acceleration { get; set; }

        /// <summary>
        /// Unit of data of angular velocity.
        /// </summary>
        [Key("angular_velocity")]
        public AngularVelocityUnit AngularVelocity { get; set; }

        /// <summary>
        /// Unit of data of the magnetic field.
        /// </summary>
        [Key("magnetic_field")]
        public MagneticFieldUnit MagneticField { get; set; }

        /// <summary>
        /// Unit of data of the orientation.
        /// </summary>
        [Key("orientation")]
        public OrientationUnit Orientation { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return string.Format(
                "{0}(accel={1}, angular={2}, magnetic={3}, orientation={4})",
                this.Key, this.Acceleration, this.AngularVelocity, this.MagneticField, this.Orientation);
        }
    }

    /// <summary>
    /// Units used for acceleration.
    /// </summary>
    public enum AccelerationUnit
    {
        /// <summary>
        /// sensor not supported.
        /// </summary>
        NotSupported,

        /// <summary>
        /// Unit:[G].
        /// </summary>
        Gravitational,

        /// <summary>
        /// Unit:[m/s2].
        /// </summary>
        MetrePerSecondSquared,
    }

    /// <summary>
    /// Units used for angular velocity.
    /// </summary>
    public enum AngularVelocityUnit
    {
        /// <summary>
        /// sensor not supported.
        /// </summary>
        NotSupported,

        /// <summary>
        /// Unit:[deg/s].
        /// </summary>
        DegreePerSecond,

        /// <summary>
        /// Unit:[rad/s].
        /// </summary>
        RadianPerSecond,
    }

    /// <summary>
    /// Units used for magnetic field.
    /// </summary>
    public enum MagneticFieldUnit
    {
        /// <summary>
        /// sensor not supported.
        /// </summary>
        NotSupported,

        /// <summary>
        /// Unit:[gauss].
        /// </summary>
        Gauss,

        /// <summary>
        /// Unit:[uT].
        /// </summary>
        MicroTesla,
    }

    /// <summary>
    /// Units used for orientation.
    /// </summary>
    public enum OrientationUnit
    {
        /// <summary>
        /// sensor not supported.
        /// </summary>
        NotSupported,

        /// <summary>
        /// Unit:[deg].
        /// </summary>
        Degree,

        /// <summary>
        /// Unit:[rad].
        /// </summary>
        Radian,
    }
}
