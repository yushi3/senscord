/*
 * SPDX-FileCopyrightText: 2017-2019 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

using System;
using System.Collections.Generic;

namespace SensCord.Sample
{
    class SimpleStreamPlayer
    {
        private const string DefaultStreamKey = "pseudo_image_stream.0";
        private const int DefaultFrameCount = 100;

        private string streamKey;
        private int frameCount;

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
            // TODO: parse arguments (-k streamKey, -f frameCount)
            var streamKey = DefaultStreamKey;
            var frameCount = DefaultFrameCount;

            var player = new SimpleStreamPlayer(streamKey, frameCount);
            player.Run();
        }

        private SimpleStreamPlayer(string streamKey, int frameCount)
        {
            this.streamKey = streamKey;
            this.frameCount = frameCount;
        }

        private void Run()
        {
            Console.WriteLine("=== SimpleStream Player ===");

            using (var core = new Core())
            {
                var version = core.GetVersion();
                Console.WriteLine($"GetVersion(): {version}");

                // smoke test
                var streamInfoList = core.GetStreamList();
                Console.WriteLine($"GetStreamList(): {streamInfoList.Length}");
                foreach (var streamInfo in streamInfoList)
                {
                    Console.WriteLine($" - {streamInfo.Type}: {streamInfo.Key}");
                }

                // smoke test
                Console.WriteLine("GetOpenedStreamCount(): {0}",
                    core.GetOpenedStreamCount(streamKey));

                // smoke test
                OpenStreamSetting setting;
                setting.FrameBuffering.Buffering = Buffering.Off;
                setting.FrameBuffering.Num = 0;
                setting.FrameBuffering.Format = BufferingFormat.Default;

                using (var stream = core.OpenStream(streamKey, setting))
                {
                    // smoke test
                    Console.WriteLine("GetOpenedStreamCount(): {0}",
                        core.GetOpenedStreamCount(streamKey));

                    // smoke test
                    var propertyList = stream.GetPropertyList();
                    Console.WriteLine($"GetPropertyList(): {propertyList.Length}");
                    foreach (var propertyKey in propertyList)
                    {
                        Console.WriteLine($" - {propertyKey}");
                    }

                    // smoke test (ImageProperty : get, set)
                    try
                    {
                        // TODO: This try-catch is a temporary measure,
                        //                  when MsgPack is not implemented.
                        // get
                        var imageProperty = new ImageProperty();
                        stream.GetProperty(ref imageProperty);
                        Console.WriteLine($"GetProperty: {imageProperty}");

                        // set
                        imageProperty.Width = 1280;
                        imageProperty.Height = 720;
                        imageProperty.StrideBytes = 1280 * 1;
                        stream.SetProperty(imageProperty);

                        // get
                        imageProperty = stream.GetProperty<ImageProperty>();
                        Console.WriteLine($"GetProperty: {imageProperty}");
                    }
                    catch (ApiException error)
                    {
                        Console.WriteLine(error.StackTrace);
                    }

                    // smoke test (UserData)
                    try
                    {
                        var tmp = new List<byte>();
                        for (var i = 0; i < 16; ++i)
                        {
                            tmp.Add((byte)i);
                        }

                        var userData = new UserDataProperty();
                        userData.Data = tmp.ToArray();
                        Console.WriteLine($"SetProperty: {userData}");
                        stream.SetProperty(userData);

                        var userData2 = stream.GetProperty<UserDataProperty>();
                        Console.WriteLine($"GetProperty: {userData2}");
                    }
                    catch (ApiException error)
                    {
                        Console.WriteLine(error.StackTrace);
                    }

                    // smoke test
                    stream.RegisterFrameCallback(OnFrameReceived);
                    stream.RegisterEventCallback(OnEventReceived);

                    stream.Start();

                    for (var i = 0; i < frameCount; ++i)
                    {
                        using (var frame = stream.GetFrame())
                        {
                            ProcessFrame(frame);

                            // frame.Dispose() by using statement.
                        }

                        // smoke test
                        if (i == 20)
                        {
                            stream.RegisterFrameCallback(OnFrameReceived2);
                        }
                        if (i == 40)
                        {
                            stream.UnregisterFrameCallback(OnFrameReceived);
                        }
                        if (i == 60)
                        {
                            stream.UnregisterFrameCallback(OnFrameReceived2);
                        }
                        if (i == 80)
                        {
                            stream.RegisterFrameCallback((Stream unused) =>
                            {
                                Console.WriteLine($"OnFrameReceived(lambda) : {stream.Key}");
                            });
                        }
                    }

                    stream.Stop();

                    // smoke test
                    stream.UnregisterEventCallback(OnEventReceived);

                    // stream.Dispose() by using statement.
                }

                // core.Dispose() by using statement.
            }

            Console.WriteLine("=== SimpleStream End ===");
            Console.ReadKey();
        }

        private void ProcessFrame(Frame frame)
        {
            Console.WriteLine("GetFrame(): seq={0}, type={1}",
                frame.SequenceNumber, frame.Type);

            var userData = frame.GetUserData();
            Console.WriteLine(" - UserData: len={0}, [{1}]",
                userData.Length, string.Join(", ", userData));

            foreach (var channel in frame.Channels)
            {
                var rawData = channel.GetRawData();
                Console.WriteLine(" - [{0}]: {1}, length={2}",
                    channel.Id, rawData.Type, rawData.Length);

                // smoke test
                Console.WriteLine("   - property: {0}",
                    channel.PropertyList.Length);
                foreach (var propertyKey in channel.PropertyList)
                {
                    Console.WriteLine($"     - {propertyKey}");

                    // TODO: search property key ...
                    if (propertyKey.Equals(ImageProperty.ConstKey))
                    {
                        try
                        {
                            var imageProperty = channel.GetProperty<ImageProperty>();
                            Console.WriteLine(imageProperty);
                        }
                        catch (ApiException error)
                        {
                            Console.WriteLine(error.StackTrace);
                        }
                    }
                }

                // smoke test
                Console.WriteLine("   - updated property: {0}",
                    channel.UpdatedPropertyList.Length);
                foreach (var propertyKey in channel.UpdatedPropertyList)
                {
                    Console.WriteLine($"     - {propertyKey}");
                }
            }
        }

        private void OnFrameReceived(Stream stream)
        {
            Console.WriteLine($"OnFrameReceived: {stream.Key}");
        }

        private void OnFrameReceived2(Stream stream)
        {
            Console.WriteLine($"OnFrameReceived2: {stream.Key}");
        }

        private void OnEventReceived(Stream stream, string eventType)
        {
            Console.WriteLine($"OnEventReceived: {stream.Key}, event={eventType}");
        }
    }
}
