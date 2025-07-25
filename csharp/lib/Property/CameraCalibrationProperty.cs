/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using System.Collections.Generic;
using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Property for camera calibration.
    /// </summary>
    [MessagePackObject]
    public class CameraCalibrationProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "camera_calibration_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// List of camera calibration parameters.
        /// </summary>
        [Key("parameters")]
        public Dictionary<long, CameraCalibrationParameters> Parameters { get; set; } =
            new Dictionary<long, CameraCalibrationParameters>();

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Key}({string.Join(", ", this.Parameters)})";
        }
    }

    /// <summary>
    /// Calibration parameters of a single camera.
    /// </summary>
    [MessagePackObject]
    public class CameraCalibrationParameters
    {
        /// <summary>
        /// Camera internal parameters.
        /// </summary>
        [Key("intrinsic")]
        public IntrinsicCalibrationParameter Intrinsic { get; set; } =
            new IntrinsicCalibrationParameter();

        /// <summary>
        /// Distortion correction coefficient.
        /// </summary>
        [Key("distortion")]
        public DistortionCalibrationParameter Distortion { get; set; } =
            new DistortionCalibrationParameter();

        /// <summary>
        /// Extrinsic parameters.
        /// </summary>
        [Key("extrinsic")]
        public ExtrinsicCalibrationParameter Extrinsic { get; set; } =
            new ExtrinsicCalibrationParameter();

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return string.Format(
                "intrinsic={0}, distortion={1}, extrinsic={2}",
                this.Intrinsic, this.Distortion, this.Extrinsic);
        }
    }

    /// <summary>
    /// Structure for handling internal parameters of calibration.
    /// </summary>
    [MessagePackObject]
    public class IntrinsicCalibrationParameter
    {
        /// <summary>
        /// The x-axis coordinate of the optical center point.
        /// </summary>
        [Key("cx")]
        public float cx { get; set; }

        /// <summary>
        /// The y-axis coordinate of the optical center point.
        /// </summary>
        [Key("cy")]
        public float cy { get; set; }

        /// <summary>
        /// Focal length on x axis.
        /// </summary>
        [Key("fx")]
        public float fx { get; set; }

        /// <summary>
        /// Focal length on y axis.
        /// </summary>
        [Key("fy")]
        public float fy { get; set; }

        /// <summary>
        /// skewness.
        /// </summary>
        [Key("s")]
        public float s { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"cx={this.cx}, cy={this.cy}, fx={this.fx}, fy={this.fy}, s={this.s}";
        }
    }

    /// <summary>
    /// Structure for handling extrinsic parameters of calibration.
    /// </summary>
    [MessagePackObject]
    public class ExtrinsicCalibrationParameter
    {
        /// <summary>
        /// Extrinsic parameter r11.
        /// </summary>
        [Key("r11")]
        public float r11 { get; set; }

        /// <summary>
        /// Extrinsic parameter r12.
        /// </summary>
        [Key("r12")]
        public float r12 { get; set; }

        /// <summary>
        /// Extrinsic parameter r13.
        /// </summary>
        [Key("r13")]
        public float r13 { get; set; }

        /// <summary>
        /// Extrinsic parameter r21.
        /// </summary>
        [Key("r21")]
        public float r21 { get; set; }

        /// <summary>
        /// Extrinsic parameter r22.
        /// </summary>
        [Key("r22")]
        public float r22 { get; set; }

        /// <summary>
        /// Extrinsic parameter r23.
        /// </summary>
        [Key("r23")]
        public float r23 { get; set; }

        /// <summary>
        /// Extrinsic parameter r31.
        /// </summary>
        [Key("r31")]
        public float r31 { get; set; }

        /// <summary>
        /// Extrinsic parameter r32.
        /// </summary>
        [Key("r32")]
        public float r32 { get; set; }

        /// <summary>
        /// Extrinsic parameter r33.
        /// </summary>
        [Key("r33")]
        public float r33 { get; set; }

        /// <summary>
        /// Extrinsic parameter t1.
        /// </summary>
        [Key("t1")]
        public float t1 { get; set; }

        /// <summary>
        /// Extrinsic parameter t2.
        /// </summary>
        [Key("t2")]
        public float t2 { get; set; }

        /// <summary>
        /// Extrinsic parameter t3.
        /// </summary>
        [Key("t3")]
        public float t3 { get; set; }

        /// <summary>
        /// Extrinsic parameter p11-p34.
        /// </summary>
        [Key("p")]
        public Matrix3x4<float> p { get; set; } = new Matrix3x4<float>();

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return string.Format(
                "r=[{0}, {1}, {2}], [{3}, {4}, {5}], [{6}, {7}, {8}], " +
                "t=[{9}, {10}, {11}], p=[{12}]",
                this.r11, this.r12, this.r13, this.r21, this.r22, this.r23,
                this.r31, this.r32, this.r33, this.t1, this.t2, this.t3, this.p);
        }
    }

    /// <summary>
    /// Structure for handling camera distortion coefficient.
    /// </summary>
    [MessagePackObject]
    public class DistortionCalibrationParameter
    {
        /// <summary>
        /// Camera distortion coefficient k1.
        /// </summary>
        [Key("k1")]
        public float k1 { get; set; }

        /// <summary>
        /// Camera distortion coefficient k2.
        /// </summary>
        [Key("k2")]
        public float k2 { get; set; }

        /// <summary>
        /// Camera distortion coefficient k3.
        /// </summary>
        [Key("k3")]
        public float k3 { get; set; }

        /// <summary>
        /// Camera distortion coefficient k4.
        /// </summary>
        [Key("k4")]
        public float k4 { get; set; }

        /// <summary>
        /// Camera distortion coefficient k5.
        /// </summary>
        [Key("k5")]
        public float k5 { get; set; }

        /// <summary>
        /// Camera distortion coefficient k6.
        /// </summary>
        [Key("k6")]
        public float k6 { get; set; }

        /// <summary>
        /// Camera distortion coefficient p1.
        /// </summary>
        [Key("p1")]
        public float p1 { get; set; }

        /// <summary>
        /// Camera distortion coefficient p2.
        /// </summary>
        [Key("p2")]
        public float p2 { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return string.Format(
                "k=[{0}, {1}, {2}, {3}, {4}, {5}], p=[{6}, {7}]",
                this.k1, this.k2, this.k3, this.k4, this.k5, this.k6, this.p1, this.p2);
        }
    }
}
