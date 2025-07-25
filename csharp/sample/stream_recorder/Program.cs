/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using System;

namespace SensCord.Sample
{
    class StreamRecorder
    {
        private const string DefaultStreamKey = "pseudo_image_stream.0";
        // private const string DefaultStreamKey = "webcam_image_stream.0";
        private const int DefaultFrameCount = 20;
        private const string DefaultFormatType = "raw";
        private const string DefaultOutputPath = ".";
        private const string DefaultNameRule = "";
        private const int GetFrameWaitMsec = 3000;

        private string streamKey;
        private int frameCount;
        private string formatType;
        private string outputPath;
        private string nameRule;
        private bool noVender;
        private bool silent;

        static void Main(string[] args)
        {
#if false
            // TODO: PATH (senscord.dll, senscord_osal.dll)
            var path = System.Environment.GetEnvironmentVariable("PATH");
            System.Environment.SetEnvironmentVariable("PATH",
                path + ";" + "C:\\senscord\\bin");
#endif
#if false
            // TODO: SENSCORD_FILE_PATH
            SensCord.Environment.SetSensCordFilePath(
                "C:\\senscord\\config" + ";" +
                "C:\\senscord\\bin\\component" + ";" +
                "C:\\senscord\\bin\\allocator" + ";" +
                "C:\\senscord\\bin\\recorder" + ";" +
                "C:\\senscord\\bin\\connection");
#endif
            // TODO: parse arguments.
            var streamKey = DefaultStreamKey;
            var frameCount = DefaultFrameCount;
            var formatType = DefaultFormatType;
            var outputPath = DefaultOutputPath;
            var nameRule = DefaultNameRule;
            var noVender = true;
            var silent = true;

            var recorder = new StreamRecorder(
                streamKey, frameCount, formatType, outputPath, nameRule,
                noVender, silent);
            recorder.Run();
        }

        private StreamRecorder(
            string streamKey, int frameCount, string formatType,
            string outputPath, string nameRule, bool noVender, bool silent)
        {
            this.streamKey = streamKey;
            this.frameCount = frameCount;
            this.formatType = formatType;
            this.outputPath = outputPath;
            this.nameRule = nameRule;
            this.noVender = noVender;
            this.silent = silent;
        }

        private void Run()
        {
            Console.WriteLine("=== Simple Stream Recorder ===");

            using (var core = new Core())
            {
                var version = core.GetVersion();
                Console.WriteLine($"GetVersion(): {version}");

                using (var stream = core.OpenStream(streamKey))
                {
                    ExecuteRecording(stream);
                }  // stream.Dispose() by using statement.
            }  // core.Dispose() by using statement.

            Console.WriteLine("=== Simple Stream Recorder End ===");
        }

        private void ExecuteRecording(Stream stream)
        {
            // get recorder list.
            var recorderList = stream.GetProperty<RecorderListProperty>();
            Console.WriteLine(recorderList);

            // get channel info (total channel num)
            var channelInfo = stream.GetProperty<ChannelInfoProperty>();
            Console.WriteLine($"Channel num: {channelInfo.Channels.Count}");

            // start stream.
            stream.Start();

            // start recording
            var recordProp = new RecordProperty
            {
                Enabled = true,
                Path = outputPath,
                BufferNum = 5
            };

            // channels
            foreach (var channel in channelInfo.Channels)
            {
                // TODO: define ChannelIdVendorBase (0x80000000)
                if (noVender && channel.Key >= 0x80000000) continue;
                recordProp.Formats[channel.Key] = formatType;
            }

            // name rules
            recordProp.NameRules[SensCord.RecordDirectory.Top] = nameRule;

            Console.WriteLine(recordProp);
            stream.SetProperty(recordProp);

            for (var cnt = 0; cnt < frameCount; ++cnt)
            {
                using (var frame = stream.GetFrame(GetFrameWaitMsec))
                {
                    if (!silent)
                    {
                        Console.WriteLine($"seq={frame.SequenceNumber}");
                    }
                }  // frame.Dispose() by using statement.
            }

            Console.WriteLine("Done.");
            stream.Stop();
        }
    }
}
