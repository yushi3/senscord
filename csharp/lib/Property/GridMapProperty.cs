/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Grid Data Property.
    /// </summary>
    [MessagePackObject]
    public class GridMapProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "grid_map_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// number of x axis grids in grid map data.
        /// </summary>
        [Key("grid_num_x")]
        public int GridNumX { get; set; }

        /// <summary>
        /// number of y axis grids in grid map data.
        /// </summary>
        [Key("grid_num_y")]
        public int GridNumY { get; set; }

        /// <summary>
        /// number of z axis grids in grid map data.
        /// </summary>
        [Key("grid_num_z")]
        public int GridNumZ { get; set; }

        /// <summary>
        /// format of the grid map.
        /// </summary>
        [Key("pixel_format")]
        public string PixelFormat { get; set; } = string.Empty;

        /// <summary>
        /// size of grids.
        /// </summary>
        [Key("grid_size")]
        public GridSize GridSize { get; set; } = new GridSize();

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return string.Format(
                "{0}(grid_num_x={1}, grid_num_y={2}, grid_num_z={3}, pixel_format={4}, {5})",
                this.Key, this.GridNumX, this.GridNumY, this.GridNumZ, this.PixelFormat, this.GridSize);
        }
    }

    /// <summary>
    /// Grid size.
    /// </summary>
    [MessagePackObject]
    public class GridSize
    {
        /// <summary>
        /// grid size of x axis.
        /// </summary>
        [Key("x")]
        public float x { get; set; }

        /// <summary>
        /// grid size of y axis.
        /// </summary>
        [Key("y")]
        public float y { get; set; }

        /// <summary>
        /// grid size of z axis.
        /// </summary>
        [Key("z")]
        public float z { get; set; }

        /// <summary>
        /// unit of x grid.
        /// </summary>
        [Key("unit")]
        public GridUnit unit { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"GridSize({this.x}, {this.y}, {this.z}, {this.unit})";
        }
    }

    /// <summary>
    /// Grid size property.
    /// </summary>
    [MessagePackObject]
    public class GridSizeProperty : GridSize, IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "grid_size_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;
    }

    /// <summary>
    /// units of grid.
    /// </summary>
    public enum GridUnit
    {
        /// <summary>
        /// pixel.
        /// </summary>
        Pixel,

        /// <summary>
        /// meter.
        /// </summary>
        Meter,
    }
}
