/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Structure containing information about the PCM.
    /// </summary>
    [MessagePackObject]
    public class AudioPcmProperty : IBaseProperty
    {
        /// <summary>
        /// Property key.
        /// </summary>
        public static readonly string ConstKey = "audio_pcm_property";

        /// <summary>
        /// Property key.
        /// </summary>
        [IgnoreMember]
        public string Key { get; } = ConstKey;

        /// <summary>
        /// Number of channels.
        /// </summary>
        [Key("channels")]
        public int Channels { get; set; }

        /// <summary>
        /// True: interleaved, False: non-interleaved.
        /// </summary>
        [Key("interleaved")]
        public bool Interleaved { get; set; }

        /// <summary>
        /// PCM format.
        /// </summary>
        [Key("format")]
        public AudioPcm.Format Format { get; set; }

        /// <summary>
        /// Number of samples per second (e.g. 8000, 44100, 48000, 96000, ...)
        /// </summary>
        [Key("samples_per_second")]
        public int SamplesPerSecond { get; set; }

        /// <summary>
        /// Number of samples per frame.
        /// </summary>
        [Key("samples_per_frame")]
        public int SamplesPerFrame { get; set; }

        /// <summary>
        /// To string.
        /// </summary>
        /// <returns>A string that represents the current object.</returns>
        public override string ToString()
        {
            return string.Format(
                "{0}(ch={1}, {2}, {3}, {4}Hz, {5} Samples/Frame)",
                this.Key, this.Channels, this.Interleaved, this.Format,
                this.SamplesPerSecond, this.SamplesPerFrame);
        }
    }

    /// <summary>
    /// PCM audio.
    /// </summary>
    public static class AudioPcm
    {
        /// <summary>
        /// PCM format.
        /// </summary>
        public enum Format
        {
            /// <summary>
            /// Unknown format.
            /// </summary>
            UNKNOWN = -1,

            /// <summary>
            /// Signed 8bit.
            /// </summary>
            S8 = 0,

            /// <summary>
            /// Unsigned 8bit.
            /// </summary>
            U8,

            /// <summary>
            /// Signed 16bit Little Endian.
            /// </summary>
            S16LE,

            /// <summary>
            /// Signed 16bit Big Endian.
            /// </summary>
            S16BE,

            /// <summary>
            /// Unsigned 16bit Little Endian.
            /// </summary>
            U16LE,

            /// <summary>
            /// Unsigned 16bit Big Endian.
            /// </summary>
            U16BE,

            /// <summary>
            /// Signed 24bit Little Endian (3 bytes format).
            /// </summary>
            S24LE3,

            /// <summary>
            /// Signed 24bit Big Endian (3 bytes format).
            /// </summary>
            S24BE3,

            /// <summary>
            /// Unsigned 24bit Little Endian (3 bytes format).
            /// </summary>
            U24LE3,

            /// <summary>
            /// Unsigned 24bit Big Endian (3 bytes format).
            /// </summary>
            U24BE3,

            /// <summary>
            /// Signed 24bit Little Endian (4 bytes format).
            /// </summary>
            S24LE,

            /// <summary>
            /// Signed 24bit Big Endian (4 bytes format).
            /// </summary>
            S24BE,

            /// <summary>
            /// Unsigned 24bit Little Endian (4 bytes format).
            /// </summary>
            U24LE,

            /// <summary>
            /// Unsigned 24bit Big Endian (4 bytes format).
            /// </summary>
            U24BE,

            /// <summary>
            /// Signed 32bit Little Endian.
            /// </summary>
            S32LE,

            /// <summary>
            /// Signed 32bit Big Endian.
            /// </summary>
            S32BE,

            /// <summary>
            /// Unsigned 32bit Little Endian.
            /// </summary>
            U32LE,

            /// <summary>
            /// Unsigned 32bit Big Endian.
            /// </summary>
            U32BE,

            /// <summary>
            /// Float 32bit Little Endian.
            /// </summary>
            FLOAT32LE,

            /// <summary>
            /// Float 32bit Big Endian.
            /// </summary>
            FLOAT32BE,

            /// <summary>
            /// Float 64bit Little Endian.
            /// </summary>
            FLOAT64LE,

            /// <summary>
            /// Float 64bit Big Endian.
            /// </summary>
            FLOAT64BE,
        }

        /// <summary>
        /// Returns the byte width.
        /// </summary>
        /// <param name="format">PCM format.</param>
        /// <returns>Returns the byte width.</returns>
        public static int ByteWidth(this Format format)
        {
            return Native.AudioPcmGetByteWidth((int)format);
        }

        /// <summary>
        /// Returns the number of bits per sample.
        /// </summary>
        /// <param name="format">PCM format.</param>
        /// <returns>Returns the number of bits per sample.</returns>
        public static int BitsPerSample(this Format format)
        {
            return Native.AudioPcmGetBitsPerSample((int)format);
        }

        /// <summary>
        /// Returns true if signed type.
        /// </summary>
        /// <param name="format">PCM format.</param>
        /// <returns>Returns true if signed type.</returns>
        public static bool IsSigned(this Format format)
        {
            return (Native.AudioPcmIsSigned((int)format) != 0);
        }

        /// <summary>
        /// Returns true if unsigned type.
        /// </summary>
        /// <param name="format">PCM format.</param>
        /// <returns>Returns true if unsigned type.</returns>
        public static bool IsUnsigned(this Format format)
        {
            return (Native.AudioPcmIsUnsigned((int)format) != 0);
        }

        /// <summary>
        /// Returns true if float type.
        /// </summary>
        /// <param name="format">PCM format.</param>
        /// <returns>Returns true if float type.</returns>
        public static bool IsFloat(this Format format)
        {
            return (Native.AudioPcmIsFloat((int)format) != 0);
        }

        /// <summary>
        /// Returns true if little endian.
        /// </summary>
        /// <param name="format">PCM format.</param>
        /// <returns>Returns true if little endian.</returns>
        public static bool IsLittleEndian(this Format format)
        {
            return (Native.AudioPcmIsLittleEndian((int)format) != 0);
        }

        /// <summary>
        /// Returns true if big endian.
        /// </summary>
        /// <param name="format">PCM format.</param>
        /// <returns>Returns true if big endian.</returns>
        public static bool IsBigEndian(this Format format)
        {
            return (Native.AudioPcmIsBigEndian((int)format) != 0);
        }
    }
}
