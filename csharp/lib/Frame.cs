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
    /// Frame interface class.
    /// </summary>
    public class Frame : IDisposable
    {
        /// <summary>
        /// Sequence Number.
        /// </summary>
        private readonly UInt64 sequenceNumber;

        /// <summary>
        /// Parent stream.
        /// </summary>
        private Stream stream;

        /// <summary>
        /// Handle of frame.
        /// </summary>
        private UInt64 handle;

        /// <summary>
        /// Frame type.
        /// </summary>
        private string type = null;

        /// <summary>
        /// Channel list.
        /// </summary>
        private Channel[] channels = null;

        /// <summary>
        /// Get the sequential number of this frame.
        /// </summary>
        public decimal SequenceNumber { get { return this.sequenceNumber; } }

        /// <summary>
        /// Get type of this frame.
        /// </summary>
        public string Type
        {
            get { return this.type ?? (this.type = this.GetFrameType()); }
        }

        /// <summary>
        /// Get channel list.
        /// </summary>
        public Channel[] Channels
        {
            get
            {
                return this.channels ?? (this.channels = this.GetChannelList());
            }
        }

        /// <summary>
        /// Create a new Frame class.
        /// </summary>
        /// <param name="stream">Stream instance.</param>
        /// <param name="frameHandle">Frame handle.</param>
        /// <exception cref="ApiException">If an error occurred.</exception>
        internal Frame(Stream stream, UInt64 frameHandle)
        {
            // sequence number.
            if (Native.FrameGetSequenceNumber(
                frameHandle, ref this.sequenceNumber) != 0)
            {
                throw new ApiException(Native.GetLastError());
            }

            this.stream = stream;
            this.handle = frameHandle;

            this.stream.Register(this);
        }

        /// <summary>
        /// Releases unmanaged resources before clearing by garbage collection.
        /// </summary>
        ~Frame()
        {
            this.Dispose(false);
        }

        /// <summary>
        /// Releases resources of the Frame class.
        /// </summary>
        public void Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Releases resources of the Frame class.
        /// </summary>
        /// <param name="disposing">Dispose managed state.</param>
        protected virtual void Dispose(bool disposing)
        {
            if (this.handle != 0)
            {
                if (disposing)
                {
                    // release the managed objects.
                }

                // release the unmanaged objects.
                bool used = false;
                if (this.channels != null)
                {
                    foreach (var channel in this.channels)
                    {
                        if (channel.IsUsed)
                        {
                            used = true;
                            break;
                        }
                    }
                }

                int ret = 0;
                if (used)
                {
                    ret = Native.StreamReleaseFrame(
                        this.stream.Handle, this.handle);
                }
                else
                {
                    ret = Native.StreamReleaseFrameUnused(
                        this.stream.Handle, this.handle);
                }
                if (ret != 0)
                {
                    throw new ApiException(Native.GetLastError());
                }
                this.stream.Unregister(this);
                this.stream = null;
                this.handle = 0;
            }
        }

        /// <summary>
        /// Get the user data.
        /// </summary>
        /// <returns>User data byte array.</returns>
        /// <exception cref="ApiException">If an error occurred.</exception>
        public byte[] GetUserData()
        {
            var userData = new UserData();
            if (Native.FrameGetUserData(this.handle, ref userData) != 0)
            {
                throw new ApiException(Native.GetLastError());
            }
            return userData.ToArray();
        }

        /// <summary>
        /// User data information.
        /// </summary>
        [StructLayout(LayoutKind.Sequential)]
        internal struct UserData
        {
            /// <summary>
            /// Data address.
            /// </summary>
            [MarshalAs(UnmanagedType.SysInt)]
            private IntPtr address;

            /// <summary>
            /// Data size.
            /// </summary>
            [MarshalAs(UnmanagedType.SysInt)]
            private IntPtr size;

            /// <summary>
            /// Data length.
            /// </summary>
            public int Length { get { return this.size.ToInt32(); } }

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
        }

        /// <summary>
        /// Get type of this frame.
        /// </summary>
        /// <returns>Type of frame.</returns>
        private string GetFrameType()
        {
            IntPtr typePtr = IntPtr.Zero;
            if (Native.FrameGetType(this.handle, ref typePtr) != 0)
            {
                throw new ApiException(Native.GetLastError());
            }
            return Native.PtrToString(typePtr);
        }

        /// <summary>
        /// Get channel list.
        /// </summary>
        /// <returns>Channel list.</returns>
        private Channel[] GetChannelList()
        {
            UInt32 count = 0;
            if (Native.FrameGetChannelCount(this.handle, ref count) != 0)
            {
                throw new ApiException(Native.GetLastError());
            }

            var list = new List<Channel>();
            for (UInt32 index = 0; index < count; ++index)
            {
                UInt64 channelHandle = 0;
                if (Native.FrameGetChannel(
                    this.handle, index, ref channelHandle) != 0)
                {
                    throw new ApiException(Native.GetLastError());
                }
                list.Add(new Channel(channelHandle));
            }
            return list.ToArray();
        }
    }
}
