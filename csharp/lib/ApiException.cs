/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using System;

namespace SensCord
{
    /// <summary>
    /// SensCord C API exception class.
    /// </summary>
    public class ApiException : Exception
    {
        /// <summary>
        /// Level of error.
        /// </summary>
        public ErrorLevel Level { get; private set; }

        /// <summary>
        /// Cause of error.
        /// </summary>
        public ErrorCause Cause { get; private set; }

        /// <summary>
        /// Where the error occurred.
        /// </summary>
        public string Block { get; private set; }

        /// <summary>
        /// Trace information.
        /// </summary>
        public string Trace { get; private set; }

        /// <summary>
        /// Create a new ApiException.
        /// </summary>
        /// <param name="status">Error status.</param>
        public ApiException(Status status) : base(status.Message)
        {
            this.Level = status.Level;
            this.Cause = status.Cause;
            this.Block = status.Block;
            this.Trace = status.Trace;
        }
    }
}
