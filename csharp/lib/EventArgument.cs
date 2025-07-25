/*
 * SPDX-FileCopyrightText: 2020-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace SensCord
{
    /// <summary>
    /// Event type.
    /// </summary>
    public static partial class EventType
    {
        /// <summary>
        /// Only for event receiving.
        /// </summary>
        public static readonly string Any = "EventAny";

        /// <summary>
        /// Error event.
        /// </summary>
        public static readonly string Error = "EventError";

        /// <summary>
        /// Fatal error event.
        /// </summary>
        public static readonly string Fatal = "EventFatal";

        /// <summary>
        /// Frame dropped event.
        /// </summary>
        public static readonly string FrameDropped = "EventFrameDropped";

        /// <summary>
        /// Property updated event.
        /// </summary>
        public static readonly string PropertyUpdated = "EventPropertyUpdated";

        /// <summary>
        /// Device plugged event.
        /// </summary>
        public static readonly string Plugged = "EventPlugged";

        /// <summary>
        /// Device unplugged event.
        /// </summary>
        public static readonly string Unplugged = "EventUnplugged";

        /// <summary>
        /// Record state event.
        /// </summary>
        public static readonly string RecordState = "EventRecordState";
    }

    /// <summary>
    /// Event argument key.
    /// </summary>
    public static partial class EventArgumentKey
    {
        /// <summary>
        /// Event argument key "cause" (for EventType.Error or Fatal).
        /// </summary>
        /// <example>
        /// <code><![CDATA[
        ///   var cause = args.Get<SensCord.ErrorCause>(EventArgumentKey.Cause);
        /// ]]></code>
        /// </example>
        public static readonly string Cause = "cause";

        /// <summary>
        /// Event argument key "message" (for EventType.Error or Fatal).
        /// </summary>
        /// <example>
        /// <code><![CDATA[
        ///   var message = args.Get<string>(EventArgumentKey.Message);
        /// ]]></code>
        /// </example>
        public static readonly string Message = "message";

        /// <summary>
        /// Event argument key "sequence_number" (for EventType.FrameDropped).
        /// </summary>
        /// <example>
        /// <code><![CDATA[
        ///   var seqNum = args.Get<UInt64>(EventArgumentKey.SequenceNumber);
        /// ]]></code>
        /// </example>
        public static readonly string SequenceNumber = "sequence_number";

        /// <summary>
        /// Event argument key "property_key" (for EventType.PropertyUpdated).
        /// </summary>
        /// <example>
        /// <code><![CDATA[
        ///   var propertyKey = args.Get<string>(EventArgumentKey.PropertyKey);
        /// ]]></code>
        /// </example>
        public static readonly string PropertyKey = "property_key";

        /// <summary>
        /// Event argument key "state" (for EventType.RecordState).
        /// </summary>
        /// <example>
        /// <code><![CDATA[
        ///   var recordState = args.Get<Int>(EventArgumentKey.RecordState);
        /// ]]></code>
        /// </example>
        public static readonly string RecordState = "state";

        /// <summary>
        /// Event argument key "count" (for EventType.RecordState).
        /// </summary>
        /// <example>
        /// <code><![CDATA[
        ///   var recordCount = args.Get<Int>(EventArgumentKey.RecordCount);
        /// ]]></code>
        /// </example>
        public static readonly string RecordCount = "count";

        /// <summary>
        /// Event argument key "path" (for EventType.RecordState).
        /// </summary>
        /// <example>
        /// <code><![CDATA[
        ///   var recordCount = args.Get<string>(EventArgumentKey.RecordPath);
        /// ]]></code>
        /// </example>
        public static readonly string RecordPath = "path";
    }

    /// <summary>
    /// Event argument.
    /// </summary>
    /// Arguments consist of an associative array of keys and values.
    public class EventArgument
    {
        /// <summary>
        /// Handle.
        /// </summary>
        private readonly UInt64 handle;

        /// <summary>
        /// Key.
        /// </summary>
        private string[] keys = new string[0];

        /// <summary>
        /// Create a new EventArgument instance.
        /// </summary>
        /// <param name="argHandle">Event argument handle.</param>
        internal EventArgument(UInt64 argHandle)
        {
            this.handle = argHandle;

            UInt32 count = 0;
            if (Native.EventArgumentGetElementCount(
                this.handle, ref count) != 0)
            {
                throw new ApiException(Native.GetLastError());
            }

            var list = new List<string>();
            for (UInt32 index = 0; index < count; ++index)
            {
                IntPtr ptr = Native.EventArgumentGetKey(this.handle, index);
                if (ptr == IntPtr.Zero)
                {
                    throw new ApiException(Native.GetLastError());
                }
                string key = Native.PtrToString(ptr);
                list.Add(key);
            }
            this.keys = list.ToArray();
        }

        /// <summary>
        /// Gets the keys.
        /// </summary>
        /// <value>The keys.</value>
        public string[] Keys { get { return this.keys;  } }

        /// <summary>
        /// Gets the value of the specified key.
        /// </summary>
        /// <returns>The value.</returns>
        /// <param name="key">Argument key.</param>
        /// <typeparam name="T">Generic type parameter.</typeparam>
        public T Get<T>(string key)
        {
            // get the required size.
            UInt32 length = 0;
            Native.EventArgumentGetSerializedBinary(
                this.handle, key, IntPtr.Zero, ref length);
            if (length == 0)
            {
                throw new ApiException(Native.GetLastError());
            }
            // get the serialized binary.
            var bytes = new byte[length];
            var handle = GCHandle.Alloc(bytes, GCHandleType.Pinned);
            var handlePtr = handle.AddrOfPinnedObject();
            try
            {
                if (Native.EventArgumentGetSerializedBinary(
                    this.handle, key, handlePtr, ref length) != 0)
                {
                    throw new ApiException(Native.GetLastError());
                }
            }
            finally
            {
                handle.Free();
            }
            return Serializer.Instance.Unpack<T>(bytes);
        }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"EventArgument[ {String.Join(", ", this.Keys)} ]";
        }
    }
}
