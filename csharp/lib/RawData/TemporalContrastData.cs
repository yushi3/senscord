/*
 * SPDX-FileCopyrightText: 2020-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;

namespace SensCord
{
    /// <summary>
    /// Temporal contrast types for TemporalContrastEvent.
    /// </summary>
    public enum TemporalContrast
    {
        /// <summary>
        /// Negative event.
        /// </summary>
        Negative = -1,

        /// <summary>
        /// Event is none.
        /// </summary>
        None,

        /// <summary>
        /// Positive event.
        /// </summary>
        Positive,
    }

    /// <summary>
    /// TemporalContrastEvent structure.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct TemporalContrastEvent
    {
        /// <summary>
        /// x position of the event.
        /// </summary>
        private ushort x;

        /// <summary>
        /// y position of the event.
        /// </summary>
        private ushort y;

        /// <summary>
        /// Polarity of the event.
        /// </summary>
        private byte p;

        /// <summary>
        /// Reserved
        /// </summary>
        private byte reserved;

        /// <summary>
        /// x position setter/getter.
        /// </summary>
        public ushort X
        {
            /// <summary>
            /// x position getter.
            /// </summary>
            get { return this.x; }

            /// <summary>
            /// x position setter
            /// </summary>
            set { this.x = value; }
        }

        /// <summary>
        /// y position setter/getter.
        /// </summary>
        public ushort Y
        {
            /// <summary>
            /// y position getter.
            /// </summary>
            get { return this.y; }

            /// <summary>
            /// y position setter.
            /// </summary>
            set { this.y = value; }
        }

        /// <summary>
        /// Polarity data setter/getter.
        /// </summary>
        public byte P
        {
            /// <summary>
            /// Polarity data getter.
            /// </summary>
            get { return this.p; }

            /// <summary>
            /// Polarity data setter.
            /// </summary>
            set { this.p = value; }
        }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"({this.X}, {this.Y}, {this.P})";
        }
    }

    /// <summary>
    /// Create Timeslice data.
    /// </summary>
    public class TemporalContrastEventsTimeslice
    {
        /// <summary>
        /// TemporalContrastDataEventsTimeslice header size.
        ///  Timestamp(8bytes) + Count(4bytes) + reserve(4bytes) + Events pointer(64bit).
        /// </summary>
        private static readonly int TimesliceHeaderSize = 24;

        /// <summary>
        /// timestamp of the bundle.
        /// </summary>
        public decimal Timestamp { get; set; }

        /// <summary>
        /// The number of events contained.
        /// </summary>
        public int Count { get; set; }

        /// <summary>
        /// The Timeslice events pointer.
        /// </summary>
        private IntPtr events = IntPtr.Zero;

        /// <summary>
        /// The Timeslice data total size.
        /// </summary>
        private int total_size = 0;

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"({this.Timestamp}, {this.Count}, {this.events})";
        }

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="rawdata">The Timeslice data pointer.</param>
        /// <param name="size">The RawData remaining size.</param>
        /// <exception cref="ArgumentOutOfRangeException">If an error occurred.</exception>
        public TemporalContrastEventsTimeslice(IntPtr rawdata, int size)
        {
            if (size < TimesliceHeaderSize)
            {
                throw new ArgumentOutOfRangeException(
                    "Timeslice index buffer overrun at EventsTimeslice header.");
            }
            this.Timestamp = Marshal.ReadInt64(rawdata);
            this.Count = Marshal.ReadInt32(rawdata + 8);

            this.total_size = TimesliceHeaderSize +
                (this.Count * Marshal.SizeOf(typeof(TemporalContrastEvent)));
            if (size < this.total_size)
            {
                throw new ArgumentOutOfRangeException("Timeslice buffer overrun.");
            }
            this.events = (rawdata + TimesliceHeaderSize);
        }

        /// <summary>
        /// Get Timeslice events data.
        /// </summary>
        /// <param name="num">The num of the Timeslice events index.</param>
        /// <returns>TemporalContrastEvent data.</returns>
        /// <exception cref="IndexOutOfRangeException">If an error occurred.</exception>
        public TemporalContrastEvent GetEvent(int num)
        {
            if (num < 0 || this.Count <= num)
            {
                throw new IndexOutOfRangeException("Index is out of range");
            }

            return (TemporalContrastEvent)Marshal.PtrToStructure(
                IntPtr.Add(
                    this.events,
                    Marshal.SizeOf(typeof(TemporalContrastEvent)) * num),
                typeof(TemporalContrastEvent));
        }

        /// <summary>
        /// Get Timeslice data total size.
        /// </summary>
        /// <returns>Timeslice data total size.</returns>
        internal int GetTotalSize()
        {
            return this.total_size;
        }
    }

    /// <summary>
    /// Create Timeslce list.
    /// </summary>
    public class TemporalContrastData
    {
        /// <summary>
        /// TemporalContrastData header size.
        ///  Count(4bytes) + reserve(4bytes) + Bundles pointer(64bit).
        /// </summary>
        private static readonly int DataHeaderSize = 16;

        /// <summary>
        /// Raw data type.
        /// </summary>
        public string Type { get; } = RawDataType.TemporalContrast;

        /// <summary>
        /// the number of event bundles contained.
        /// </summary>
        public int Count { get; set; }

        /// <summary>
        /// TemporalContrast timeslice list.
        /// </summary>
        private TemporalContrastEventsTimeslice[] bundles = null;

        /// <summary>
        /// TemporalContrast timeslice list getter.
        /// </summary>
        public TemporalContrastEventsTimeslice[] Bundles
        {
            /// <summary>
            /// TemporalContrastEventsTimeslice data getter.
            /// </summary>
            get { return this.bundles; }
        }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"{this.Type}({this.Count}, {this.Bundles})";
        }

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="rawdata">Channel RawData.</param>
        public TemporalContrastData(RawData rawdata)
        {
            if (rawdata.Type != this.Type)
            {
                throw new ArgumentException(
                    String.Format("Invalid RawData type({0}).", rawdata.Type));
            }

            IntPtr address = rawdata.Address;
            int size = rawdata.Length;

            if (size < DataHeaderSize)
            {
                throw new ArgumentOutOfRangeException(
                    String.Format(
                        "size({0}) is smaller than TemporalContrastData header size.",
                        size));
            }

            int cursor = 0;
            this.Count = Marshal.ReadInt32(address + cursor);
            cursor += DataHeaderSize;
            this.bundles = new TemporalContrastEventsTimeslice[this.Count];
            for (int i = 0; i < this.Count; i++)
            {
                TemporalContrastEventsTimeslice bundle =
                    new TemporalContrastEventsTimeslice(
                        (address + cursor), (size - cursor));
                this.Bundles[i] = bundle;
                cursor += bundle.GetTotalSize();
            }
        }
    }
}
