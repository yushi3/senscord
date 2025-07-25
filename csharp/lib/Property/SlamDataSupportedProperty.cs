/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Data format supported by SLAM stream.
    /// </summary>
    [MessagePackObject]
    public class SlamDataSupportedProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "slam_data_supported_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Odometry supported.
        /// </summary>
        [Key("odometry_supported")]
        public bool OdometrySupported { get; set; }

        /// <summary>
        /// GridMap supported.
        /// </summary>
        [Key("gridmap_supported")]
        public bool GridMapSupported { get; set; }

        /// <summary>
        /// PointCloud supported.
        /// </summary>
        [Key("pointcloud_supported")]
        public bool PointCloudSupported { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return string.Format(
                "{0}(odometry={1}, gridmap={2}, pointcloud={3})",
                this.Key, this.OdometrySupported, this.GridMapSupported, this.PointCloudSupported);
        }
    }
}
