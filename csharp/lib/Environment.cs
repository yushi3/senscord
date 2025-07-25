/*
 * SPDX-FileCopyrightText: 2017-2022 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using System;

namespace SensCord
{
    /// <summary>
    /// Environment variable utility class.
    /// </summary>
    public static class Environment
    {
        /// <summary>
        /// Set the file search paths.
        /// Use instead of SENSCORD_FILE_PATH.
        /// </summary>
        /// <param name="paths">The same format as SENSCORD_FILE_PATH.</param>
        /// <exception cref="ApiException">If an error occurred.</exception>
        public static void SetSensCordFilePath(string paths)
        {
            if (Native.SetFileSearchPath(paths) != 0)
            {
                throw new ApiException(Native.GetLastError());
            }
        }
    }
}
