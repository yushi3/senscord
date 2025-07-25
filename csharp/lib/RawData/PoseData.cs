/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Pose(quaternion) data.
    /// </summary>
    [MessagePackObject]
    public class PoseQuaternionData
    {
        /// <summary>
        /// Raw data type.
        /// </summary>
        [IgnoreMember]
        public string Type { get; } = RawDataType.Pose;

        /// <summary>
        ///  Position (x,y,z).
        /// </summary>
        [Key("position")]
        public Vector3<float> Position { get; set; } = new Vector3<float>();

        /// <summary>
        /// Orientation (x,y,z,w).
        /// </summary>
        [Key("orientation")]
        public Quaternion<float> Orientation { get; set; } = new Quaternion<float>();

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Type}(position={this.Position}, orientation={this.Orientation})";
        }
    }

    /// <summary>
    /// Pose(quaternion) data.
    /// </summary>
    [MessagePackObject]
    public class PoseData : PoseQuaternionData { }

    /// <summary>
    /// Pose(rotation matrix) data.
    /// </summary>
    [MessagePackObject]
    public class PoseMatrixData
    {
        /// <summary>
        /// Raw data type.
        /// </summary>
        [IgnoreMember]
        public string Type { get; } = RawDataType.Pose;

        /// <summary>
        ///  Position (x,y,z).
        /// </summary>
        [Key("position")]
        public Vector3<float> Position { get; set; } = new Vector3<float>();

        /// <summary>
        /// Rotation matrix.
        /// </summary>
        [Key("rotation")]
        public Matrix3x3<float> Rotation { get; set; } = new Matrix3x3<float>();

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Type}(position={this.Position}, orientation={this.Rotation})";
        }
    }
}
