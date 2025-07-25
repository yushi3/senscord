/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace SensCord
{
    /// <summary>
    /// Channel of frame.
    /// </summary>
    public class Channel
    {
        /// <summary>
        /// Handle of channel.
        /// </summary>
        private readonly UInt64 channelHandle;

        /// <summary>
        /// Channel ID.
        /// </summary>
        private readonly UInt32 channelId;

        /// <summary>
        /// Property key List.
        /// </summary>
        private string[] propertyList = null;

        /// <summary>
        /// Updated property key List.
        /// </summary>
        private string[] updatedPropertyList = null;

        /// <summary>
        /// Get the channel ID.
        /// </summary>
        public long Id { get { return this.channelId; } }

        /// <summary>
        /// Get the stored property key list on this channel.
        /// </summary>
        public string[] PropertyList
        {
            get
            {
                return this.propertyList ?? (
                    this.propertyList = this.GetPropertyList());
            }
        }

        /// <summary>
        /// Get the updated property key list on this channel.
        /// </summary>
        public string[] UpdatedPropertyList
        {
            get
            {
                return this.updatedPropertyList ?? (
                    this.updatedPropertyList = this.GetUpdatedPropertyList());
            }
        }

        /// <summary>
        /// Whether RawData is used.
        /// </summary>
        internal bool IsUsed { get; private set; }

        /// <summary>
        /// Create a new Channel class.
        /// </summary>
        /// <param name="channelHandle">Channel handle.</param>
        /// <exception cref="ApiException">If an error occurred.</exception>
        internal Channel(UInt64 channelHandle)
        {
            // channel id.
            if (Native.ChannelGetChannelId(
                channelHandle, ref this.channelId) != 0)
            {
                throw new ApiException(Native.GetLastError());
            }

            this.channelHandle = channelHandle;
            this.IsUsed = false;
        }

        /// <summary>
        /// Get the raw data structure.
        /// </summary>
        /// <returns>Raw data structure.</returns>
        /// <exception cref="ApiException">If an error occurred.</exception>
        public RawData GetRawData()
        {
            var rawData = new RawData();
            if (Native.ChannelGetRawData(
                this.channelHandle, ref rawData) != 0)
            {
                throw new ApiException(Native.GetLastError());
            }
            this.IsUsed = true;
            return rawData;
        }

        /// <summary>
        /// Get the property related to this raw data.
        /// </summary>
        /// <typeparam name="T">Type of property class.</typeparam>
        /// <param name="propertyKey">Key of property to get.</param>
        /// <param name="value">Class to store the property values.</param>
        /// <param name="tempSize">Size of temporary buffer.</param>
        /// <exception cref="ApiException">If an error occurred.</exception>
        public void GetProperty<T>(
            string propertyKey, ref T value,
            int tempSize = 1024) where T : class
        {
            var bytes = new byte[tempSize];
            var handle = GCHandle.Alloc(bytes, GCHandleType.Pinned);
            var handlePtr = handle.AddrOfPinnedObject();
            var bufferSize = new UIntPtr((uint)bytes.Length);
            var outputSize = new UIntPtr();
            try
            {
                if (Native.ChannelGetSerializedProperty(
                    this.channelHandle, propertyKey, handlePtr, bufferSize,
                    ref outputSize) != 0)
                {
                    throw new ApiException(Native.GetLastError());
                }
            }
            finally
            {
                handle.Free();
            }
            value = Serializer.Instance.Unpack<T>(bytes);
        }

        /// <summary>
        /// Get the property related to this raw data.
        /// </summary>
        /// <typeparam name="T">Type derived from BaseProperty class.</typeparam>
        /// <param name="value">Class to store the property values.</param>
        /// <exception cref="ApiException">If an error occurred.</exception>
        public void GetProperty<T>(ref T value) where T : class, IBaseProperty
        {
            this.GetProperty(value.Key, ref value);
        }

        /// <summary>
        /// Get the property related to this raw data.
        /// </summary>
        /// <typeparam name="T">Type derived from BaseProperty class.</typeparam>
        /// <returns>Property values.</returns>
        /// <exception cref="ApiException">If an error occurred.</exception>
        public T GetProperty<T>() where T : class, IBaseProperty, new()
        {
            var value = new T();
            this.GetProperty(ref value);
            return value;
        }

        /// <summary>
        /// Get the stored property key list on this channel.
        /// </summary>
        /// <returns>Property key list.</returns>
        /// <exception cref="ApiException">If an error occurred.</exception>
        private string[] GetPropertyList()
        {
            UInt32 count = 0;
            if (Native.ChannelGetPropertyCount(
                this.channelHandle, ref count) != 0)
            {
                throw new ApiException(Native.GetLastError());
            }

            var list = new List<string>();
            for (UInt32 index = 0; index < count; ++index)
            {
                IntPtr keyPtr = IntPtr.Zero;
                if (Native.ChannelGetPropertyKey(
                    this.channelHandle, index, ref keyPtr) != 0)
                {
                    throw new ApiException(Native.GetLastError());
                }
                list.Add(Native.PtrToString(keyPtr));
            }
            return list.ToArray();
        }

        /// <summary>
        /// Get the updated property key list on this channel.
        /// </summary>
        /// <returns>Updated property key list.</returns>
        /// <exception cref="ApiException">If an error occurred.</exception>
        private string[] GetUpdatedPropertyList()
        {
            UInt32 count = 0;
            if (Native.ChannelGetUpdatedPropertyCount(
                this.channelHandle, ref count) != 0)
            {
                throw new ApiException(Native.GetLastError());
            }

            var list = new List<string>();
            for (UInt32 index = 0; index < count; ++index)
            {
                IntPtr keyPtr = IntPtr.Zero;
                if (Native.ChannelGetUpdatedPropertyKey(
                    this.channelHandle, index, ref keyPtr) != 0)
                {
                    throw new ApiException(Native.GetLastError());
                }
                list.Add(Native.PtrToString(keyPtr));
            }
            return list.ToArray();
        }
    }
}
