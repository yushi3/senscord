/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using System;
using MessagePack;

namespace SensCord
{
    /// <summary>
    /// Serializer.
    /// </summary>
    internal sealed class Serializer
    {
        /// <summary>
        /// Serializer.
        /// </summary>
        private static Serializer singletonInstance = new Serializer();

        /// <summary>
        /// Get instance.
        /// </summary>
        internal static Serializer Instance
        {
            get { return singletonInstance; }
        }

        /// <summary>
        /// Constructor.
        /// </summary>
        private Serializer() { }

        /// <summary>
        /// Serialize the data.
        /// </summary>
        /// <typeparam name="T">Any class.</typeparam>
        /// <param name="data">Data to serialize.</param>
        /// <returns>Serialized byte array.</returns>
        internal byte[] Pack<T>(T data) where T : class
        {
            ////Console.WriteLine($"#DBG# Pack: {MessagePackSerializer.SerializeToJson(data)}");
            return MessagePackSerializer.Serialize<T>(data);
        }

        /// <summary>
        /// Serialize the data.
        /// </summary>
        /// <typeparam name="T">Any class.</typeparam>
        /// <param name="data">Data to serialize.</param>
        /// <param name="length">Length of byte array.</param>
        /// <returns>Serialized byte array.</returns>
        internal byte[] Pack<T>(T data, int length) where T : class
        {
            ////Console.WriteLine($"#DBG# Pack: {MessagePackSerializer.SerializeToJson(data)}");
            var bytes = MessagePackSerializer.Serialize<T>(data);
            if (bytes.Length < length)
            {
                Array.Resize(ref bytes, length);
            }
            return bytes;
        }

        /// <summary>
        /// Deserialize the byte array.
        /// </summary>
        /// <typeparam name="T">Any class.</typeparam>
        /// <param name="bytes">Byte array to deserialize.</param>
        /// <returns>Return the serialized data.</returns>
        internal T Unpack<T>(byte[] bytes)
        {
            T data = MessagePackSerializer.Deserialize<T>(bytes);
            ////Console.WriteLine($"#DBG# Unpack: {MessagePackSerializer.SerializeToJson(data)}");
            return data;
        }
    }
}
