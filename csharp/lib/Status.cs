/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using System;
using System.Runtime.InteropServices;

namespace SensCord
{
    /// <summary>
    /// Level of error.
    /// (high) Fatal > Fail > Undefined (low).
    /// </summary>
    public enum ErrorLevel
    {
        /// <summary>
        /// Undefined.
        /// </summary>
        Undefined = 0,

        /// <summary>
        /// Fail.
        /// </summary>
        Fail,

        /// <summary>
        /// Fatal.
        /// </summary>
        Fatal,
    }

    /// <summary>
    /// Cause of error.
    /// </summary>
    public enum ErrorCause
    {
        /// <summary>
        /// None.
        /// </summary>
        None = 0,

        /// <summary>
        /// Not found.
        /// </summary>
        NotFound,

        /// <summary>
        /// Invalid argument.
        /// </summary>
        InvalidArgument,

        /// <summary>
        /// Resource exhausted.
        /// </summary>
        ResourceExhausted,

        /// <summary>
        /// Permission denied.
        /// </summary>
        PermissionDenied,

        /// <summary>
        /// Busy.
        /// </summary>
        Busy,

        /// <summary>
        /// Timeout.
        /// </summary>
        Timeout,

        /// <summary>
        /// Cancelled.
        /// </summary>
        Cancelled,

        /// <summary>
        /// Aborted.
        /// </summary>
        Aborted,

        /// <summary>
        /// Already exists.
        /// </summary>
        AlreadyExists,

        /// <summary>
        /// Invalid operation.
        /// </summary>
        InvalidOperation,

        /// <summary>
        /// Out of range.
        /// </summary>
        OutOfRange,

        /// <summary>
        /// Data loss.
        /// </summary>
        DataLoss,

        /// <summary>
        /// Hardware error.
        /// </summary>
        HardwareError,

        /// <summary>
        /// Not supported.
        /// </summary>
        NotSupported,

        /// <summary>
        /// Unknown.
        /// </summary>
        Unknown,
    }

    /// <summary>
    /// Error status.
    /// </summary>
    [StructLayout(LayoutKind.Sequential)]
    public struct Status
    {
        /// <summary>
        /// Level of error.
        /// </summary>
        [MarshalAs(UnmanagedType.I4)]
        public ErrorLevel level;

        /// <summary>
        /// Cause of error.
        /// </summary>
        [MarshalAs(UnmanagedType.I4)]
        public ErrorCause cause;

        /// <summary>
        /// Error massage.
        /// </summary>
        [MarshalAs(UnmanagedType.SysInt)]
        private IntPtr messagePtr;

        /// <summary>
        /// Where the error occurred.
        /// </summary>
        [MarshalAs(UnmanagedType.SysInt)]
        private IntPtr blockPtr;

        /// <summary>
        /// Trace information.
        /// </summary>
        [MarshalAs(UnmanagedType.SysInt)]
        private IntPtr tracePtr;

        /// <summary>
        /// Level of error.
        /// </summary>
        public ErrorLevel Level { get { return this.level; } }

        /// <summary>
        /// Cause of error.
        /// </summary>
        public ErrorCause Cause { get { return this.cause; } }

        /// <summary>
        /// Error massage.
        /// </summary>
        public string Message { get { return Native.PtrToString(this.messagePtr); } }

        /// <summary>
        /// Where the error occurred.
        /// </summary>
        public string Block { get { return Native.PtrToString(this.blockPtr); } }

        /// <summary>
        /// Trace information.
        /// </summary>
        public string Trace { get { return Native.PtrToString(this.tracePtr); } }
    }
}
