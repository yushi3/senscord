/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using System;
using System.Runtime.InteropServices;

namespace SensCord
{
    /// <summary>
    /// Raw data structure.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct RawData
    {
        /// <summary>
        /// Data address.
        /// </summary>
        [MarshalAs(UnmanagedType.SysInt)]
        private IntPtr address;

        /// <summary>
        /// Data length.
        /// </summary>
        [MarshalAs(UnmanagedType.SysInt)]
        private IntPtr size;

        /// <summary>
        /// Data type.
        /// </summary>
        [MarshalAs(UnmanagedType.SysInt)]
        private IntPtr type;

        /// <summary>
        /// Nanoseconds timestamp captured by the device.
        /// </summary>
        [MarshalAs(UnmanagedType.U8)]
        private UInt64 timestamp;

        /// <summary>
        /// Data length.
        /// </summary>
        internal IntPtr Address { get { return this.address; } }

        /// <summary>
        /// Data length.
        /// </summary>
        public int Length { get { return this.size.ToInt32(); } }

        /// <summary>
        /// Data type.
        /// </summary>
        public string Type { get { return Native.PtrToString(this.type); } }

        /// <summary>
        /// Nanoseconds timestamp captured by the device.
        /// </summary>
        public decimal Timestamp { get { return this.timestamp;  } }

        /// <summary>
        /// Return The byte array.
        /// </summary>
        /// <returns>Byte array.</returns>
        public byte[] ToArray()
        {
            var buffer = new byte[this.Length];
            if (buffer.Length > 0)
            {
                Marshal.Copy(this.address, buffer, 0, buffer.Length);
            }
            return buffer;
        }

        /// <summary>
        /// Deserialize raw data.
        /// </summary>
        /// <returns>Deserialized RawData instance.</returns>
        /// <typeparam name="T">RawData class.</typeparam>
        /// <example>
        /// <code><![CDATA[
        ///   var rawData = channel.GetRawData();
        ///   var rotationData = rawData.Deserialize<RotationData>();
        /// ]]></code>
        /// </example>
        public T Deserialize<T>() where T : class, new()
        {
            var buffer = this.ToArray();
            return Serializer.Instance.Unpack<T>(buffer);
        }
    }
}
