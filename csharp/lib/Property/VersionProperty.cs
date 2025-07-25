/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Version information.
    /// </summary>
    [MessagePackObject]
    public class VersionProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "version_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Name.
        /// </summary>
        [Key("name")]
        public string Name { get; set; } = string.Empty;

        /// <summary>
        /// Major version.
        /// </summary>
        [Key("major")]
        public int Major { get; set; }

        /// <summary>
        /// Minor version.
        /// </summary>
        [Key("minor")]
        public int Minor { get; set; }

        /// <summary>
        /// Patch version.
        /// </summary>
        [Key("patch")]
        public int Patch { get; set; }

        /// <summary>
        /// Version description.
        /// </summary>
        [Key("description")]
        public string Description { get; set; } = string.Empty;

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return string.Format(
                $"{0}({1} {2}.{3}.{4} {5})",
                this.Key, this.Name, this.Major, this.Minor, this.Patch, this.Description);
        }
    }
}
