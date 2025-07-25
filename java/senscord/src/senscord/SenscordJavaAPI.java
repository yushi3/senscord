/*
 * SPDX-FileCopyrightText: 2019-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

package senscord;

import com.sun.jna.Callback;
import com.sun.jna.Library;
import com.sun.jna.Native;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import com.sun.jna.ptr.ByteByReference;
import com.sun.jna.ptr.DoubleByReference;
import com.sun.jna.ptr.FloatByReference;
import com.sun.jna.ptr.IntByReference;
import com.sun.jna.ptr.LongByReference;
import com.sun.jna.ptr.PointerByReference;
import com.sun.jna.ptr.ShortByReference;

import senscord_properties.AudioPcmProperty;
import senscord_types.KeyPointData;
import senscord_types.ObjectDetectionData;
import senscord_types.ObjectTrackingData;
import senscord_types.OpenStreamSetting;
import senscord_types.PoseMatrixData;
import senscord_types.PoseQuaternionData;
import senscord_types.RawData;
import senscord_types.RotationData;
import senscord_types.SensCordVersion;
import senscord_types.Status;
import senscord_types.StreamTypeInfo;
import senscord_types.UserData;
import senscord_types.Vector3Float;

public class SenscordJavaAPI {
    /**
     * Senscord API interface.
     *
     */
    public interface SenscordCLibrary extends Library {
        @SuppressWarnings("deprecation")
        SenscordCLibrary INSTANCE =
                (SenscordCLibrary) Native.loadLibrary(System.getenv("SENSCORD_LIBRARY_PATH"),
                        SenscordJavaAPI.SenscordCLibrary.class);

        /* Common */

        /**
         * Get information on the last error that occurred.
         *
         * @return Error status.
         */
        Status.ByValue senscord_get_last_error();

        /**
         * Set the file search paths.
         * Use instead of SENSCORD_FILE_PATH.
         *
         * @param paths The same format as SENSCORD_FILE_PATH.
         * @return 0 is success or minus is failed (error code).
         */
        int senscord_set_file_search_path(String paths);

        /* Core */

        /**
         * Initialize Core, called at once.
         *
         * @param core Core handle.
         * @return 0 is success or minus is failed (error code).
         * @see "Core::Init"
         */
        int senscord_core_init(PointerByReference core);

        /**
         * Finalize Core and close all opened streams.
         *
         * @param core Core handle.
         * @return 0 is success or minus is failed (error code).
         * @see "Core::Exit"
         */
        int senscord_core_exit(Pointer core);

        /**
         * Open the new stream from key.
         *
         * @param core Core handle.
         * @param stream_key The key of the stream to open.
         * @param stream The new stream handle.
         * @return 0 is success or minus is failed (error code).
         * @see "Core::OpenStream"
         */
        int senscord_core_open_stream(Pointer core, String stream_key, PointerByReference stream);

        /**
         * Open the new stream from key and specified configuration.
         *
         * @param core Core handle.
         * @param stream_key The key of the stream to open.
         * @param setting Configuration to open stream.
         * @param stream The new stream handle.
         * @return 0 is success or minus is failed (error code).
         * @see "Core::OpenStream"
         */
        int senscord_core_open_stream_with_setting(Pointer core, String stream_key,
                OpenStreamSetting.ByReference setting, PointerByReference stream);

        /**
         * Close the opened stream.
         *
         * @param core Core handle.
         * @param stream The opened stream handle.
         * @return 0 is success or minus is failed (error code).
         * @see "Core::CloseStream"
         */
        int senscord_core_close_stream(Pointer core, Pointer stream);

        /**
         * Get count of supported streams list.
         *
         * @param core Core handle.
         * @param count Count of supported streams list.
         * @return 0 is success or minus is failed (error code).
         * @see "Core::GetStreamList"
         */
        int senscord_core_get_stream_count(Pointer core, IntByReference count);

        /**
         * Get supported stream information.
         *
         * @param core Core handle.
         * @param index Index of supported streams list. (min=0, max=count-1)
         * @param stream_info Location of stream information.
         * @return 0 is success or minus is failed (error code).
         * @see "Core::GetStreamList"
         */
        int senscord_core_get_stream_info(Pointer core, int index,
                StreamTypeInfo.ByReference stream_info);

        /**
         * Get count of opened stream.
         *
         * @param core Core handle.
         * @param stream_key Stream key.
         * @param count Count of opened stream.
         * @return 0 is success or minus is failed (error code).
         * @see "Core::GetOpenedStreamCount"
         */
        int senscord_core_get_opened_stream_count(Pointer core, String stream_key,
                IntByReference count);

        /**
         * Get the version of this core library.
         *
         * @param core Core handle.
         * @param version The version of this core library.
         * @return 0 is success or minus is failed (error code).
         * @see "Core::GetVersion"
         */
        int senscord_core_get_version(Pointer core, SensCordVersion.ByReference version);

        /* Stream */

        /**
         * Start stream.
         *
         * @param stream Stream handle.
         * @return 0 is success or minus is failed (error code).
         * @see "Stream::Start"
         */
        int senscord_stream_start(Pointer stream);

        /**
         * Stop stream.
         *
         * @param stream Stream handle.
         * @return 0 is success or minus is failed (error code).
         * @see "Stream::Stop"
         */
        int senscord_stream_stop(Pointer stream);

        /**
         * Get the received frame.
         *
         * @param stream Stream handle.
         * @param frame Location of received frame.
         * @param timeout_msec Time of wait msec if no received. 0 is polling, minus is forever.
         * @return 0 is success or minus is failed (error code).
         * @see "Stream::GetFrame"
         * @see "SENSCORD_TIMEOUT_POLLING, SENSCORD_TIMEOUT_FOREVER"
         */
        int senscord_stream_get_frame(Pointer stream, PointerByReference frame, int timeout_msec);

        /**
         * Release the gotten frame.
         *
         * @param stream Stream handle.
         * @param frame Received frame by senscord_stream_get_frame().
         * @return 0 is success or minus is failed (error code).
         * @see "Stream::ReleaseFrame"
         */
        int senscord_stream_release_frame(Pointer stream, Pointer frame);

        /**
         * Release the gotten frame.
         * Use this function if you do not refer to the raw data of the channel.
         *
         * @param stream Stream handle.
         * @param frame Received frame by senscord_stream_get_frame().
         * @return 0 is success or minus is failed (error code).
         * @see "Stream::ReleaseFrameUnused"
         */
        int senscord_stream_release_frame_unused(Pointer stream, Pointer frame);

        /**
         * Clear frames have not gotten.
         *
         * @param stream Stream handle.
         * @param frame_number Number of cleared frames. (optional)
         * @return 0 is success or minus is failed (error code).
         * @see "Stream::ClearFrames"
         */
        int senscord_stream_clear_frames(Pointer stream, IntByReference frame_number);

        public interface senscord_frame_callback_interface extends Callback {
            /**
             * Frame received callback function.
             *
             * @param stream Stream handle.
             * @param arg Private data.
             * @see senscord_stream_register_frame_callback
             */
            void invoke(Pointer stream, Pointer arg);
        }

        /**
         * Register the callback for frame reached.
         *
         * @param stream Stream handle.
         * @param callback Function pointer.
         * @param arg Private data with callback.
         * @return 0 is success or minus is failed (error code).
         * @see "Stream::RegisterFrameCallback"
         */
        int senscord_stream_register_frame_callback(Pointer stream,
                senscord_frame_callback_interface callback, Pointer arg);

        /**
         * Unregister the callback for frame reached.
         *
         * @param stream Stream handle.
         * @return 0 is success or minus is failed (error code).
         * @see "Stream::UnregisterFrameCallback"
         */
        int senscord_stream_unregister_frame_callback(Pointer stream);


        /**
         * Event definitions.
         *
         * @see "senscord::kEventError, senscord::kEventFrameDropped, etc."
         */
        public static interface Event {
            public static final String SENSCORD_EVENT_ANY = "EventAny";
            public static final String SENSCORD_EVENT_ERROR = "EventError";
            public static final String SENSCORD_EVENT_FATAL = "EventFatal";
            public static final String SENSCORD_EVENT_FRAME_DROPPED = "EventFrameDropped";
            public static final String SENSCORD_EVENT_PROPERTY_UPDATED = "EventPropertyUpdated";
            public static final String SENSCORD_EVENT_PLUGGED = "EventPlugged";
            public static final String SENSCORD_EVENT_UNPLUGGED = "EventUnplugged";
            public static final String SENSCORD_EVENT_RECORD_STATE = "EventRecordState";
        }

        /**
         * Event argument definitions.
         */
        public static interface EventArgumentKey {
            public static final String SENSCORD_EVENT_ARGUMENT_CAUSE = "cause";
            public static final String SENSCORD_EVENT_ARGUMENT_MESSAGE = "message";
            public static final String SENSCORD_EVENT_ARGUMENT_SEQUENCE_NUMBER = "sequence_number";
            public static final String SENSCORD_EVENT_ARGUMENT_PROPERTY_KEY = "property_key";
            public static final String SENSCORD_EVENT_ARGUMENT_RECORD_STATE = "state";
            public static final String SENSCORD_EVENT_ARGUMENT_RECORD_COUNT = "count";
            public static final String SENSCORD_EVENT_ARGUMENT_RECORD_PATH = "path";
        }

        public interface senscord_event_received_callback_interface extends Callback {
            /**
             * Event received callback function.
             *
             * @param event Event type to receive.
             * @param param Function pointer.
             * @param priv Private data.
             * @see "senscord_stream_register_event_callback"
             * @see "SENSCORD_EVENT_ERROR, SENSCORD_EVENT_FRAME_DROPPED, etc."
             */
            void invoke(String event, Pointer param, Pointer priv);
        }

        public interface senscord_event_received_callback2_interface extends Callback {
            /**
             * Event received callback function.
             *
             * @param stream        Stream handle.
             * @param event_type    Event type to receive.
             * @param args          Event argument handle.
             * @param private_data  Private data.
             * @see "senscord_stream_register_event_callback2"
             * @see "SENSCORD_EVENT_ERROR, SENSCORD_EVENT_FRAME_DROPPED, etc."
             */
            void invoke(Pointer stream, String event_type, Pointer args, Pointer private_data);
        }

        /**
         * Register the callback for event receiving.
         *
         * @param Stream Stream handle.
         * @param event_type Event type to receive.
         * @param callback Function pointer.
         * @param data Private data with callback.
         * @return 0 is success or minus is failed (error code).
         * @see "Stream::RegisterEventCallback"
         * @deprecated Register the callback for event receiving.
         */
        int senscord_stream_register_event_callback(Pointer Stream, String event_type,
                senscord_event_received_callback_interface callback, Pointer data);

        /**
         * Register the callback for event receiving.
         *
         * @param stream Stream handle.
         * @param event_type Event type to receive.
         * @param callback Function pointer.
         * @param private_data Private data with callback.
         * @return 0 is success or minus is failed (error code).
         * @see "Stream::RegisterEventCallback"
         */
        int senscord_stream_register_event_callback2(Pointer stream, String event_type,
                senscord_event_received_callback2_interface callback, Pointer private_data);

        /**
         * Unregister the event callback.
         *
         * @param Stream Stream handle.
         * @param event_type Event type to receive.
         * @return 0 is success or minus is failed (error code).
         * @see "Stream::UnregisterEventCallback"
         */
        int senscord_stream_unregister_event_callback(Pointer Stream, String event_type);

        /**
         * Raw data types.
         */
        public static interface RawDataType {
            public static final String SENSCORD_RAW_DATA_TYPE_IMAGE = "image_data";
            public static final String SENSCORD_RAW_DATA_TYPE_META = "meta_data";
            public static final String SENSCORD_RAW_DATA_TYPE_DEPTH = "depth_data";
            public static final String SENSCORD_RAW_DATA_TYPE_CONFIDENCE = "confidence_data";
            public static final String SENSCORD_RAW_DATA_TYPE_ACCELERATION = "acceleration_data";
            public static final String SENSCORD_RAW_DATA_TYPE_ANGULAR_VELOCITY = "angular_velocity_data";
            public static final String SENSCORD_RAW_DATA_TYPE_MAGNETIC_FIELD = "magnetic_field_data";
            public static final String SENSCORD_RAW_DATA_TYPE_ROTATION = "rotation_data";
            public static final String SENSCORD_RAW_DATA_TYPE_POSE = "pose_data";
            public static final String SENSCORD_RAW_DATA_TYPE_POINT_CLOUD = "point_cloud_data";
            public static final String SENSCORD_RAW_DATA_TYPE_GRID_MAP = "grid_map_data";
            public static final String SENSCORD_RAW_DATA_TYPE_OBJECT_DETECTION = "object_detection_data";
            public static final String SENSCORD_RAW_DATA_TYPE_KEY_POINT = "key_point_data";
            public static final String SENSCORD_RAW_DATA_TYPE_TEMPORAL_CONTRAST = "pixel_polarity_data";
            public static final String SENSCORD_RAW_DATA_TYPE_OBJECT_TRACKING = "object_tracking_data";
            public static final String SENSCORD_RAW_DATA_TYPE_AUDIO = "audio_data";
        }

        /**
         * Stream types.
         */
        public static interface StreamType {
            public static final String SENSCORD_STREAM_TYPE_IMAGE = "image";
            public static final String SENSCORD_STREAM_TYPE_DEPTH = "depth";
            public static final String SENSCORD_STREAM_TYPE_IMU = "imu";
            public static final String SENSCORD_STREAM_TYPE_SLAM = "slam";
            public static final String SENSCORD_STREAM_TYPE_OBJECT_DETECTION = "object_detection";
            public static final String SENSCORD_STREAM_TYPE_KEY_POINT = "key_point";
            public static final String SENSCORD_STREAM_TYPE_TEMPORAL_CONTRAST = "pixel_polarity";
            public static final String SENSCORD_STREAM_TYPE_OBJECT_TRACKING = "object_tracking";
            public static final String SENSCORD_STREAM_TYPE_AUDIO = "audio";
        }

        /**
         * Property keys.
         */
        public static interface PropertyKey {
            public static final String SENSCORD_VERSION_PROPERTY_KEY = "version_property";
            public static final String SENSCORD_STREAM_TYPE_PROPERTY_KEY = "stream_type_property";
            public static final String SENSCORD_STREAM_KEY_PROPERTY_KEY = "stream_key_property";
            public static final String SENSCORD_STREAM_STATE_PROPERTY_KEY = "stream_state_property";
            public static final String SENSCORD_FRAME_BUFFERING_PROPERTY_KEY =
                    "frame_buffering_property";
            public static final String SENSCORD_CURRENT_FRAME_NUM_PROPERTY_KEY =
                    "current_frame_num_property";
            public static final String SENSCORD_CHANNEL_INFO_PROPERTY_KEY = "channel_info_property";
            public static final String SENSCORD_CHANNEL_MASK_PROPERTY_KEY = "channel_mask_property";
            public static final String SENSCORD_PRESET_LIST_PROPERTY_KEY = "preset_list_property";
            public static final String SENSCORD_PRESET_PROPERTY_KEY = "preset_property";
            public static final String SENSCORD_IMAGE_PROPERTY_KEY = "image_property";
            public static final String SENSCORD_IMAGE_CROP_PROPERTY_KEY = "image_crop_property";
            public static final String SENSCORD_IMAGE_CROP_BOUNDS_PROPERTY_KEY =
                    "image_crop_bounds_property";
            public static final String SENSCORD_CONFIDENCE_PROPERTY_KEY = "confidence_property";
            public static final String SENSCORD_COLOR_SPACE_PROPERTY_KEY = "color_space_property";
            public static final String SENSCORD_FRAME_RATE_PROPERTY_KEY = "frame_rate_property";
            public static final String SENSCORD_SKIP_FRAME_PROPERTY_KEY = "skip_frame_property";
            public static final String SENSCORD_LENS_PROPERTY_KEY = "lens_property";
            public static final String SENSCORD_DEPTH_PROPERTY_KEY = "depth_property";
            public static final String SENSCORD_IMAGE_SENSOR_FUNCTION_SUPPORTED_PROPERTY_KEY =
                    "image_sensor_function_supported_property";
            public static final String SENSCORD_IMAGE_SENSOR_FUNCTION_PROPERTY_KEY =
                    "image_sensor_function_property";
            public static final String SENSCORD_EXPOSURE_PROPERTY_KEY = "exposure_property";
            public static final String SENSCORD_WHITE_BALANCE_PROPERTY_KEY =
                    "white_balance_property";
            public static final String SENSCORD_CAMERA_CALIBRATION_PROPERTY_KEY =
                    "camera_calibration_property";
            public static final String SENSCORD_INTERLACE_INFO_PROPERTY_KEY =
                    "interlace_info_property";
            public static final String SENSCORD_INTERLACE_PROPERTY_KEY = "interlace_property";
            public static final String SENSCORD_BASELINE_LENGTH_PROPERTY_KEY =
                    "baseline_length_property";
            public static final String SENSCORD_IMU_DATA_UNIT_PROPERTY_KEY =
                    "imu_data_unit_property";
            public static final String SENSCORD_SAMPLING_FREQUENCY_PROPERTY_KEY =
                    "sampling_frequency_property";
            public static final String SENSCORD_ACCELEROMETER_RANGE_PROPERTY_KEY =
                    "accelerometer_range_property";
            public static final String SENSCORD_GYROMETER_RANGE_PROPERTY_KEY =
                    "gyrometer_range_property";
            public static final String SENSCORD_MAGNETOMETER_RANGE_PROPERTY_KEY =
                    "magnetometer_range_property";
            public static final String SENSCORD_MAGNETOMETER_RANGE3_PROPERTY_KEY =
                    "magnetometer_range3_property";
            public static final String SENSCORD_ACCELERATION_CALIB_PROPERTY_KEY =
                    "acceleration_calib_property";
            public static final String SENSCORD_ANGULAR_VELOCITY_CALIB_PROPERTY_KEY =
                    "angular_velocity_calib_property";
            public static final String SENSCORD_MAGNETIC_FIELD_CALIB_PROPERTY_KEY =
                    "magnetic_field_calib_property";
            public static final String SENSCORD_MAGNETIC_NORTH_CALIB_PROPERTY_KEY =
                    "magnetic_north_calib_property";
            public static final String SENSCORD_SLAM_DATA_SUPPORTED_PROPERTY_KEY =
                    "slam_data_supported_property";
            public static final String SENSCORD_INITIAL_POSE_PROPERTY_KEY = "initial_pose_property";
            public static final String SENSCORD_POSE_DATA_PROPERTY_KEY = "pose_data_property";
            public static final String SENSCORD_ODOMETRY_DATA_PROPERTY_KEY =
                    "odometry_data_property";
            public static final String SENSCORD_GRID_MAP_PROPERTY_KEY = "grid_map_property";
            public static final String SENSCORD_GRID_SIZE_PROPERTY_KEY = "grid_size_property";
            public static final String SENSCORD_POINT_CLOUD_PROPERTY_KEY = "point_cloud_property";
            public static final String SENSCORD_REGISTER_ACCESS_64_PROPERTY_KEY =
                    "register_access_64_property";
            public static final String SENSCORD_REGISTER_ACCESS_32_PROPERTY_KEY =
                    "register_access_32_property";
            public static final String SENSCORD_REGISTER_ACCESS_16_PROPERTY_KEY =
                    "register_access_16_property";
            public static final String SENSCORD_REGISTER_ACCESS_8_PROPERTY_KEY =
                    "register_access_8_property";
            public static final String SENSCORD_USER_DATA_PROPERTY_KEY = "user_data_property";
            public static final String SENSCORD_RECORDER_LIST_PROPERTY_KEY =
                    "recorder_list_property";
            public static final String SENSCORD_RECORD_PROPERTY_KEY = "record_property";
            public static final String SENSCORD_PLAY_FILE_INFO_PROPERTY_KEY =
                    "play_file_info_property";
            public static final String SENSCORD_PLAY_MODE_PROPERTY_KEY = "play_mode_property";
            public static final String SENSCORD_PLAY_PAUSE_PROPERTY_KEY =
                    "play_pause_property";
            public static final String SENSCORD_PLAY_PROPERTY_KEY = "play_property";
            public static final String SENSCORD_PLAY_POSITION_PROPERTY_KEY =
                    "play_position_property";
            public static final String SENSCORD_ROI_PROPERTY_KEY = "roi_property";
            public static final String SENSCORD_TEMPORAL_CONTRAST_DATA_PROPERTY_KEY =
                    "pixel_polarity_data_property";
            public static final String SENSCORD_PIXEL_POLARITY_DATA_PROPERTY_KEY =
                    "pixel_polarity_data_property";
            public static final String SENSCORD_SCORE_THRESHOLD_PROPERTY_KEY =
                    "score_threshold_property";
            public static final String SENSCORD_VELOCITY_DATA_UNIT_PROPERTY_KEY =
                    "velocity_data_unit_property";
            public static final String SENSCORD_DATA_RATE_PROPERTY_KEY = "data_rate_property";
            public static final String SENSCORD_COORDINATE_SYSTEM_PROPERTY_KEY =
                    "coordinate_system_property";
            public static final String SENSCORD_AUDIO_PROPERTY_KEY = "audio_property";
            public static final String SENSCORD_AUDIO_PCM_PROPERTY_KEY = "audio_pcm_property";
        }

        /**
         * Get the property.
         *
         * @param stream Stream handle.
         * @param property_key Key of property to get.
         * @param value Pointer to the structure of the property.
         * @param size Size of property structure.
         * @return 0 is success or minus is failed (error code).
         * @see "Stream::GetProperty"
         */
        int senscord_stream_get_property(Pointer stream, String property_key,
                Structure.ByReference value, int size);

        /**
         * Set the property with key.
         *
         * @param stream Stream handle.
         * @param property_key Key of property to set.
         * @param value Pointer to the structure of the property.
         * @param size Size of property structure.
         * @return 0 is success or minus is failed (error code).
         * @see "Stream::SetProperty"
         */
        int senscord_stream_set_property(Pointer stream, String property_key,
                Structure.ByReference value, int size);

        /**
         * Get the serialized property.
         *
         * @param stream Stream handle.
         * @param property_key Key of property to get.
         * @param buffer Buffer that stores output property values.
         * @param buffer_size Buffer size.
         * @param output_size Size of output property. (optional)
         * @return 0 is success or minus is failed (error code).
         * @see "Stream::GetProperty"
         */
        int senscord_stream_get_serialized_property(Pointer stream, String property_key,
                Structure.ByReference buffer, int buffer_size, IntByReference output_size);

        /**
         * Set the serialized property with key.
         *
         * @param stream Stream handle.
         * @param property_key Key of property to set.
         * @param buffer Buffer that contains input property values.
         * @param buffer_size Buffer size.
         * @return 0 is success or minus is failed (error code).
         * @see "Stream::SetProperty"
         */
        int senscord_stream_set_serialized_property(Pointer stream, String property_key,
                Structure.ByReference buffer, int buffer_size);

        /**
         * Get the count of supported property key on this stream.
         *
         * @param stream Stream handle.
         * @param count Count of supported property key.
         * @return 0 is success or minus is failed (error code).
         * @see "Stream::GetPropertyList"
         */
        int senscord_stream_get_property_count(Pointer stream, IntByReference count);

        /**
         * Get the supported property key on this stream.
         *
         * @param stream Stream handle.
         * @param index Index of supported property key list. (min=0, max=count-1)
         * @param property_key Location of property key.
         * @return 0 is success or minus is failed (error code).
         * @see "Stream::GetPropertyList"
         */
        int senscord_stream_get_property_key(Pointer stream, int index, String[] property_key);

        /**
         * Set the user data property.
         *
         * @param stream Stream handle.
         * @param buffer Buffer that contains input property values.
         * @param size Buffer size.
         * @return 0 is success or minus is failed (error code).
         * @see "Stream::SetProperty"
         */
        int senscord_stream_set_userdata_property(Pointer stream, Structure.ByReference buffer,
                int size);

        /**
         * Get the user data property.
         *
         * @param stream Stream handle.
         * @param property Buffer that stores output property values.
         * @param size Buffer size.
         * @return 0 is success or minus is failed (error code).
         * @see "Stream::GetProperty"
         */
        int senscord_stream_get_userdata_property(Pointer stream, Structure.ByReference property,
                int size);

        /**
         * Lock to access properties.
         *
         * @param stream Stream handle.
         * @param time_msec Time of wait msec if locked already. 0 is polling, minus is forever.
         * @return 0 is success or minus is failed (error code).
         * @see "Stream::LockProperty"
         * @see "SENSCORD_TIMEOUT_POLLING, SENSCORD_TIMEOUT_FOREVER"
         */
        int senscord_stream_lock_property(Pointer stream, int time_msec);

        /**
         * Lock to access properties (specified keys).
         *
         * @param stream Stream handle.
         * @param keys Key of property to lock.
         * @param count Count of property key.
         * @param time_msec Time of wait msec if locked already. 0 is polling, minus is forever.
         * @param resource Resource handle.
         * @return 0 is success or minus is failed (error code).
         * @see "Stream::LockProperty"
         * @see "SENSCORD_TIMEOUT_POLLING, SENSCORD_TIMEOUT_FOREVER"
         */
        int senscord_stream_lock_property_with_key(
                Pointer stream, String[] keys, int count, int time_msec, LongByReference resource);

        /**
         * Unlock to access properties.
         *
         * @param stream Stream handle.
         * @return 0 is success or minus is failed (error code).
         * @see "Stream::UnlockProperty"
         */
        int senscord_stream_unlock_property(Pointer stream);

        /**
         * Unlock to access properties (specified resource).
         *
         * @param stream Stream handle.
         * @param resource Resource handle.
         * @return 0 is success or minus is failed (error code).
         * @see "Stream::UnlockProperty"
         */
        int senscord_stream_unlock_property_by_resource(Pointer stream, long resource);

        /* Frame */

        /**
         * Get the sequential number of frame.
         *
         * @param frame Frame handle.
         * @param frame_number The number of this frame.
         * @return 0 is success or minus is failed (error code).
         * @see "Frame::GetSequenceNumber"
         */
        int senscord_frame_get_sequence_number(Pointer frame, LongByReference frame_number);

        /**
         * Get type of frame.
         *
         * @param frame Frame handle.
         * @param type Type of frame.
         * @return 0 is success or minus is failed (error code).
         * @see "Frame::GetType"
         */
        int senscord_frame_get_type(Pointer frame, String[] type);

        /**
         * Get channel count.
         *
         * @param frame Frame handle.
         * @param channel_count Location of channel count.
         * @return 0 is success or minus is failed (error code).
         * @see "Frame::GetChannelList"
         */
        int senscord_frame_get_channel_count(Pointer frame, IntByReference channel_count);

        /**
         * Get channel data.
         *
         * @param frame Frame handle.
         * @param index Index of channel list. (min=0, max=count-1)
         * @param channel Channel handle.
         * @return 0 is success or minus is failed (error code).
         * @see "Frame::GetChannelList"
         */
        int senscord_frame_get_channel(Pointer frame, int index, PointerByReference channel);

        /**
         * Get channel data.
         *
         * @param frame Frame handle.
         * @param channel_id Channel ID to get.
         * @param channel Channel handle.
         * @return 0 is success or minus is failed (error code).
         * @see "Frame::GetChannel"
         */
        int senscord_frame_get_channel_from_channel_id(Pointer frame, int channel_id,
                PointerByReference channel);

        /**
         * Get the user data.
         *
         * @param frame Frame handle.
         * @param user_data User data.
         * @return 0 is success or minus is failed (error code).
         * @see "Frame::GetUserData"
         */
        int senscord_frame_get_user_data(Pointer frame, UserData.ByReference user_data);

        /* Channel */

        /**
         * Get the channel ID.
         *
         * @param channel Channel handle.
         * @param id Channel ID.
         * @return 0 is success or minus is failed (error code).
         * @see "Channel::GetChannelId"
         */
        int senscord_channel_get_channel_id(Pointer channel, IntByReference id);

        /**
         * Get the raw data.
         *
         * @param channel Channel handle.
         * @param raw_data Raw data.
         * @return 0 is success or minus is failed (error code).
         * @see "Channel::GetRawData"
         */
        int senscord_channel_get_raw_data(Pointer channel, RawData.ByReference raw_data);

        /**
         * Get the property related to this channel.
         *
         * @param channel Channel handle.
         * @param property_key Key of property to get.
         * @param value Pointer to the structure of the property.
         * @param size Size of property structure.
         * @return 0 is success or minus is failed (error code).
         * @see "Channel::GetProperty"
         */
        int senscord_channel_get_property(Pointer channel, String property_key,
                Structure.ByReference value, int size);

        /**
         * Get the serialized property related to this raw data.
         *
         * @param channel Channel handle.
         * @param property_key Key of property to get.
         * @param value Pointer to the structure of the property.
         * @param size Buffer size.
         * @param out_size Size of output property. (optional)
         * @return 0 is success or minus is failed (error code).
         * @see "Channel::GetProperty"
         */
        int senscord_channel_get_serialized_property(Pointer channel, String property_key,
                Structure.ByReference value, int size, IntByReference out_size);

        /**
         * Get the count of stored property key on this channel.
         *
         * @param channel Channel handle.
         * @param count Count of stored property key.
         * @return 0 is success or minus is failed (error code).
         * @see "Channel::GetPropertyList"
         */
        int senscord_channel_get_property_count(Pointer channel, IntByReference count);

        /**
         * Get the stored property key on this channel.
         *
         * @param channel Channel handle.
         * @param index Index of stored property key list. (min=0, max=count-1)
         * @param property_key Location of property key.
         * @return 0 is success or minus is failed (error code).
         * @see "Channel::GetPropertyList"
         */
        int senscord_channel_get_property_key(Pointer channel, int index, String[] property_key);

        /**
         * Get the count of updated property key on this channel.
         *
         * @param channel Channel handle.
         * @param count Count of updated property key.
         * @return 0 is success or minus is failed (error code).
         * @see "Channel::GetUpdatedPropertyList"
         */
        int senscord_channel_get_updated_property_count(Pointer channel, IntByReference count);

        /**
         * Get the updated property key on this channel.
         *
         * @param channel Channel handle.
         * @param index Index of updated property key list. (min=0, max=count-1)
         * @param property_key Location of property key.
         * @return 0 is success or minus is failed (error code).
         * @see "Channel::GetUpdatedPropertyList"
         */
        int senscord_channel_get_updated_property_key(Pointer channel, int index,
                String[] property_key);

        /* Serialize / Deserialize */


        /**
         * Deserialize raw data. (AccelerationData, AngularVelocityData, MagneticFieldData)
         * To release, you need to call the senscord_release_vector3_data() function.
         *
         * @param raw_data Raw data.
         * @param raw_data_size Raw data size.
         * @param deserialize_data Deserialized vector3 data.
         * @return 0 is success or minus is failed (error code).
         * @see "senscord_release_vector3_data"
         */
        int senscord_deserialize_vector3_data(Pointer raw_data, int raw_data_size,
                Vector3Float.ByReference[] deserialize_data);

        /**
         * Release Vector3 data.
         *
         * @param data Vector3 data.
         * @return 0 is success or minus is failed (error code).
         */
        int senscord_release_vector3_data(Vector3Float.ByReference data);


        /**
         * Deserialize raw data. (RotationData)
         * To release, you need to call the senscord_release_rotation_data() function.
         *
         * @param raw_data Raw data.
         * @param raw_data_size Raw data size.
         * @param deserialize_data Deserialized rotation data.
         * @return 0 is success or minus is failed (error code).
         */
        int senscord_deserialize_rotation_data(Pointer raw_data, int raw_data_size,
                RotationData.ByReference[] deserialize_data);

        /**
         * Release rotation data.
         *
         * @param data Rotation data.
         * @return 0 is success or minus is failed (error code).
         */
        int senscord_release_rotation_data(RotationData.ByReference data);

        /**
         * Deserialize raw data. (Pose QuaternionData)
         * To release, you need to call the senscord_release_pose_quaternion_data() function.
         *
         * @param raw_data Raw data.
         * @param raw_data_size Raw data size.
         * @param deserialized_data Deserialized pose quaternion data.
         * @return 0 is success or minus is failed (error code).
         */
        int senscord_deserialize_pose_quaternion_data(Pointer raw_data, int raw_data_size,
                PoseQuaternionData.ByReference[] deserialized_data);

        /**
         * Release Pose quaternion data.
         * @param data pose quaternion data.
         * @return 0 is success or minus is failed (error code).
         */
        int senscord_release_pose_quaternion_data(PoseQuaternionData.ByReference data);

        /**
         * Deserialize raw data. (PoseMatrixData)
         * To release, you need to call the senscord_release_pose_matrix_data() function.
         *
         * @param raw_data Raw data.
         * @param raw_data_size Raw data size.
         * @param deserialized_data Deserialized pose matrix data.
         * @return 0 is success or minus is failed (error code).
         */
        int senscord_deserialize_pose_matrix_data(Pointer raw_data, int raw_data_size,
                PoseMatrixData.ByReference[] deserialized_data);

        /**
         * Release Pose matrix data.
         *
         * @param data Pose matrix data.
         * @return 0 is success or minus is failed (error code).
         */
        int senscord_release_pose_matrix_data(PoseMatrixData.ByReference data);

        /**
         * Deserialize raw data. (ObjectDetectionData)
         * To release, you need to call the senscord_release_object_detection_data() function.
         *
         * @param raw_data Raw data.
         * @param raw_data_size Raw data size.
         * @param deserialized_data Deserialized object detection data.
         * @return 0 is success or minus is failed (error code).
         * @see senscord_release_object_detection_data
         */
        int senscord_deserialize_object_detection_data(Pointer raw_data, int raw_data_size,
                ObjectDetectionData.ByReference[] deserialized_data);

        /**
         * Release Object detection data.
         * @param data Object detection data.
         */
        int senscord_release_object_detection_data(ObjectDetectionData.ByReference data);

        /**
         * Deserialize raw data. (KeyPointData)
         * To release, you need to call the senscord_release_key_point_data() function.
         *
         * @param raw_data Raw data.
         * @param raw_data_size Raw data size.
         * @param deserialized_data Deserialized key point data.
         * @return 0 is success or minus is failed (error code).
         * @see senscord_release_key_point_data
         */
        int senscord_deserialize_key_point_data(Pointer raw_data, int raw_data_size,
                KeyPointData.ByReference[] deserialized_data);

        /**
         * Release Object detection data.
         * @param data Object detection data.
         */
        int senscord_release_key_point_data(KeyPointData.ByReference data);

        /**
         * Deserialize raw data. (ObjectTrackingData)
         * To release, you need to call the senscord_release_object_tracking_data() function.
         *
         * @param raw_data Raw data.
         * @param raw_data_size Raw data size.
         * @param deserialized_data Deserialized object tracking data.
         * @return 0 is success or minus is failed (error code).
         * @see senscord_release_object_tracking_data
         */
        int senscord_deserialize_object_tracking_data(Pointer raw_data, int raw_data_size,
                ObjectTrackingData.ByReference[] deserialized_data);

        /**
         * Release Object tracking data.
         * @param data Object tracking data.
         */
        int senscord_release_object_tracking_data(ObjectTrackingData.ByReference data);

        /* Event argument */

        /**
         * Gets the int value of the specified key.
         * @param args Event argument handle.
         * @param key Argument key.
         * @param value Location to store the value.
         * @return 0 is success or minus is failed.
         */
        int senscord_event_argument_getvalue_int8(
                Pointer args, String key, ByteByReference value);

        /**
         * Gets the int value of the specified key.
         * @param args Event argument handle.
         * @param key Argument key.
         * @param value Location to store the value.
         * @return 0 is success or minus is failed.
         */
        int senscord_event_argument_getvalue_int16(
                Pointer args, String key, ShortByReference value);

        /**
         * Gets the int value of the specified key.
         * @param args Event argument handle.
         * @param key Argument key.
         * @param value Location to store the value.
         * @return 0 is success or minus is failed.
         */
        int senscord_event_argument_getvalue_int32(
                Pointer args, String key, IntByReference value);

        /**
         * Gets the int value of the specified key.
         * @param args Event argument handle.
         * @param key Argument key.
         * @param value Location to store the value.
         * @return 0 is success or minus is failed.
         */
        int senscord_event_argument_getvalue_int64(
                Pointer args, String key, LongByReference value);

        /**
         * Gets the float value of the specified key.
         * @param args Event argument handle.
         * @param key Argument key.
         * @param value Location to store the value.
         * @return 0 is success or minus is failed.
         */
        int senscord_event_argument_getvalue_float(
                Pointer args, String key, FloatByReference value);

        /**
         * Gets the double value of the specified key.
         * @param args Event argument handle.
         * @param key Argument key.
         * @param value Location to store the value.
         * @return 0 is success or minus is failed.
         */
        int senscord_event_argument_getvalue_double(
                Pointer args, String key, DoubleByReference value);

        /**
         * Gets the string of the specified key.
         * @param args Event argument handle.
         * @param key Argument key.
         * @param buffer Location to store the string.
         * @param length [in] Buffer size.
         *               [out] String length. (not including '\0')
         * @return 0 is success or minus is failed.
         */
        int senscord_event_argument_getvalue_string(
                Pointer args, String key,
                byte[] buffer, IntByReference length);

        /**
         * Gets the binary array of the specified key.
         * @param args Event argument handle.
         * @param key Argument key.
         * @param buffer Location to store the binary array.
         * @param length [in] Buffer size.
         *               [out] Length of binary array.
         * @return 0 is success or minus is failed.
         */
        int senscord_event_argument_getvalue_binary(
                Pointer args, String key,
                byte[] buffer, IntByReference length);

        /**
         * Get the number of elements.
         * @param args Event argument handle.
         * @param count Location to store the number of elements.
         * @return 0 is success or minus is failed.
         */
        int senscord_event_argument_get_element_count(
                Pointer args, IntByReference count);

        /**
         * Gets the key at the specified index.
         * @param args Event argument handle.
         * @param index Index (0 to elements-1)
         * @return String pointer. Returns NULL if invalid.
         */
        String senscord_event_argument_get_key(
                Pointer args, int index);

        /**
         * Set the channel id to property key.
         * @param key Property key.
         * @param channel_id Channel ID.
         * @param made_key Property key + Channel ID.
         * @param length [in] made_key buffer size.
         *               [out] made_key length.
         * @return 0 is success or minus is failed.
         */
        int senscord_property_key_set_channel_id(
                String key, int channel_id, byte[] made_key,
                IntByReference length);

        /**
         * Returns the byte width in PCM format.
         * @param format PCM format. {@link AudioPcmProperty.Format}
         * @return Byte width.
         */
        int senscord_audio_pcm_get_byte_width(int format);

        /**
         * Returns the number of bits per sample in PCM format.
         * @param format PCM format. {@link AudioPcmProperty.Format}
         * @return Number of bits per sample.
         */
        int senscord_audio_pcm_get_bits_per_sample(int format);

        /**
         * Returns non-zero if PCM format is signed type.
         * @param format PCM format. {@link AudioPcmProperty.Format}
         * @return Non-zero if signed type.
         */
        int senscord_audio_pcm_is_signed(int format);

        /**
         * Returns non-zero if PCM format is unsigned type.
         * @param format PCM format. {@link AudioPcmProperty.Format}
         * @return Non-zero if unsigned type.
         */
        int senscord_audio_pcm_is_unsigned(int format);

        /**
         * Returns non-zero if PCM format is float type.
         * @param format PCM format. {@link AudioPcmProperty.Format}
         * @return Non-zero if float type.
         */
        int senscord_audio_pcm_is_float(int format);

        /**
         * Returns non-zero if PCM format is little endian.
         * @param format PCM format. {@link AudioPcmProperty.Format}
         * @return Non-zero if little endian.
         */
        int senscord_audio_pcm_is_little_endian(int format);

        /**
         * Returns non-zero if PCM format is big endian.
         * @param format PCM format. {@link AudioPcmProperty.Format}
         * @return Non-zero if big endian.
         */
        int senscord_audio_pcm_is_big_endian(int format);
    }
}
