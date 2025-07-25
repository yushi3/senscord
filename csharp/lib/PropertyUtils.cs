/*
 * SPDX-FileCopyrightText: 2022-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using System;
using System.Collections.Generic;
using System.Text;

namespace SensCord
{
    /// <summary>
    /// Utilities API for Property.
    /// </summary>
    public class PropertyUtilsParam
    {
        /// <summary>
        /// Additional information identifier for channel id.
        /// </summary>
        public static readonly string AppendInfoChannel = "ch=";

        /// <summary>
        /// Use property key.
        /// </summary>
        private string key = string.Empty;

        /// <summary>
        /// Append information table.
        /// </summary>
        private Dictionary<string, string> appendDict;

        /// <summary>
        /// Constructor.
        /// </summary>
        /// <param name="in_key">The key of property.</param>
        public PropertyUtilsParam(string in_key)
        {
            this.appendDict = new Dictionary<string, string>();
            this.ParseKey(in_key);
        }

        /// <summary>
        /// Parse the property key.
        /// </summary>
        /// <param name="in_key">The key of property.</param>
        private void ParseKey(string in_key)
        {
            Int32 spos = in_key.IndexOf("[", System.StringComparison.CurrentCulture);
            Int32 epos = in_key.LastIndexOf("]", System.StringComparison.CurrentCulture);

            if (spos == -1)
            {
                if (epos == -1)
                {
                    this.key = in_key;
                }
                else
                {
                    this.key = string.Empty;
                }
            }
            else
            {
                if (epos != (in_key.Length - 1))
                {
                    this.key = string.Empty;
                }
                else if (spos == 0)
                {
                    this.key = string.Empty;
                }
                else
                {
                    this.key = in_key.Substring(0, spos);
                    this.ParseAppendInfo(in_key);
                }
            }
        }

        /// <summary>
        /// Make property key with append information.
        /// </summary>
        /// <returns>Made append information.</returns>
        public string GetFullKey()
        {
            if (this.key == string.Empty)
            {
                return string.Empty;
            }

            Int32 count = 0;
            var joint = new StringBuilder();
            joint.Append(this.key);
            joint.Append("[");
            foreach (KeyValuePair<string, string> item in this.appendDict)
            {
                joint.Append(item.Key);
                joint.Append(item.Value);
                ++count;
                if (count < this.appendDict.Count)
                {
                    joint.Append(",");
                }
            }
            joint.Append("]");
            return joint.ToString();
        }

        /// <summary>
        /// Parse the append information from property key.
        /// </summary>
        /// <param name="in_key">The key of property.</param>
        private void ParseAppendInfo(string in_key)
        {
            Int32 last_spos = in_key.LastIndexOf("[", System.StringComparison.CurrentCulture);
            Int32 first_epos = in_key.IndexOf("]", System.StringComparison.CurrentCulture);

            string extract_append =
                in_key.Substring(last_spos + 1, first_epos - last_spos - 1);

            string[] append_split = extract_append.Split(',');
            foreach (var append in append_split)
            {
                Int32 pos = append.IndexOf("=", System.StringComparison.CurrentCulture);
                if (pos != -1)
                {
                    this.appendDict[append.Substring(0, pos + 1)] =
                        append.Substring(pos + 1);
                }
            }
        }

        /// <summary>
        /// Set Channel ID value.
        /// </summary>
        /// <param name="value">Append information tag value.</param>
        public void SetChannelId(long value)
        {
            this.appendDict[AppendInfoChannel] = value.ToString();
        }
    }

    /// <summary>
    /// Utilities API for Property.
    /// </summary>
    public class PropertyUtils
    {
        /// <summary>
        /// Set Channel ID to property key.
        /// </summary>
        /// <param name="key">The key of property.</param>
        /// <param name="channelId">Channel ID to be set to key.</param>
        /// <returns>Made property key (key + channel_id).</returns>
        public static string SetChannelId(string key, long channelId)
        {
            if (string.IsNullOrEmpty(key))
            {
                return string.Empty;
            }

            PropertyUtilsParam utils = new PropertyUtilsParam(key);
            utils.SetChannelId(channelId);
            return utils.GetFullKey();
        }
    }
}