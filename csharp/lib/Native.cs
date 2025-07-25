/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using System;
using System.Runtime.InteropServices;

namespace SensCord
{
    /// <summary>
    /// SensCord C API.
    /// </summary>
    internal static class Native
    {
        /// <summary>
        /// SensCord library name.
        /// </summary>
        private const string dllName = "senscord";

        /// <summary>
        /// Calling convention.
        /// </summary>
        private const CallingConvention convention = CallingConvention.Cdecl;

        /// <summary>
        /// Char set.
        /// </summary>
        private const CharSet charSet = CharSet.Ansi;

        /// <summary>
        /// Copies all characters from unmanaged string.
        /// </summary>
        /// <returns>A managed string.</returns>
        /// <param name="ptr">The address of the unmanaged string.</param>
        internal static string PtrToString(IntPtr ptr)
        {
            return Marshal.PtrToStringAnsi(ptr);
        }

        #region Common APIs

        /// <summary>
        /// senscord_get_last_error of naive API.
        /// </summary>
        /// <returns>Status.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_get_last_error")]
        internal static extern Status GetLastError();

        /// <summary>
        /// senscord_set_file_search_path of naive API.
        /// </summary>
        /// <param name="paths">The same format as SENSCORD_FILE_PATH.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention, CharSet = charSet,
            EntryPoint = "senscord_set_file_search_path")]
        internal static extern int SetFileSearchPath(string paths);

        #endregion

        #region Core APIs

        /// <summary>
        /// senscord_core_init of naive API.
        /// </summary>
        /// <param name="core">Core handle.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_core_init")]
        internal static extern int CoreInit(ref UInt64 core);

        /// <summary>
        /// senscord_core_exit of naive API.
        /// </summary>
        /// <param name="core">Core handle.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_core_exit")]
        internal static extern int CoreExit(UInt64 core);

        /// <summary>
        /// senscord_core_open_stream of naive API.
        /// </summary>
        /// <param name="core">Core handle.</param>
        /// <param name="streamKey">The key of the stream to open.</param>
        /// <param name="stream">The new stream handle.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention, CharSet = charSet,
            EntryPoint = "senscord_core_open_stream")]
        internal static extern int CoreOpenStream(
            UInt64 core, string streamKey, ref UInt64 stream);

        /// <summary>
        /// senscord_core_open_stream_with_setting of naive API.
        /// </summary>
        /// <param name="core">Core handle.</param>
        /// <param name="streamKey">The key of the stream to open.</param>
        /// <param name="setting">Config to open stream.</param>
        /// <param name="stream">The new stream handle.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention, CharSet = charSet,
            EntryPoint = "senscord_core_open_stream_with_setting")]
        internal static extern int CoreOpenStream(
            UInt64 core, string streamKey, ref NativeOpenStreamSetting setting, ref UInt64 stream);

        /// <summary>
        /// senscord_core_close_stream of naive API.
        /// </summary>
        /// <param name="core">Core handle.</param>
        /// <param name="stream">The opened stream handle.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_core_close_stream")]
        internal static extern int CoreCloseStream(UInt64 core, UInt64 stream);

        /// <summary>
        /// senscord_core_get_version of naive API.
        /// </summary>
        /// <param name="core">Core handle.</param>
        /// <param name="version">The version of this core library.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention, CharSet = charSet,
            EntryPoint = "senscord_core_get_version")]
        internal static extern int CoreGetVersion(
            UInt64 core, ref NativeSensCordVersion version);

        /// <summary>
        /// senscord_core_get_stream_count of naive API.
        /// </summary>
        /// <param name="core">Core handle.</param>
        /// <param name="count">Count of opened stream.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_core_get_stream_count")]
        internal static extern int CoreGetStreamCount(
            UInt64 core, ref UInt32 count);

        /// <summary>
        /// senscord_core_get_stream_info of naive API.
        /// </summary>
        /// <param name="core">Core handle.</param>
        /// <param name="index">Index of supported streams list.</param>
        /// <param name="streamInfo">Location of stream information.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_core_get_stream_info")]
        internal static extern int CoreGetStreamInfo(
            UInt64 core, UInt32 index, ref StreamTypeInfo streamInfo);

        /// <summary>
        /// senscord_core_get_opened_stream_count of naive API.
        /// </summary>
        /// <param name="core">Core handle.</param>
        /// <param name="streamKey">Stream key.</param>
        /// <param name="count">Count of opened stream.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention, CharSet = charSet,
            EntryPoint = "senscord_core_get_opened_stream_count")]
        internal static extern int CoreGetOpenedStreamCount(
            UInt64 core, string streamKey, ref UInt32 count);

        #endregion

        #region Stream APIs

        /// <summary>
        /// senscord_stream_start of naive API.
        /// </summary>
        /// <param name="stream">Stream handle.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_stream_start")]
        internal static extern int StreamStart(UInt64 stream);

        /// <summary>
        /// senscord_stream_stop of naive API.
        /// </summary>
        /// <param name="stream">Stream handle.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_stream_stop")]
        internal static extern int StreamStop(UInt64 stream);

        /// <summary>
        /// senscord_stream_get_frame of naive API.
        /// </summary>
        /// <param name="stream">Stream handle.</param>
        /// <param name="frame">Location of received frame.</param>
        /// <param name="timeoutMsec">Time of wait msec if no received.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_stream_get_frame")]
        internal static extern int StreamGetFrame(
            UInt64 stream, ref UInt64 frame, Int32 timeoutMsec);

        /// <summary>
        /// senscord_stream_release_frame of naive API.
        /// </summary>
        /// <param name="stream">Stream handle.</param>
        /// <param name="frame">Received frame by senscord_stream_get_frame().</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_stream_release_frame")]
        internal static extern int StreamReleaseFrame(
            UInt64 stream, UInt64 frame);

        /// <summary>
        /// senscord_stream_release_frame_unused of naive API.
        /// </summary>
        /// <param name="stream">Stream handle.</param>
        /// <param name="frame">Received frame by senscord_stream_get_frame().</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_stream_release_frame_unused")]
        internal static extern int StreamReleaseFrameUnused(
            UInt64 stream, UInt64 frame);

        /// <summary>
        /// senscord_stream_clear_frames of naive API.
        /// </summary>
        /// <param name="stream">Stream handle.</param>
        /// <param name="frame_number">Number of cleared frames (optional).</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_stream_clear_frames")]
        internal static extern int StreamClearFrames(
            UInt64 stream, ref Int32 frame_number);

        /// <summary>
        /// senscord_stream_get_serialized_property of naive API.
        /// </summary>
        /// <param name="stream">Stream handle.</param>
        /// <param name="propertyKey">Key of property to get.</param>
        /// <param name="buffer">Buffer that stores output property values.</param>
        /// <param name="bufferSize">Buffer size.</param>
        /// <param name="outputSize">Size of output property (optional).</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention, CharSet = charSet,
            EntryPoint = "senscord_stream_get_serialized_property")]
        internal static extern int StreamGetSerializedProperty(
            UInt64 stream, string propertyKey, IntPtr buffer, UIntPtr bufferSize,
            ref UIntPtr outputSize);

        /// <summary>
        /// senscord_stream_set_serialized_property of naive API.
        /// </summary>
        /// <param name="stream">Stream handle.</param>
        /// <param name="propertyKey">Key of property to set.</param>
        /// <param name="buffer">Buffer that contains input property values.</param>
        /// <param name="bufferSize">Buffer size.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention, CharSet = charSet,
            EntryPoint = "senscord_stream_set_serialized_property")]
        internal static extern int StreamSetSerializedProperty(
            UInt64 stream, string propertyKey, IntPtr buffer, UIntPtr bufferSize);

        /// <summary>
        /// senscord_stream_get_property_count of naive API.
        /// </summary>
        /// <param name="stream">Stream handle.</param>
        /// <param name="count">Count of supported property key.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_stream_get_property_count")]
        internal static extern int StreamGetPropertyCount(
            UInt64 stream, ref UInt32 count);

        /// <summary>
        /// senscord_stream_get_property_key of naive API.
        /// </summary>
        /// <param name="stream">Stream handle.</param>
        /// <param name="index">Index of supported property key list.</param>
        /// <param name="propertyKey">Location of property key.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_stream_get_property_key")]
        internal static extern int StreamGetPropertyKey(
            UInt64 stream, UInt32 index, ref IntPtr propertyKey);

        /// <summary>
        /// senscord_stream_lock_property of naive API.
        /// </summary>
        /// <param name="stream">Stream handle.</param>
        /// <param name="timeoutMsec">Time of wait msec if locked already.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_stream_lock_property")]
        internal static extern int StreamLockProperty(
            UInt64 stream, Int32 timeoutMsec);

        /// <summary>
        /// senscord_stream_lock_property_with_key of naive API.
        /// </summary>
        /// <param name="stream">Stream handle.</param>
        /// <param name="keys">Key of property to lock.</param>
        /// <param name="count">Count of property key.</param>
        /// <param name="timeoutMsec">Time of wait msec if locked already.</param>
        /// <param name="resource">Resource handle.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_stream_lock_property_with_key")]
        internal static extern int StreamLockProperty(
            UInt64 stream, string[] keys, UInt32 count,
            Int32 timeoutMsec, ref UInt64 resource);

        /// <summary>
        /// senscord_stream_unlock_property of naive API.
        /// </summary>
        /// <param name="stream">Stream handle.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_stream_unlock_property")]
        internal static extern int StreamUnlockProperty(UInt64 stream);

        /// <summary>
        /// senscord_stream_unlock_property_by_resource of naive API.
        /// </summary>
        /// <param name="stream">Stream handle.</param>
        /// <param name="resource">Resource handle.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_stream_unlock_property_by_resource")]
        internal static extern int StreamUnlockProperty(
            UInt64 stream, UInt64 resource);

        /// <summary>
        /// Frame callback.
        /// </summary>
        /// <param name="stream">Stream handle.</param>
        /// <param name="reserved">Private data.</param>
        [UnmanagedFunctionPointer(convention)]
        internal delegate void FrameCallback(UInt64 stream, IntPtr reserved);

        /// <summary>
        /// senscord_stream_register_frame_callback of naive API.
        /// </summary>
        /// <param name="stream">Stream handle.</param>
        /// <param name="callback">Function pointer.</param>
        /// <param name="privateData">Private data with callback.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_stream_register_frame_callback")]
        internal static extern int StreamRegisterFrameCallback(
            UInt64 stream, FrameCallback callback, IntPtr privateData);

        /// <summary>
        /// senscord_stream_unregister_frame_callback of naive API.
        /// </summary>
        /// <param name="stream">Stream handle.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_stream_unregister_frame_callback")]
        internal static extern int StreamUnregisterFrameCallback(UInt64 stream);

        /// <summary>
        /// Event callback (old).
        /// </summary>
        /// <param name="eventType">Event type to receive.</param>
        /// <param name="reserved1">Not used.</param>
        /// <param name="reserved2">Private data.</param>
        [UnmanagedFunctionPointer(convention, CharSet = charSet)]
        internal delegate void EventCallbackOld(
            string eventType, IntPtr reserved1, IntPtr reserved2);

        /// <summary>
        /// senscord_stream_register_event_callback of naive API.
        /// </summary>
        /// <param name="stream">Stream handle.</param>
        /// <param name="eventType">Event type to receive.</param>
        /// <param name="callback">Function pointer.</param>
        /// <param name="privateData">Private data with callback.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention, CharSet = charSet,
            EntryPoint = "senscord_stream_register_event_callback")]
        internal static extern int StreamRegisterEventCallbackOld(
            UInt64 stream, string eventType, EventCallbackOld callback,
            IntPtr privateData);

        /// <summary>
        /// Event callback.
        /// </summary>
        /// <param name="stream">Stream handle.</param>
        /// <param name="eventType">Event type to receive.</param>
        /// <param name="args">Event argument handle.</param>
        /// <param name="reserved">Private data.</param>
        [UnmanagedFunctionPointer(convention, CharSet = charSet)]
        internal delegate void EventCallback(
            UInt64 stream, string eventType, UInt64 args, IntPtr reserved);

        /// <summary>
        /// senscord_stream_register_event_callback2 of naive API.
        /// </summary>
        /// <returns>Error code.</returns>
        /// <param name="stream">Stream handle.</param>
        /// <param name="eventType">Event type to receive.</param>
        /// <param name="callback">Function pointer.</param>
        /// <param name="privateData">Private data with callback.</param>
        [DllImport(dllName, CallingConvention = convention, CharSet = charSet,
            EntryPoint = "senscord_stream_register_event_callback2")]
        internal static extern int StreamRegisterEventCallback(
            UInt64 stream, string eventType, EventCallback callback,
            IntPtr privateData);

        /// <summary>
        /// senscord_stream_unregister_event_callback of naive API.
        /// </summary>
        /// <param name="stream">Stream handle.</param>
        /// <param name="eventType">Event type to receive.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention, CharSet = charSet,
            EntryPoint = "senscord_stream_unregister_event_callback")]
        internal static extern int StreamUnregisterEventCallback(
            UInt64 stream, string eventType);

        #endregion

        #region Frame APIs

        /// <summary>
        /// senscord_frame_get_sequence_number of naive API.
        /// </summary>
        /// <param name="frame">Frame handle.</param>
        /// <param name="sequenceNumber">The number of this frame.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_frame_get_sequence_number")]
        internal static extern int FrameGetSequenceNumber(
            UInt64 frame, ref UInt64 sequenceNumber);

        /// <summary>
        /// senscord_frame_get_type of naive API.
        /// </summary>
        /// <param name="frame">Frame handle.</param>
        /// <param name="type">Type of frame.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_frame_get_type")]
        internal static extern int FrameGetType(UInt64 frame, ref IntPtr type);

        /// <summary>
        /// senscord_frame_get_channel_count of naive API.
        /// </summary>
        /// <param name="frame">Frame handle.</param>
        /// <param name="count">Location of channel count.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_frame_get_channel_count")]
        internal static extern int FrameGetChannelCount(
            UInt64 frame, ref UInt32 count);

        /// <summary>
        /// senscord_frame_get_channel of naive API.
        /// </summary>
        /// <param name="frame">Frame handle.</param>
        /// <param name="index">Index of channel list.</param>
        /// <param name="channel">Channel handle.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_frame_get_channel")]
        internal static extern int FrameGetChannel(
            UInt64 frame, UInt32 index, ref UInt64 channel);

        /// <summary>
        /// senscord_frame_get_user_data of naive API.
        /// </summary>
        /// <param name="frame">Frame handle.</param>
        /// <param name="userData">User data.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_frame_get_user_data")]
        internal static extern int FrameGetUserData(
            UInt64 frame, ref Frame.UserData userData);

        #endregion

        #region Channel APIs

        /// <summary>
        /// senscord_channel_get_channel_id of naive API.
        /// </summary>
        /// <param name="channel">Channel handle.</param>
        /// <param name="channelId">Channel ID.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_channel_get_channel_id")]
        internal static extern int ChannelGetChannelId(
            UInt64 channel, ref UInt32 channelId);

        /// <summary>
        /// senscord_channel_get_raw_data of naive API.
        /// </summary>
        /// <param name="channel">Channel handle.</param>
        /// <param name="rawData">Raw data.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_channel_get_raw_data")]
        internal static extern int ChannelGetRawData(
            UInt64 channel, ref RawData rawData);

        /// <summary>
        /// senscord_channel_get_serialized_property of naive API.
        /// </summary>
        /// <param name="channel">Channel handle.</param>
        /// <param name="propertyKey">Key of property to get.</param>
        /// <param name="buffer">Buffer that stores output property values.</param>
        /// <param name="bufferSize">Buffer size.</param>
        /// <param name="outputSize">Size of output property (optional).</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention, CharSet = charSet,
            EntryPoint = "senscord_channel_get_serialized_property")]
        internal static extern int ChannelGetSerializedProperty(
            UInt64 channel, string propertyKey, IntPtr buffer,
            UIntPtr bufferSize, ref UIntPtr outputSize);

        /// <summary>
        /// senscord_channel_get_property_count of naive API.
        /// </summary>
        /// <param name="channel">Channel handle.</param>
        /// <param name="count">Count of stored property key.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_channel_get_property_count")]
        internal static extern int ChannelGetPropertyCount(
            UInt64 channel, ref UInt32 count);

        /// <summary>
        /// senscord_channel_get_property_key of naive API.
        /// </summary>
        /// <param name="channel">Channel handle.</param>
        /// <param name="index">Index of stored property key list.</param>
        /// <param name="propertyKey">Location of property key.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_channel_get_property_key")]
        internal static extern int ChannelGetPropertyKey(
            UInt64 channel, UInt32 index, ref IntPtr propertyKey);

        /// <summary>
        /// senscord_channel_get_updated_property_count of naive API.
        /// </summary>
        /// <param name="channel">Channel handle.</param>
        /// <param name="count">Count of updated property key.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_channel_get_updated_property_count")]
        internal static extern int ChannelGetUpdatedPropertyCount(
            UInt64 channel, ref UInt32 count);

        /// <summary>
        /// senscord_channel_get_updated_property_key of naive API.
        /// </summary>
        /// <param name="channel">Channel handle.</param>
        /// <param name="index">Index of updated property key list.</param>
        /// <param name="propertyKey">Location of property key.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention,
            EntryPoint = "senscord_channel_get_updated_property_key")]
        internal static extern int ChannelGetUpdatedPropertyKey(
            UInt64 channel, UInt32 index, ref IntPtr propertyKey);

        #endregion

        #region EventArgument APIs

        /// <summary>
        /// senscord_event_argument_get_serialized_binary of naive API.
        /// </summary>
        /// <param name="args">Event argument handle.</param>
        /// <param name="key">Argument key.</param>
        /// <param name="buffer">Location to store the binary array.</param>
        /// <param name="length">Buffer size.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention, CharSet = charSet,
            EntryPoint = "senscord_event_argument_get_serialized_binary")]
        internal static extern int EventArgumentGetSerializedBinary(
            UInt64 args, string key, IntPtr buffer, ref UInt32 length);

        /// <summary>
        /// senscord_event_argument_get_element_count of naive API.
        /// </summary>
        /// <param name="args">Event argument handle.</param>
        /// <param name="count">Location to store the number of elements.</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention, CharSet = charSet,
            EntryPoint = "senscord_event_argument_get_element_count")]
        internal static extern int EventArgumentGetElementCount(
            UInt64 args, ref UInt32 count);

        /// <summary>
        /// senscord_event_argument_get_key of naive API.
        /// </summary>
        /// <param name="args">Event argument handle.</param>
        /// <param name="index">Index (0 to elements-1).</param>
        /// <returns>Error code.</returns>
        [DllImport(dllName, CallingConvention = convention, CharSet = charSet,
            EntryPoint = "senscord_event_argument_get_key")]
        internal static extern IntPtr EventArgumentGetKey(
            UInt64 args, UInt32 index);

        #endregion

        #region Audio APIs

        /// <summary>
        /// senscord_audio_pcm_get_byte_width of naive API.
        /// </summary>
        /// <param name="format">PCM format</param>
        /// <returns>Byte width.</returns>
        [DllImport(dllName, CallingConvention = convention, CharSet = charSet,
            EntryPoint = "senscord_audio_pcm_get_byte_width")]
        internal static extern int AudioPcmGetByteWidth(int format);

        /// <summary>
        /// senscord_audio_pcm_get_bits_per_sample of naive API.
        /// </summary>
        /// <param name="format">PCM format</param>
        /// <returns>Number of bits per sample.</returns>
        [DllImport(dllName, CallingConvention = convention, CharSet = charSet,
            EntryPoint = "senscord_audio_pcm_get_bits_per_sample")]
        internal static extern int AudioPcmGetBitsPerSample(int format);

        /// <summary>
        /// senscord_audio_pcm_is_signed of naive API.
        /// </summary>
        /// <param name="format">PCM format</param>
        /// <returns>Non-zero if signed type.</returns>
        [DllImport(dllName, CallingConvention = convention, CharSet = charSet,
            EntryPoint = "senscord_audio_pcm_is_signed")]
        internal static extern int AudioPcmIsSigned(int format);

        /// <summary>
        /// senscord_audio_pcm_is_unsigned of naive API.
        /// </summary>
        /// <param name="format">PCM format</param>
        /// <returns>Non-zero if unsigned type.</returns>
        [DllImport(dllName, CallingConvention = convention, CharSet = charSet,
            EntryPoint = "senscord_audio_pcm_is_unsigned")]
        internal static extern int AudioPcmIsUnsigned(int format);

        /// <summary>
        /// senscord_audio_pcm_is_float of naive API.
        /// </summary>
        /// <param name="format">PCM format</param>
        /// <returns>Non-zero if float type.</returns>
        [DllImport(dllName, CallingConvention = convention, CharSet = charSet,
            EntryPoint = "senscord_audio_pcm_is_float")]
        internal static extern int AudioPcmIsFloat(int format);

        /// <summary>
        /// senscord_audio_pcm_is_little_endian of naive API.
        /// </summary>
        /// <param name="format">PCM format</param>
        /// <returns>Non-zero if little endian.</returns>
        [DllImport(dllName, CallingConvention = convention, CharSet = charSet,
            EntryPoint = "senscord_audio_pcm_is_little_endian")]
        internal static extern int AudioPcmIsLittleEndian(int format);

        /// <summary>
        /// senscord_audio_pcm_is_big_endian of naive API.
        /// </summary>
        /// <param name="format">PCM format</param>
        /// <returns>Non-zero if big endian.</returns>
        [DllImport(dllName, CallingConvention = convention, CharSet = charSet,
            EntryPoint = "senscord_audio_pcm_is_big_endian")]
        internal static extern int AudioPcmIsBigEndian(int format);

        #endregion
    }

    /// <summary>
    /// The information of stream key.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct StreamTypeInfo
    {
        /// <summary>
        /// Stream key.
        /// </summary>
        [MarshalAs(UnmanagedType.SysInt)]
        private IntPtr key;

        /// <summary>
        /// Stream type.
        /// </summary>
        [MarshalAs(UnmanagedType.SysInt)]
        private IntPtr type;

        /// <summary>
        /// Unique identifier of a SensCord process.
        /// </summary>
        [MarshalAs(UnmanagedType.SysInt)]
        private IntPtr id;

        /// <summary>
        /// Stream key.
        /// </summary>
        public string Key { get { return Native.PtrToString(this.key); } }

        /// <summary>
        /// Stream type.
        /// </summary>
        public string Type { get { return Native.PtrToString(this.type); } }

        /// <summary>
        /// Id.
        /// </summary>
        public string Id { get { return Native.PtrToString(this.id); } }
    }

    /// <summary>
    /// Structure for StreamArgument(native).
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    internal struct NativeStreamArgument
    {
        /// <summary>
        /// Name.
        /// </summary>
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 32)]
        public string name;

        /// <summary>
        /// Value.
        /// </summary>
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string value;
    }

    /// <summary>
    /// Open stream setting.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    internal struct NativeOpenStreamSetting
    {
        /// <summary>
        /// Frame buffering setting.
        /// </summary>
        [MarshalAs(UnmanagedType.Struct)]
        public FrameBuffering frame_buffering;

        /// <summary>
        /// Count of the stream argument array.
        /// </summary>
        [MarshalAs(UnmanagedType.U4)]
        public int arguments_count;

        /// <summary>
        /// Stream argument array.
        /// </summary>
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32)]
        public NativeStreamArgument[] arguments;
    }

    /// <summary>
    /// Frame buffering setting.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct FrameBuffering
    {
        /// <summary>
        /// Buffering enabling.
        /// </summary>
        [MarshalAs(UnmanagedType.I4)]
        public Buffering Buffering;

        /// <summary>
        ///  Max buffering frame number.
        /// </summary>
        [MarshalAs(UnmanagedType.I4)]
        public int Num;

        /// <summary>
        /// Buffering format.
        /// </summary>
        [MarshalAs(UnmanagedType.I4)]
        public BufferingFormat Format;
    }

    /// <summary>
    /// Frame buffering.
    /// </summary>
    public enum Buffering
    {
        /// <summary>
        /// Use config.
        /// </summary>
        UseConfig = -2,

        /// <summary>
        /// Buffering default.
        /// </summary>
        Default = -1,

        /// <summary>
        /// Buffering disable.
        /// </summary>
        Off = 0,

        /// <summary>
        /// Buffering enable.
        /// </summary>
        On,
    }

    /// <summary>
    /// Frame buffering format.
    /// </summary>
    public enum BufferingFormat
    {
        /// <summary>
        /// Use config.
        /// </summary>
        UseConfig = -2,

        /// <summary>
        /// Default format.
        /// </summary>
        Default = -1,

        /// <summary>
        /// Discard the latest frame.
        /// </summary>
        Discard = 0,

        /// <summary>
        /// Overwrite the oldest frame.
        /// </summary>
        Overwrite,
    }

    /// <summary>
    /// Structure for version(native).
    /// </summary>
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    internal struct NativeVersion
    {
        /// <summary>
        /// Name.
        /// </summary>
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string name;

        /// <summary>
        /// Major version.
        /// </summary>
        [MarshalAs(UnmanagedType.U4)]
        public UInt32 major;

        /// <summary>
        /// Minor version.
        /// </summary>
        [MarshalAs(UnmanagedType.U4)]
        public UInt32 minor;

        /// <summary>
        /// Patch version.
        /// </summary>
        [MarshalAs(UnmanagedType.U4)]
        public UInt32 patch;

        /// <summary>
        /// Version description.
        /// </summary>
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)]
        public string description;
    }

    /// <summary>
    /// Structure for stream version(native).
    /// </summary>
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    internal struct NativeStreamVersion
    {
        /// <summary>
        /// Stream key.
        /// </summary>
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 64)]
        public string stream_key;

        /// <summary>
        /// Stream version.
        /// </summary>
        [MarshalAs(UnmanagedType.Struct)]
        public NativeVersion stream_version;

        /// <summary>
        /// Number of linkage versions.
        /// </summary>
        [MarshalAs(UnmanagedType.U4)]
        public UInt32 linkage_count;

        /// <summary>
        /// Stream linkage versions.
        /// </summary>
        [MarshalAs(UnmanagedType.SysInt)]
        public IntPtr linkage_versions;

        /// <summary>
        /// Destination ID.
        /// </summary>
        [MarshalAs(UnmanagedType.I4)]
        public Int32 destination_id;

        /// <summary>
        /// LinkageVersions (native type).
        /// </summary>
        /// <param name="index">Index of linkage version.</param>
        /// <returns>LinkageVersions structure.</returns>
        public NativeVersion LinkageVersions(int index)
        {
            return (NativeVersion)Marshal.PtrToStructure(
                IntPtr.Add(
                    this.linkage_versions,
                    Marshal.SizeOf(typeof(NativeVersion)) * index),
                typeof(NativeVersion));
        }
    }

    /// <summary>
    /// Structure for SensCord version(native).
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    internal struct NativeSensCordVersion
    {
        /// <summary>
        /// SensCord version.
        /// </summary>
        [MarshalAs(UnmanagedType.Struct)]
        public NativeVersion senscord_version;

        /// <summary>
        /// Project version.
        /// </summary>
        [MarshalAs(UnmanagedType.Struct)]
        public NativeVersion project_version;

        /// <summary>
        /// Number of streams.
        /// </summary>
        [MarshalAs(UnmanagedType.U4)]
        public UInt32 stream_count;

        /// <summary>
        /// Stream versions.
        /// </summary>
        [MarshalAs(UnmanagedType.SysInt)]
        public IntPtr stream_versions;

        /// <summary>
        /// Destination ID.
        /// </summary>
        [MarshalAs(UnmanagedType.I4)]
        public Int32 destination_id;

        /// <summary>
        /// Number of servers.
        /// </summary>
        [MarshalAs(UnmanagedType.U4)]
        public UInt32 server_count;

        /// <summary>
        /// Server versions.
        /// </summary>
        [MarshalAs(UnmanagedType.SysInt)]
        public IntPtr server_version;

        /// <summary>
        /// StreamVersion (native type).
        /// </summary>
        /// <param name="index">Index of stream version.</param>
        /// <returns>StreamVersion structure.</returns>
        public NativeStreamVersion StreamVersions(int index)
        {
            return (NativeStreamVersion)Marshal.PtrToStructure(
                IntPtr.Add(
                    this.stream_versions,
                    Marshal.SizeOf(typeof(NativeStreamVersion)) * index),
                typeof(NativeStreamVersion));
        }

        /// <summary>
        /// SensCordVersion (native type).
        /// </summary>
        /// <param name="index">Index of server.</param>
        /// <returns>SensCordVersion structure.</returns>
        public NativeSensCordVersion ServerVersions(int index)
        {
            return (NativeSensCordVersion)Marshal.PtrToStructure(
                IntPtr.Add(
                    this.server_version,
                    Marshal.SizeOf(typeof(NativeSensCordVersion)) * index),
                typeof(NativeSensCordVersion));
        }
    }
}
