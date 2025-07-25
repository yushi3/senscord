/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Structure for the image of the camera exposure.
    /// </summary>
    [MessagePackObject]
    public class ExposureProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "exposure_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Exposure time : Auto.
        /// </summary>
        public static readonly int ExposureTimeAuto = 0;

        /// <summary>
        /// ISO Sensitivity : Auto.
        /// </summary>
        public static readonly int IsoSensitivityAuto = 0;

        /// <summary>
        /// Mode of exposure.
        /// </summary>
        [Key("mode")]
        public string Mode { get; set; } = string.Empty;

        /// <summary>
        /// Compensation value of EV.
        /// </summary>
        [Key("ev_compensation")]
        public float EvCompensation { get; set; }

        /// <summary>
        /// Time of exposure [usec].
        /// </summary>
        [Key("exposure_time")]
        public int ExposureTime { get; set; }

        /// <summary>
        /// ISO sensitivity. (100,200,400,800 ...)
        /// </summary>
        [Key("iso_sensitivity")]
        public int IsoSensitivity { get; set; }

        /// <summary>
        /// Exposure metering mode.
        /// </summary>
        [Key("metering")]
        public string Metering { get; set; } = string.Empty;

        /// <summary>
        /// Target region of the camera exposure.
        /// </summary>
        [Key("target_region")]
        public RectangleRegionParameter TargetRegion { get; set; } =
            new RectangleRegionParameter();

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return string.Format(
                "{0}(left={1} top={2} width={3} height={4})",
                this.Key, this.Mode, this.EvCompensation,
                this.ExposureTime, this.IsoSensitivity,
                this.Metering, this.TargetRegion);
        }
    }

    /// <summary>
    /// Structure for the region of plane for AE or ROI, etc.
    /// </summary>
    [MessagePackObject]
    public class RectangleRegionParameter
    {
        /// <summary>
        /// Upper position of region from origin.
        /// </summary>
        [Key("top")]
        public int Top { get; set; }

        /// <summary>
        /// Left  position of region from origin.
        /// </summary>
        [Key("left")]
        public int Left { get; set; }

        /// <summary>
        /// Bottom position of region from origin.
        /// </summary>
        [Key("bottom")]
        public int Bottom { get; set; }

        /// <summary>
        /// Right position of region from origin.
        /// </summary>
        [Key("right")]
        public int Right { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return string.Format(
                "Top={0} Left={1} Bottom={2} Right={3}",
                this.Top, this.Left, this.Bottom, this.Right);
        }
    }

    /// <summary>
    /// Exposure modes.
    /// </summary>
    public static partial class ExposureMode
    {
        /// <summary>
        /// Auto.
        /// </summary>
        public static readonly string Auto = "auto";

        /// <summary>
        /// Hold.
        /// </summary>
        public static readonly string Hold = "hold";

        /// <summary>
        /// Manual.
        /// </summary>
        public static readonly string Manual = "manual";

        /// <summary>
        /// GainFix.
        /// </summary>
        public static readonly string GainFix = "gainfix";

        /// <summary>
        /// TimeFix.
        /// </summary>
        public static readonly string TimeFix = "timefix";
    }

    /// <summary>
    /// Exposure metering modes.
    /// </summary>
    public static partial class ExposureMetering
    {
        /// <summary>
        /// None.
        /// </summary>
        public static readonly string None = "none";

        /// <summary>
        /// Average.
        /// </summary>
        public static readonly string Average = "average";

        /// <summary>
        /// Center weighted.
        /// </summary>
        public static readonly string CenterWeighted = "center_weighted";

        /// <summary>
        /// Spot.
        /// </summary>
        public static readonly string Spot = "spot";

        /// <summary>
        /// Matrix.
        /// </summary>
        public static readonly string Matrix = "matrix";
    }
}
