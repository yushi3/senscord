/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Threading;

namespace SensCord
{
    /// <summary>
    /// Frame receive handler.
    /// </summary>
    /// <param name="stream">Stream instance.</param>
    public delegate void FrameReceiveHandler(Stream stream);

    /// <summary>
    /// Event receive handler.
    /// </summary>
    /// <param name="stream">Stream instance.</param>
    /// <param name="eventType">Event type.</param>
    public delegate void EventReceiveHandler(Stream stream, string eventType);

    /// <summary>
    /// Event receive handler.
    /// </summary>
    /// <param name="stream">Stream instance.</param>
    /// <param name="eventType">Event type.</param>
    /// <param name="args">Event argument.</param>
    public delegate void EventReceiveHandler2(
        Stream stream, string eventType, EventArgument args);

    /// <summary>
    /// Property lock resource class.
    /// </summary>
    public class PropertyLockResource : IDisposable
    {
        /// <summary>
        /// Stream instance.
        /// </summary>
        private Stream stream;

        /// <summary>
        /// Property lock handle.
        /// </summary>
        internal UInt64 Handle { get; private set; }

        /// <summary>
        /// Create a new PropertyLockResource instance.
        /// </summary>
        /// <param name="stream">Stream instance.</param>
        /// <param name="handle">Property lock handle.</param>
        internal PropertyLockResource(Stream stream, UInt64 handle)
        {
            this.stream = stream;
            this.Handle = handle;
        }

        /// <summary>
        /// Destructor.
        /// </summary>
        ~PropertyLockResource()
        {
            this.Dispose(false);
        }

        /// <summary>
        /// Releases unmanaged resources before clearing by garbage collection.
        /// </summary>
        public void Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Releases resources of the PropertyLockResource class.
        /// </summary>
        /// <param name="disposing">Dispose managed state.</param>
        protected virtual void Dispose(bool disposing)
        {
            if (this.Handle != 0)
            {
                if (disposing)
                {
                    // release the managed objects.
                }

                // release the unmanaged objects.
                if (Native.StreamUnlockProperty(
                    this.stream.Handle, this.Handle) != 0)
                {
                    throw new ApiException(Native.GetLastError());
                }
                this.stream = null;
                this.Handle = 0;
            }
        }
    }

    /// <summary>
    /// Stream interface class.
    /// </summary>
    public class Stream : IDisposable
    {
        /// <summary>
        /// Stream handle.
        /// </summary>
        internal UInt64 Handle { get; private set; }

        /// <summary>
        /// Stream key.
        /// </summary>
        public string Key { get; private set; }

        /// <summary>
        /// Returns true if the stream's start is active.
        /// </summary>
        public bool IsStarted { get; private set; }

        /// <summary>
        /// Core handle.
        /// </summary>
        private Core core;

        /// <summary>
        /// Frame receive handler.
        /// </summary>
        private event FrameReceiveHandler FrameReceiveHandler;

        /// <summary>
        /// Event receive handler (old).
        /// </summary>
        private event EventReceiveHandler EventReceiveHandlerOld;

        /// <summary>
        /// Frame callback.
        /// </summary>
        private readonly Native.FrameCallback frameCallbackDelegate;

        /// <summary>
        /// Event callback.
        /// </summary>
        private readonly Native.EventCallback eventCallbackDelegate;

        /// <summary>
        /// Event callback (old).
        /// </summary>
        private readonly Native.EventCallbackOld eventCallbackDelegateOld;

        /// <summary>
        /// Event receive handler.
        /// </summary>
        private ConcurrentDictionary<string, EventReceiveHandler2> eventReceiveHandler =
            new ConcurrentDictionary<string, EventReceiveHandler2>();

        /// <summary>
        /// Frame dispose.
        /// </summary>
        private Action frameDispose;

        /// <summary>
        /// Create a new Stream instance.
        /// </summary>
        /// <param name="core">Core instance.</param>
        /// <param name="streamHandle">Stream handle.</param>
        /// <param name="streamKey">Stream key.</param>
        internal Stream(Core core, UInt64 streamHandle, string streamKey)
        {
            this.core = core;
            this.Handle = streamHandle;
            this.Key = streamKey;
            this.IsStarted = false;

            this.frameCallbackDelegate = new Native.FrameCallback(this.FrameDispatcher);
            this.eventCallbackDelegate = new Native.EventCallback(this.EventDispatcher);
            this.eventCallbackDelegateOld = new Native.EventCallbackOld(this.EventDispatcherOld);

            this.core.Register(this);
        }

        /// <summary>
        /// Releases unmanaged resources before clearing by garbage collection.
        /// </summary>
        ~Stream()
        {
            this.Dispose(false);
        }

        /// <summary>
        /// Releases resources of the Stream class.
        /// </summary>
        public void Dispose()
        {
            this.Dispose(true);
            GC.SuppressFinalize(this);
        }

        /// <summary>
        /// Releases resources of the Core class.
        /// </summary>
        /// <param name="disposing">Dispose managed state.</param>
        protected virtual void Dispose(bool disposing)
        {
            if (this.Handle != 0)
            {
                if (disposing)
                {
                    // release the managed objects.
                }

                // release the unmanaged objects.
                Interlocked.Exchange(
                    ref this.frameDispose, default(Action))?.Invoke();
                if (Native.CoreCloseStream(
                    this.core.Handle, this.Handle) != 0)
                {
                    throw new ApiException(Native.GetLastError());
                }
                this.core.Unregister(this);
                this.core = null;
                this.Handle = 0;
            }
        }

        /// <summary>
        /// Start this stream.
        /// </summary>
        /// <exception cref="ApiException">If an error occurred.</exception>
        public void Start()
        {
            if (Native.StreamStart(this.Handle) != 0)
            {
                throw new ApiException(Native.GetLastError());
            }
            this.IsStarted = true;
        }

        /// <summary>
        /// Stop this stream.
        /// </summary>
        /// <exception cref="ApiException">If an error occurred.</exception>
        public void Stop()
        {
            if (this.IsStarted)
            {
                if (Native.StreamStop(this.Handle) != 0)
                {
                    throw new ApiException(Native.GetLastError());
                }
                this.IsStarted = false;
            }
        }

        /// <summary>
        /// Get the received frame.
        /// </summary>
        /// <param name="timeoutMsec">Time of wait msec if no received.
        /// 0 is polling, minus is forever.</param>
        /// <returns>Frame instance.</returns>
        /// <exception cref="ApiException">If an error occurred.</exception>
        public Frame GetFrame(int timeoutMsec = -1)
        {
            UInt64 frameHandle = 0;
            if (Native.StreamGetFrame(
                this.Handle, ref frameHandle, timeoutMsec) != 0)
            {
                throw new ApiException(Native.GetLastError());
            }
            return new Frame(this, frameHandle);
        }

        /// <summary>
        /// Clear frames have not gotten.
        /// </summary>
        /// <returns>number of cleared frames.</returns>
        /// <exception cref="ApiException">If an error occurred.</exception>
        public int ClearFrames()
        {
            int count = 0;
            if (Native.StreamClearFrames(this.Handle, ref count) != 0)
            {
                throw new ApiException(Native.GetLastError());
            }
            return count;
        }

        /// <summary>
        /// Get the property.
        /// </summary>
        /// <typeparam name="T">Any class.</typeparam>
        /// <param name="propertyKey">Key of property to get.</param>
        /// <param name="value">Acquired property values are stored.</param>
        /// <param name="tempSize">Size of temporary buffer.</param>
        /// <exception cref="ApiException">If an error occurred.</exception>
        public void GetProperty<T>(
            string propertyKey, ref T value,
            int tempSize = 1024) where T : class
        {
            var bytes = Serializer.Instance.Pack(value, tempSize);
            var handle = GCHandle.Alloc(bytes, GCHandleType.Pinned);
            var handlePtr = handle.AddrOfPinnedObject();
            var bufferSize = new UIntPtr((uint)bytes.Length);
            var outputSize = new UIntPtr();
            try
            {
                if (Native.StreamGetSerializedProperty(
                    this.Handle, propertyKey, handlePtr, bufferSize,
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
        /// Get the property.
        /// </summary>
        /// <typeparam name="T">Type derived from BaseProperty class.</typeparam>
        /// <param name="value">Acquired property values are stored.</param>
        /// <exception cref="ApiException">If an error occurred.</exception>
        public void GetProperty<T>(ref T value) where T : class, IBaseProperty
        {
            this.GetProperty(value.Key, ref value);
        }

        /// <summary>
        /// Get the property.
        /// </summary>
        /// <typeparam name="T">Type derived from BaseProperty class.</typeparam>
        /// <returns>Acquired property values.</returns>
        /// <exception cref="ApiException">If an error occurred.</exception>
        public T GetProperty<T>() where T : class, IBaseProperty, new()
        {
            var value = new T();
            this.GetProperty(ref value);
            return value;
        }

        /// <summary>
        /// Set the property with key.
        /// </summary>
        /// <typeparam name="T">Any class.</typeparam>
        /// <param name="propertyKey">Key of property to set.</param>
        /// <param name="value">Property value to set.</param>
        /// <exception cref="ApiException">If an error occurred.</exception>
        public void SetProperty<T>(string propertyKey, T value) where T : class
        {
            var bytes = Serializer.Instance.Pack(value);
            var handle = GCHandle.Alloc(bytes, GCHandleType.Pinned);
            var handlePtr = handle.AddrOfPinnedObject();
            var bufferSize = new UIntPtr((uint)bytes.Length);
            try
            {
                if (Native.StreamSetSerializedProperty(
                    this.Handle, propertyKey, handlePtr, bufferSize) != 0)
                {
                    throw new ApiException(Native.GetLastError());
                }
            }
            finally
            {
                handle.Free();
            }
        }

        /// <summary>
        /// Set the property.
        /// </summary>
        /// <typeparam name="T">Type derived from BaseProperty class.</typeparam>
        /// <param name="value">Property value to set.</param>
        /// <exception cref="ApiException">If an error occurred.</exception>
        public void SetProperty<T>(T value) where T : class, IBaseProperty
        {
            this.SetProperty(value.Key, value);
        }

        /// <summary>
        /// Get the supported property key list on this stream.
        /// </summary>
        /// <returns>Supported property key list.</returns>
        /// <exception cref="ApiException">If an error occurred.</exception>
        public string[] GetPropertyList()
        {
            UInt32 count = 0;
            if (Native.StreamGetPropertyCount(this.Handle, ref count) != 0)
            {
                throw new ApiException(Native.GetLastError());
            }

            var list = new List<string>();
            for (UInt32 index = 0; index < count; ++index)
            {
                IntPtr keyPtr = IntPtr.Zero;
                if (Native.StreamGetPropertyKey(
                    this.Handle, index, ref keyPtr) != 0)
                {
                    throw new ApiException(Native.GetLastError());
                }
                list.Add(Native.PtrToString(keyPtr));
            }
            return list.ToArray();
        }

        /// <summary>
        /// Lock to access properties.
        /// </summary>
        /// <param name="timeoutMsec">Time of wait msec if locked already.
        /// 0 is polling, minus is forever.</param>
        /// <exception cref="ApiException">If an error occurred.</exception>
        public void LockProperty(int timeoutMsec = -1)
        {
            if (Native.StreamLockProperty(this.Handle, timeoutMsec) != 0)
            {
                throw new ApiException(Native.GetLastError());
            }
        }

        /// <summary>
        /// Lock to access properties (specified keys).
        /// </summary>
        /// <param name="keys">Property key to lock.</param>
        /// <param name="timeoutMsec">Time of wait msec if locked already.
        /// 0 is polling, minus is forever.</param>
        /// <returns>Locked properties resource.</returns>
        /// <exception cref="ApiException">If an error occurred.</exception>
        public PropertyLockResource LockProperty(
            string[] keys, int timeoutMsec = -1)
        {
            UInt64 resource = 0;
            if (Native.StreamLockProperty(
                this.Handle, keys, (UInt32)keys.Length,
                timeoutMsec, ref resource) != 0)
            {
                throw new ApiException(Native.GetLastError());
            }
            PropertyLockResource resource_wrapper =
                new PropertyLockResource(this, resource);
            return resource_wrapper;
        }

        /// <summary>
        /// Unlock to access properties.
        /// </summary>
        /// <exception cref="ApiException">If an error occurred.</exception>
        public void UnlockProperty()
        {
            if (Native.StreamUnlockProperty(this.Handle) != 0)
            {
                throw new ApiException(Native.GetLastError());
            }
        }

        /// <summary>
        /// Unlock to access properties (specified resource).
        /// </summary>
        /// <param name="resource">Resource handle.</param>
        /// <exception cref="ApiException">If an error occurred.</exception>
        public void UnlockProperty(PropertyLockResource resource)
        {
            if (Native.StreamUnlockProperty(this.Handle, resource.Handle) != 0)
            {
                throw new ApiException(Native.GetLastError());
            }
        }

        /// <summary>
        /// Callback function called from the C API when receiving frame.
        /// </summary>
        /// <param name="stream">Stream handler.</param>
        /// <param name="reserved">Reserved.</param>
        private void FrameDispatcher(UInt64 stream, IntPtr reserved)
        {
            this.FrameReceiveHandler?.Invoke(this);
        }

        /// <summary>
        /// Register the frame receive handler.
        /// </summary>
        /// <param name="receiver">Frame receive handler.</param>
        /// <exception cref="ApiException">If an error occurred.</exception>
        /// <exception cref="ArgumentNullException">Argument is Null.</exception>
        public void RegisterFrameCallback(FrameReceiveHandler receiver)
        {
            if (receiver == null)
            {
                throw new ArgumentNullException(nameof(receiver));
            }

            if (this.FrameReceiveHandler == null)
            {
                IntPtr reserved = IntPtr.Zero;
                if (Native.StreamRegisterFrameCallback(
                    this.Handle, this.frameCallbackDelegate, reserved) != 0)
                {
                    throw new ApiException(Native.GetLastError());
                }
            }
            this.FrameReceiveHandler += receiver;
        }

        /// <summary>
        /// Unregister the frame receive handler.
        /// </summary>
        /// <param name="receiver">Frame receive handler.</param>
        /// <exception cref="ApiException">If an error occurred.</exception>
        /// <exception cref="ArgumentNullException">Argument is Null.</exception>
        public void UnregisterFrameCallback(FrameReceiveHandler receiver)
        {
            if (receiver == null)
            {
                throw new ArgumentNullException(nameof(receiver));
            }

            this.FrameReceiveHandler -= receiver;
            if (this.FrameReceiveHandler == null)
            {
                if (Native.StreamUnregisterFrameCallback(this.Handle) != 0)
                {
                    throw new ApiException(Native.GetLastError());
                }
            }
        }

        /// <summary>
        /// Callback function called from the C API when receiving event.
        /// </summary>
        /// <param name="streamHandle">Stream handle.</param>
        /// <param name="eventType">Event type.</param>
        /// <param name="argHandle">Event argument handle.</param>
        /// <param name="reserved">Reserved.</param>
        private void EventDispatcher(
            UInt64 streamHandle, string eventType,
            UInt64 argHandle, IntPtr reserved)
        {
            if (this.eventReceiveHandler.TryGetValue(
                eventType, out EventReceiveHandler2 receiver))
            {
                EventArgument args = new EventArgument(argHandle);
                receiver?.Invoke(this, eventType, args);
            }
        }

        /// <summary>
        /// Callback function called from the C API when receiving event.
        /// </summary>
        /// <param name="eventType">Event type.</param>
        /// <param name="reserved1">Reserved1.</param>
        /// <param name="reserved2">Reserved2.</param>
        private void EventDispatcherOld(
            string eventType, IntPtr reserved1, IntPtr reserved2)
        {
            this.EventReceiveHandlerOld?.Invoke(this, eventType);
        }

        /// <summary>
        /// Register the event receive handler.
        /// </summary>
        /// <param name="eventType">Event type.</param>
        /// <param name="receiver">Event receive handler.</param>
        /// <exception cref="ApiException">If an error occurred.</exception>
        /// <exception cref="ArgumentNullException">Argument is Null.</exception>
        public void RegisterEventCallback(
            string eventType, EventReceiveHandler2 receiver)
        {
            var ret = this.eventReceiveHandler.AddOrUpdate(
                eventType, receiver, (_key, _val) => receiver);
            if (ret == receiver)
            {
                IntPtr reserved = IntPtr.Zero;
                if (Native.StreamRegisterEventCallback(
                    this.Handle, eventType, this.eventCallbackDelegate,
                    reserved) != 0)
                {
                    throw new ApiException(Native.GetLastError());
                }
            }
        }

        /// <summary>
        /// Unregister the event receive handler.
        /// </summary>
        /// <param name="eventType">Event type.</param>
        /// <exception cref="ApiException">If an error occurred.</exception>
        /// <exception cref="ArgumentNullException">Argument is Null.</exception>
        public void UnregisterEventCallback(string eventType)
        {
            if (this.eventReceiveHandler.TryRemove(eventType, out _))
            {
                if (Native.StreamUnregisterEventCallback(
                    this.Handle, eventType) != 0)
                {
                    throw new ApiException(Native.GetLastError());
                }
            }
        }

        /// <summary>
        /// Register the event receive handler.
        /// </summary>
        /// <param name="receiver">Event receive handler.</param>
        /// <exception cref="ApiException">If an error occurred.</exception>
        /// <exception cref="ArgumentNullException">Argument is Null.</exception>
        public void RegisterEventCallback(EventReceiveHandler receiver)
        {
            if (receiver == null)
            {
                throw new ArgumentNullException(nameof(receiver));
            }

            if (this.EventReceiveHandlerOld == null)
            {
                IntPtr reserved = IntPtr.Zero;
                if (Native.StreamRegisterEventCallbackOld(
                    this.Handle, "EventAny", this.eventCallbackDelegateOld,
                    reserved) != 0)
                {
                    throw new ApiException(Native.GetLastError());
                }
            }
            this.EventReceiveHandlerOld += receiver;
        }

        /// <summary>
        /// Unregister the event receive handler.
        /// </summary>
        /// <param name="receiver">Event receive handler.</param>
        /// <exception cref="ApiException">If an error occurred.</exception>
        /// <exception cref="ArgumentNullException">Argument is Null.</exception>
        public void UnregisterEventCallback(EventReceiveHandler receiver)
        {
            if (receiver == null)
            {
                throw new ArgumentNullException(nameof(receiver));
            }

            this.EventReceiveHandlerOld -= receiver;
            if (this.EventReceiveHandlerOld == null)
            {
                if (Native.StreamUnregisterEventCallback(
                    this.Handle, "EventAny") != 0)
                {
                    throw new ApiException(Native.GetLastError());
                }
            }
        }

        /// <summary>
        /// Register the frame dispose.
        /// </summary>
        /// <param name="frame">Frame instance.</param>
        internal void Register(Frame frame)
        {
            this.frameDispose += frame.Dispose;
        }

        /// <summary>
        /// Unregister the frame dispose.
        /// </summary>
        /// <param name="frame">Frame instance.</param>
        internal void Unregister(Frame frame)
        {
            this.frameDispose -= frame.Dispose;
        }
    }
}
