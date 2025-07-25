/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using System.Collections.Generic;
using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Property of standard register 64 bit read/write access.
    /// </summary>
    [MessagePackObject]
    public class RegisterAccess64Property : RegisterAccessProperty<ulong>
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "register_access_64_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public override string Key { get; } = ConstKey;
    }

    /// <summary>
    /// Property of standard register 32 bit read/write access.
    /// </summary>
    [MessagePackObject]
    public class RegisterAccess32Property : RegisterAccessProperty<uint>
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "register_access_32_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public override string Key { get; } = ConstKey;
    }

    /// <summary>
    /// Property of standard register 16 bit read/write access.
    /// </summary>
    [MessagePackObject]
    public class RegisterAccess16Property : RegisterAccessProperty<ushort>
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "register_access_16_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public override string Key { get; } = ConstKey;
    }

    /// <summary>
    /// Property of standard register 8 bit read/write access.
    /// </summary>
    [MessagePackObject]
    public class RegisterAccess8Property : RegisterAccessProperty<byte>
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "register_access_8_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public override string Key { get; } = ConstKey;
    }

    /// <summary>
    /// Property of standard register read/write access.
    /// </summary>
    /// <typeparam name="T">Generic type parameter.</typeparam>
    [MessagePackObject]
    public abstract class RegisterAccessProperty<T> : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public abstract string Key { get; }

        /// <summary>
        /// Register ID.
        /// </summary>
        [Key("id")]
        public uint Id { get; set; }

        /// <summary>
        /// RegisterAccessElement array.
        /// </summary>
        [Key("element")]
        public List<RegisterAccessElement<T>> Element { get; set; } =
            new List<RegisterAccessElement<T>>();

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Key}(id={this.Id}, {string.Join(", ", this.Element)})";
        }
    }

    /// <summary>
    /// Information for single register access.
    /// </summary>
    /// <typeparam name="T">Generic type parameter.</typeparam>
    [MessagePackObject]
    public class RegisterAccessElement<T>
    {
        /// <summary>
        /// Target address.
        /// </summary>
        [Key("address")]
        public ulong Address { get; set; }

        /// <summary>
        /// Writing data or read data.
        /// </summary>
        [Key("data")]
        public T Data { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"address={this.Address}, data={this.Data}";
        }
    }
}
