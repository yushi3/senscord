/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Threading;

namespace SensCord
{
    /// <summary>
    /// The core class of managing streams.
    /// </summary>
    public class Core : IDisposable
    {
        /// <summary>
        /// Core handle.
        /// </summary>
        internal UInt64 Handle { get; private set; }

        /// <summary>
        /// Stream dispose.
        /// </summary>
        private Action streamDispose;

        /// <summary>
        /// Create a new Core class.
        /// </summary>
        /// <exception cref="ApiException">If an error occurred.</exception>
        public Core()
        {
            UInt64 handle = 0;
            if (Native.CoreInit(ref handle) != 0)
            {
                throw new ApiException(Native.GetLastError());
            }
            this.Handle = handle;
        }

        /// <summary>
        /// Releases unmanaged resources before clearing by garbage collection.
        /// </summary>
        ~Core()
        {
            this.Dispose(false);
        }

        /// <summary>
        /// Releases resources of the Core class.
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
                    ref this.streamDispose, default(Action))?.Invoke();
                if (Native.CoreExit(this.Handle) != 0)
                {
                    throw new ApiException(Native.GetLastError());
                }
                this.Handle = 0;
            }
        }

        /// <summary>
        /// Open the new stream from key.
        /// </summary>
        /// <param name="streamKey">The key of the stream to open.</param>
        /// <returns>Stream instance.</returns>
        /// <exception cref="ApiException">If an error occurred.</exception>
        public Stream OpenStream(string streamKey)
        {
            UInt64 streamHandle = 0;
            if (Native.CoreOpenStream(
                this.Handle, streamKey, ref streamHandle) != 0)
            {
                throw new ApiException(Native.GetLastError());
            }
            return new Stream(this, streamHandle, streamKey);
        }

        /// <summary>
        /// Open the new stream from key and specified setting.
        /// </summary>
        /// <param name="streamKey">The key of the stream to open.</param>
        /// <param name="setting">Setting to open stream.</param>
        /// <returns>Stream instance.</returns>
        /// <exception cref="ApiException">If an error occurred.</exception>
        public Stream OpenStream(string streamKey, OpenStreamSetting setting)
        {
            const int StreamArgumentsMax = 32;
            if (setting.Arguments != null && (setting.Arguments.Count > StreamArgumentsMax))
            {
                throw new ArgumentOutOfRangeException(
                    "Arguments", "The number of elements exceeds max.");
            }
            UInt64 streamHandle = 0;
            NativeOpenStreamSetting tmp_setting = new NativeOpenStreamSetting();
            tmp_setting.frame_buffering = setting.FrameBuffering;
            tmp_setting.arguments = new NativeStreamArgument[StreamArgumentsMax];
            tmp_setting.arguments_count = 0;
            if (setting.Arguments != null)
            {
                foreach (KeyValuePair<string, string> item in setting.Arguments)
                {
                    tmp_setting.arguments[tmp_setting.arguments_count].name = item.Key;
                    tmp_setting.arguments[tmp_setting.arguments_count].value = item.Value;
                    ++tmp_setting.arguments_count;
                }
            }
            if (Native.CoreOpenStream(
                this.Handle, streamKey, ref tmp_setting,
                ref streamHandle) != 0)
            {
                throw new ApiException(Native.GetLastError());
            }
            return new Stream(this, streamHandle, streamKey);
        }

        /// <summary>
        /// Get the version of this core library.
        /// </summary>
        /// <returns>The version string of this core library.</returns>
        /// <exception cref="ApiException">If an error occurred.</exception>
        public SensCordVersion GetVersion()
        {
            NativeSensCordVersion tmp_version =
                new NativeSensCordVersion();
            if (Native.CoreGetVersion(
                this.Handle, ref tmp_version) != 0)
            {
                throw new ApiException(Native.GetLastError());
            }
            SensCordVersion version =
                this.ConvertSensCordVersion(tmp_version);
            return version;
        }

        /// <summary>
        /// Convert senscord version structure.
        /// </summary>
        /// <param name="src">Native senscord version structure.</param>
        /// <returns>Converted version.</returns>
        private SensCordVersion ConvertSensCordVersion(NativeSensCordVersion src)
        {
            SensCordVersion version = new SensCordVersion();
            version.SenscordVersion =
                this.ConvertVersion(src.senscord_version);
            version.ProjectVersion =
                this.ConvertVersion(src.project_version);
            version.StreamVersions =
                new Dictionary<string, StreamVersion>();
            for (int index = 0; index < src.stream_count; ++index)
            {
                NativeStreamVersion srcStream =
                    src.StreamVersions(index);
                StreamVersion stream =
                    this.ConvertStreamVersion(srcStream);
                version.StreamVersions.Add(
                    srcStream.stream_key, stream);
            }
            version.ServerVersions =
                new Dictionary<int, SensCordVersion>();
            for (int index = 0; index < src.server_count; ++index)
            {
                NativeSensCordVersion srcServerVersion =
                    src.ServerVersions(index);
                version.ServerVersions.Add(
                    srcServerVersion.destination_id,
                    this.ConvertSensCordVersion(srcServerVersion));
            }
            return version;
        }

        /// <summary>
        /// Convert version structure.
        /// </summary>
        /// <param name="src">Native version structure.</param>
        /// <returns>Converted version.</returns>
        private Version ConvertVersion(NativeVersion src)
        {
            Version version = new Version();
            version.Name = src.name;
            version.Major = src.major;
            version.Minor = src.minor;
            version.Patch = src.patch;
            version.Desciption = src.description;
            return version;
        }

        /// <summary>
        /// Convert stream version structure.
        /// </summary>
        /// <param name="src">Native stream version structure.</param>
        /// <returns>Converted version.</returns>
        private StreamVersion ConvertStreamVersion(NativeStreamVersion src)
        {
            StreamVersion stream = new StreamVersion();
            stream.Version = this.ConvertVersion(src.stream_version);
            stream.LinkageVersion = new List<Version>();
            for (int index = 0; index < src.linkage_count; ++index)
            {
                Version version =
                    this.ConvertVersion(src.LinkageVersions(index));
                stream.LinkageVersion.Add(version);
            }
            stream.DestinationId = src.destination_id;
            return stream;
        }

        /// <summary>
        /// Get supported streams list.
        /// </summary>
        /// <returns>Supported streams list.</returns>
        /// <exception cref="ApiException">If an error occurred.</exception>
        public StreamTypeInfo[] GetStreamList()
        {
            UInt32 count = 0;
            if (Native.CoreGetStreamCount(this.Handle, ref count) != 0)
            {
                throw new ApiException(Native.GetLastError());
            }

            var list = new List<StreamTypeInfo>();
            for (UInt32 index = 0; index < count; ++index)
            {
                var info = new StreamTypeInfo();
                if (Native.CoreGetStreamInfo(
                    this.Handle, index, ref info) != 0)
                {
                    throw new ApiException(Native.GetLastError());
                }
                list.Add(info);
            }
            return list.ToArray();
        }

        /// <summary>
        /// Get the count of opened by stream key in the process.
        /// </summary>
        /// <param name="streamKey">Stream key.</param>
        /// <returns>Opened count.</returns>
        /// <exception cref="ApiException">If an error occurred.</exception>
        public int GetOpenedStreamCount(string streamKey)
        {
            UInt32 count = 0;
            if (Native.CoreGetOpenedStreamCount(
                this.Handle, streamKey, ref count) != 0)
            {
                throw new ApiException(Native.GetLastError());
            }
            return (int)count;
        }

        /// <summary>
        /// Register the stream dispose.
        /// </summary>
        /// <param name="stream">Stream instance.</param>
        internal void Register(Stream stream)
        {
            this.streamDispose += stream.Dispose;
        }

        /// <summary>
        /// Unregister the stream dispose.
        /// </summary>
        /// <param name="stream">Stream instance.</param>
        internal void Unregister(Stream stream)
        {
            this.streamDispose -= stream.Dispose;
        }
    }

    /// <summary>
    /// Open stream setting.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct OpenStreamSetting
    {
        /// <summary>
        /// Frame buffering setting.
        /// </summary>
        [MarshalAs(UnmanagedType.Struct)]
        public FrameBuffering FrameBuffering;

        /// <summary>
        /// Stream arguments.
        /// </summary>
        private Dictionary<string, string> args;

        /// <summary>
        /// Stream arguments.
        /// </summary>
        public Dictionary<string, string> Arguments
        {
            get { return this.args; }
            set { this.args = value; }
        }
    }

    /// <summary>
    /// Structure for version.
    /// </summary>
    public struct Version
    {
        /// <summary>
        /// Name.
        /// </summary>
        public string Name;

        /// <summary>
        /// Major version.
        /// </summary>
        public UInt32 Major;

        /// <summary>
        /// Minor version.
        /// </summary>
        public UInt32 Minor;

        /// <summary>
        /// Patch version.
        /// </summary>
        public UInt32 Patch;

        /// <summary>
        /// Version description.
        /// </summary>
        public string Desciption;

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return string.Format(
                "name={0} major={1} minor={2} patch={3} " +
                "description={4}",
                this.Name, this.Major, this.Minor, this.Patch, this.Desciption);
        }
    }

    /// <summary>
    /// Structure for stream version.
    /// </summary>
    public struct StreamVersion
    {
        /// <summary>
        /// Stream version.
        /// </summary>
        public Version Version;

        /// <summary>
        /// Stream linkage version.
        /// </summary>
        public List<Version> LinkageVersion;

        /// <summary>
        /// Destination ID.
        /// </summary>
        public Int32 DestinationId;

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"Version({this.Version}) " +
                   $"Linkage({string.Join(", ", this.LinkageVersion)}) " +
                   $"DestinationId={this.DestinationId}";
        }
    }

    /// <summary>
    /// Structure for SensCord version.
    /// </summary>
    public struct SensCordVersion
    {
        /// <summary>
        /// SensCord version.
        /// </summary>
        public Version SenscordVersion;

        /// <summary>
        /// Project version.
        /// </summary>
        public Version ProjectVersion;

        /// <summary>
        /// Stream versions(Key=StreamKey).
        /// </summary>
        public Dictionary<string, StreamVersion> StreamVersions;

        /// <summary>
        /// Server versions(Key=Destination ID).
        /// </summary>
        public Dictionary<Int32, SensCordVersion> ServerVersions;

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return $"SensCord({this.SenscordVersion})\n" +
                   $"Project({this.ProjectVersion})\n" +
                   $"Stream({string.Join(", ", this.StreamVersions)})\n" +
                   $"Server({string.Join(", ", this.ServerVersions)})";
        }
    }
}
