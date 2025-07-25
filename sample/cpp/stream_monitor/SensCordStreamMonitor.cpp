/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

#include "senscord/senscord.h"
#include "senscord/osal.h"

#define TEST_PRINT(...) \
  do { senscord::osal::OSPrintf("[L%d] ", __LINE__); \
       senscord::osal::OSPrintf(__VA_ARGS__); } while (0)

namespace {

// ===============================================================
// Default values.
// ===============================================================
const char* kDefaultStreamKey = "pseudo_image_stream.0";
const double kDefaultConfidenceMinValue = 0.0;
const double kDefaultConfidenceMaxValue = 4095.0;
// ===============================================================

/**
 * @brief OpenStream buffering settings.
 */
const senscord::Buffering kFrameBuffering = senscord::kBufferingOff;
const int32_t kFrameBufferNum = 2;
const senscord::BufferingFormat kFrameBufferingFormat =
    senscord::kBufferingFormatOverwrite;

/**
 * @brief Display scaling factor.
 */
const int16_t kMinDisplayScaleFactor = 25;
const int16_t kMaxDisplayScaleFactor = 400;
const int16_t kDefaultDisplayScaleFactor = 100;

/**
 * @brief The angle of view when no drawing is performed.
 */
const int16_t kNotifyWindowWidth = 640;
const int16_t kNotifyWindowHeight = 480;

/**
 * @brief Timeout (msec) of GetFrame.
 */
const int32_t kGetFrameTimeoutMsec = 5000;

/**
 * @brief Fps update interval (seconds).
 */
const double kFpsUpdateInterval = 0.5;

// Global variables
const char* stream_key_ = kDefaultStreamKey;
double confidence_min_value_ = kDefaultConfidenceMinValue;
double confidence_max_value_ = kDefaultConfidenceMaxValue;
uint32_t display_channel_id_ = 0;
uint32_t display_scale_factor_ = kDefaultDisplayScaleFactor;
double display_fps_ = 0.0;
std::set<uint32_t> channel_ids_;

/**
 * @brief Viewing mode.
 *        true:  Image drawing mode.
 *        false: Frame receiving rate check mode.
 */
bool is_viewing_ = true;

/**
 * @brief Shift direction.
 */
enum ShiftDirection {
  kForwardShift = 0,
  kBackwardShift,
};

/**
 * @brief Elapsed time.
 */
class ElapsedTime {
 public:
  ElapsedTime() : start_tick_(cv::getTickCount()) {}
  void reset_time() {
    start_tick_ = cv::getTickCount();
  }
  int64_t elapsed_ticks() {
    return cv::getTickCount() - start_tick_;
  }
  double elapsed_ms() {
    return static_cast<double>(
        elapsed_ticks() * 1000) / cv::getTickFrequency();
  }
  double elapsed_sec() {
    return static_cast<double>(
        elapsed_ticks()) / cv::getTickFrequency();
  }
 private:
  int64_t start_tick_;
};

/**
 * @brief Calculation data rate.
 */
class DataRate : public ElapsedTime {
 public :
  DataRate() : count_(0) {}
  void reset() {
    reset_time();
    count_ = 0;
  }
  void count() { ++count_; }
  double rate_per_sec() {
    return count_ / elapsed_sec();
  }
 private:
  uint count_;
};

/**
 * @brief Draw text.
 * @param[in] (image) Draw image data.
 * @param[in] (line_num) Draw text line number.
 * @param[in] (format) Draw text format.
 */
void PutText(
    const cv::Mat& image, int line_num, const char* format, ...) {
  const int kPutTextPosX = 10;
  const int kPutTextPosY = 35;
  const int kFontThickness = 1;
  const int kLineSpacing = 35;
  const double kFontScale = 1.0;
  const cv::Scalar kFontColor = CV_RGB(255, 255, 255);
  const cv::Scalar kFontBorderColor = CV_RGB(0, 0, 0);

  char text[64];
  va_list args;
  va_start(args, format);
  std::vsnprintf(text, sizeof(text), format, args);
  va_end(args);

  cv::putText(
      image, text,
      cv::Point(kPutTextPosX, kPutTextPosY + kLineSpacing * line_num),
      cv::FONT_HERSHEY_DUPLEX, kFontScale, kFontBorderColor,
      kFontThickness + 2);
  cv::putText(
      image, text,
      cv::Point(kPutTextPosX, kPutTextPosY + kLineSpacing * line_num),
      cv::FONT_HERSHEY_DUPLEX, kFontScale, kFontColor, kFontThickness);
}

/**
 * @brief Draw stream info.
 * @param[in] (image) Draw image data
 * @param[in] (width) Draw width text.
 * @param[in] (height) Draw height text.
 * @param[in] (rawdata_type) Draw rawdata type.
 */
void PutStreamInfo(
    const cv::Mat& image, uint32_t width, uint32_t height,
    const std::string& rawdata_type) {
  // Put Channel ID info
  PutText(image, 0, "Ch:%x", display_channel_id_);

  // Put RawDataType info
  PutText(image, 1, "%s", rawdata_type.c_str());

  // Put resolution & scale factor info
  PutText(image, 2, "%dx%d(%d%%)", width, height, display_scale_factor_);

  // Put FPS info
  PutText(image, 3, "%.2lfFPS", display_fps_);

  // Put notice info if the format is unsupported
  if ((is_viewing_) && (height == 0 || width == 0)) {
    PutText(image, 4, "Unsupported format");
  }

  // Put info for Frame receiving rate check mode
  if (!is_viewing_) {
    PutText(image, 5, "Frame receiving rate check mode.");
    PutText(image, 6, "Press 'v' key to image drawing mode.");
  }
}

/**
 * @brief Get normalize factors.
 * @param[in] (min) Min range values.
 * @param[in] (max) Max range values.
 * @return Normalize factors.
 */
std::pair<double, double> GetNormalizeFactors(
    const double min, const double max) {
  double scale = UINT8_MAX / (max - min);
  double delta = min * scale * (-1);
  return std::make_pair(scale, delta);
}

/**
 * @brief Normalize from src image to dst image (8bit: 0-255).
 * @param[in] (image) Raw data of Image targeted for Normalize.
 * @param[in] (scale) Scale value to make 8bit.
 * @param[in] (delta) Add to scaled value.
 * @return Image normalized to 8bit is returned.
 */
cv::Mat NormalizeImageTo8bitImage(
    const cv::Mat& image, const double scale, const double delta) {
  cv::Mat normalized_image(image.rows, image.cols, CV_8UC1);
  image.convertTo(normalized_image, CV_8UC1, scale, delta);
  return normalized_image;
}

/**
 * @brief Converts yuv pixel to bgr pixel data.
 * @param[in] (y) Luminance y.
 * @param[in] (u) Chroma u.
 * @param[in] (v) Chroma v.
 * @return Converted bgr pixel data.
 */
cv::Vec3b YuvToBgr(const int16_t y, const int16_t u, const int16_t v) {
  // The conversion constants are based on OpenCV's source code.
  uint8_t b = cv::saturate_cast<uint8_t>(y + 1.773 * u);
  uint8_t g = cv::saturate_cast<uint8_t>(y - 0.344 * u - 0.714 * v);
  uint8_t r = cv::saturate_cast<uint8_t>(y + 1.403 * v);

  return cv::Vec3b(b, g, r);
}

/**
 * @brief Converts nv16 data to bgr image data.
 * @param[in] (width) Image width.
 * @param[in] (height) Image height.
 * @param[in] (stride_bytes) Image stride bytes.
 * @param[in] (rawdata) The RawData in nv16 data format.
 * @return Converted bgr image data.
 */
cv::Mat ConvertNv16ToBgrImage(
    const uint32_t width, const uint32_t height,
    const uint32_t stride_bytes, uint8_t* rawdata) {
  cv::Mat y_image(height, stride_bytes, CV_8UC1, rawdata);
  cv::Mat uv_image(
    height, stride_bytes, CV_8UC1, rawdata + (height * stride_bytes));
  cv::Mat bgr_image(height, width, CV_8UC3);

  for (uint32_t h = 0; h < height; ++h) {
    for (uint32_t w = 0; w < width; ++w) {
      int16_t y = y_image.at<uint8_t>(h, w);
      int16_t u = uv_image.at<uint8_t>(h, w - w % 2);
      int16_t v = uv_image.at<uint8_t>(h, w - w % 2 + 1);
      bgr_image.at<cv::Vec3b>(h, w) =
          YuvToBgr(
              y,
              static_cast<int16_t>(u - 128),
              static_cast<int16_t>(v - 128));
    }
  }
  return bgr_image;
}

/**
 * @brief Remove padding data.
 * @param[in] (image) Target image data.
 * @param[in] (width) Image width.
 * @param[in] (height) Image height.
 * @return Image data with padding removed.
 */
cv::Mat RemovePadding(
    const cv::Mat& image, const uint32_t width, const uint32_t height) {
  return cv::Mat(image, cv::Rect(0, 0, width, height));
}

/**
 * @brief Create unsupported 8bit image data.
 * @return Unsupported 8bit image data.
 */
cv::Mat MakeUnsupportedImage() {
  return cv::Mat(
      kNotifyWindowHeight, kNotifyWindowWidth, CV_8UC1, cv::Scalar(255));
}

/**
 * @brief Create blank 8bit image data.
 * @return Blank 8bit image data.
 */
cv::Mat MakeBlackBackImage() {
  return cv::Mat(
      kNotifyWindowHeight, kNotifyWindowWidth, CV_8UC1, cv::Scalar(0));
}

/**
 * @brief Converts 1bit data to 8bit image data.
 * @param[in] (width) Image width.
 * @param[in] (height) Image height.
 * @param[in] (stride_bytes) Image stride bytes. 
 * @param[in] (rawdata) The RawData in 1bit data format.
 * @return Converted 8bit image data.
 */
cv::Mat Convert1bitToImage(
    const uint32_t width, const uint32_t height, const uint32_t stride_bytes,
    uint8_t* rawdata) {
  const uint8_t kBitNum = 8;
  cv::Mat image(height, width, CV_8UC1);

  for (uint32_t y = 0; y < height; ++y) {
    for (uint32_t x = 0; x < width; ++x) {
      // Divide pixel(8bit) into 1bit, and store 1bit converted widening
      // to 8bit in stretched_image.
      uint64_t index = x / kBitNum + y * stride_bytes;
      image.at<uint8_t>(y, x) =
          ((rawdata[index] & (1 << x % kBitNum)) > 0) ? UINT8_MAX : 0;
    }
  }
  return image;
}

/**
 * @brief Convert unsigned 16bit to 8bit image data.
 * @param[in] (width) Image width.
 * @param[in] (height) Image height
 * @param[in] (stride_bytes) Image stride bytes. 
 * @param[in] (min_range) Maximum value of RawData.
 * @param[in] (max_range) Minimum value of RawData.
 * @param[in] (is_colored) If True, color it.
 * @param[in] (rawdata) The RawData in unsigned 16bit data format.
 * @return Converted 8bit image data.
 */
cv::Mat Convert16bitToImage(
    const uint32_t width, const uint32_t height, const uint32_t stride_bytes,
    const double min_range, const double max_range, const bool is_colored,
    uint16_t* rawdata) {
  uint32_t padding =
      (stride_bytes / static_cast<uint32_t>(sizeof(uint16_t))) - width;
  cv::Mat image_with_padding(height, width + padding, CV_16UC1, rawdata);
  cv::Mat image = RemovePadding(image_with_padding, width, height);

  std::pair<double, double> normalize_factors =
      GetNormalizeFactors(min_range, max_range);

  cv::Mat normalized_image = NormalizeImageTo8bitImage(
      image, normalize_factors.first, normalize_factors.second);
  if (is_colored) {
    cv::Mat colored_image(height, width, CV_8UC3);
    cv::applyColorMap(normalized_image, colored_image, cv::COLORMAP_JET);
    return colored_image;
  }
  return normalized_image;
}

/**
 * @brief Convert float 32bit to 8bit image data.
 * @param[in] (width) Image width.
 * @param[in] (height) Image height.
 * @param[in] (stride_bytes) Image stride bytes.
 * @param[in] (min_range) Maximum value of RawData.
 * @param[in] (max_range) Minimum value of RawData.
 * @param[in] (is_colored) If True, color it.
 * @param[in] (rawdata) The RawData in float 32bit data format.
 * @return Converted 8bit image data.
 */
cv::Mat ConvertFloatToImage(
    const uint32_t width, const uint32_t height, const uint32_t stride_bytes,
    const double min_range, const double max_range, const bool is_colored,
    float* rawdata) {
  uint32_t padding =
      (stride_bytes / static_cast<uint32_t>(sizeof(float))) - width;
  cv::Mat image_with_padding(height, width + padding, CV_32FC1, rawdata);
  cv::Mat image = RemovePadding(image_with_padding, width, height);

  std::pair<double, double> normalize_factors =
      GetNormalizeFactors(min_range, max_range);

  cv::Mat normalized_image = NormalizeImageTo8bitImage(
      image, normalize_factors.first, normalize_factors.second);
  if (is_colored) {
    cv::Mat colored_image(height, width, CV_8UC3);
    cv::applyColorMap(normalized_image, colored_image, cv::COLORMAP_JET);
    return colored_image;
  }
  return normalized_image;
}

/**
 * @brief Convert image data.
 * @param[in] (property) Image property.
 * @param[in] (rawdata_address) Image rawdata address.
 * @return Image data converted from image data.
 */
cv::Mat ConvertRawImageDataToImage(
    const senscord::ImageProperty& property, void* rawdata_address) {
  cv::Mat image;
  if (property.pixel_format == senscord::kPixelFormatGREY) {
    cv::Mat image_with_padding(
        property.height, property.stride_bytes, CV_8UC1,
        reinterpret_cast<uint8_t*>(rawdata_address));
    image = RemovePadding(image_with_padding, property.width, property.height);
  } else if (property.pixel_format == senscord::kPixelFormatNV16) {
    image = ConvertNv16ToBgrImage(
        property.width, property.height, property.stride_bytes,
        reinterpret_cast<uint8_t*>(rawdata_address));
  } else {
    // Format is unsupported.
    image = MakeUnsupportedImage();
  }
  return image;
}

/**
 * @brief Convert depth data.
 * @param[in] (depth_property) DepthProperty.
 * @param[in] (image_property) ImageProperty.
 * @param[in] (rawdata_address) Depth rawdata address.
 * @return Image data converted from depth data.
 */
cv::Mat ConvertRawDepthDataToImage(
    const senscord::DepthProperty& depth_property,
    const senscord::ImageProperty& image_property, void* rawdata_address) {
  cv::Mat image;
  if (image_property.pixel_format == senscord::kPixelFormatZ16 ||
      image_property.pixel_format == senscord::kPixelFormatD16) {
    image = Convert16bitToImage(
        image_property.width, image_property.height,
        image_property.stride_bytes,
        depth_property.depth_min_range, depth_property.depth_max_range, true,
        reinterpret_cast<uint16_t*>(rawdata_address));
  } else if (image_property.pixel_format == senscord::kPixelFormatZ32F) {
    image = ConvertFloatToImage(
        image_property.width, image_property.height,
        image_property.stride_bytes,
        depth_property.depth_min_range, depth_property.depth_max_range, true,
        reinterpret_cast<float*>(rawdata_address));
  } else {
    // Format is unsupported.
    image = MakeUnsupportedImage();
  }
  return image;
}

/**
 * @brief Convert confidence data.
 * @param[in] (property) Confidence property.
 * @param[in] (rawdata_address) rawdata address.
 * @return Image data converted from confidence data.
 */
cv::Mat ConvertRawConfidenceDataToImage(
    const senscord::ConfidenceProperty& property, void* rawdata_address) {
  cv::Mat image;
  if (property.pixel_format == senscord::kPixelFormatC1P ||
      property.pixel_format == senscord::kPixelFormatC1N) {
    image = Convert1bitToImage(
        property.width, property.height, property.stride_bytes,
        reinterpret_cast<uint8_t*>(rawdata_address));
  } else if (property.pixel_format == senscord::kPixelFormatC16) {
    image = Convert16bitToImage(
        property.width, property.height, property.stride_bytes,
        confidence_min_value_, confidence_max_value_, false,
        reinterpret_cast<uint16_t*>(rawdata_address));
  } else if (property.pixel_format == senscord::kPixelFormatC32F) {
    image = ConvertFloatToImage(
        property.width, property.height, property.stride_bytes,
        confidence_min_value_, confidence_max_value_, false,
        reinterpret_cast<float*>(rawdata_address));
  } else {
    // Format is unsupported.
    image = MakeUnsupportedImage();
  }
  return image;
}

/**
 * @brief Display opencv image data.
 * @param[in] (image) Converted image matrix.
 * @param[in] (width) Image width.
 * @param[in] (height) Image height.
 * @param[in] (rawdata_type) Rawdata type.
 */
void DisplayData(
    const cv::Mat& image, const uint32_t width, const uint32_t height,
    const std::string& rawdata_type) {
  cv::Mat resized_image;

  if (display_scale_factor_ == 100 || !is_viewing_ || width == 0 ||
    height == 0) {
    resized_image = image;
  } else {
    cv::resize(
        image, resized_image, cv::Size(),
        display_scale_factor_ / 100.0,
        display_scale_factor_ / 100.0);
  }

  PutStreamInfo(resized_image, width, height, rawdata_type);
  cv::imshow(stream_key_, resized_image);
}

/**
 * @brief Display image channel data.
 * @param[in] (channel) Channel instance.
 * @param[in] (rawdata) Channel rawdata.
 */
void DisplayImageData(
    senscord::Channel* channel, const senscord::Channel::RawData& rawdata) {
  senscord::ImageProperty property = {};
  senscord::Status status = channel->GetProperty(
      senscord::kImagePropertyKey, &property);
  if (!status.ok()) {
    TEST_PRINT("Channel::GetProperty(): status=%s\n",
      status.ToString().c_str());
  }

  cv::Mat image;
  if (status.ok()) {
    image = ConvertRawImageDataToImage(property, rawdata.address);
  } else {
    image = MakeUnsupportedImage();
  }
  DisplayData(image, image.cols, image.rows, rawdata.type);
}

/**
 * @brief Display depth channel data.
 * @param[in] (channel) Channel instance.
 * @param[in] (rawdata) Channel rawdata.
 */
void DisplayDepthData(
    senscord::Channel* channel, const senscord::Channel::RawData& rawdata) {
  senscord::DepthProperty depth_property = {};
  senscord::Status status = channel->GetProperty(
      senscord::kDepthPropertyKey, &depth_property);
  if (!status.ok()) {
    TEST_PRINT("Channel::GetProperty(): status=%s\n",
      status.ToString().c_str());
  }

  senscord::ImageProperty image_property = {};
  if (status.ok()) {
    status = channel->GetProperty(senscord::kImagePropertyKey, &image_property);
    if (!status.ok()) {
      TEST_PRINT("Channel::GetProperty(): status=%s\n",
        status.ToString().c_str());
    }
  }

  cv::Mat image;
  if (status.ok()) {
    image = ConvertRawDepthDataToImage(
      depth_property, image_property, rawdata.address);
  } else {
    image = MakeUnsupportedImage();
  }

  DisplayData(image, image.cols, image.rows, rawdata.type);
}

/**
 * @brief Display confidence channel data.
 * @param[in] (channel) Channel instance.
 * @param[in] (rawdata) Channel rawdata.
 */
void DisplayConfidenceData(
    senscord::Channel* channel, const senscord::Channel::RawData& rawdata) {
  senscord::ConfidenceProperty property = {};
  senscord::Status status = channel->GetProperty(
      senscord::kConfidencePropertyKey, &property);
  if (!status.ok()) {
    TEST_PRINT("Channel::GetProperty(): status=%s\n",
      status.ToString().c_str());
  }

  cv::Mat image;
  if (status.ok()) {
    image = ConvertRawConfidenceDataToImage(property, rawdata.address);
  } else {
    image = MakeUnsupportedImage();
  }
  DisplayData(image, image.cols, image.rows, rawdata.type);
}

/**
 * @brief Display channel data.
 * @param[in] (channel) Channel instance.
 */
void DisplayData(senscord::Channel* channel) {
  // Get raw data
  senscord::Channel::RawData rawdata = {};
  senscord::Status status = channel->GetRawData(&rawdata);
  if (status.ok()) {
    if (rawdata.type == senscord::kRawDataTypeImage) {
      DisplayImageData(channel, rawdata);
      return;
    } else if (rawdata.type == senscord::kRawDataTypeDepth) {
      DisplayDepthData(channel, rawdata);
      return;
    } else if (rawdata.type == senscord::kRawDataTypeConfidence) {
      DisplayConfidenceData(channel, rawdata);
      return;
    }
  } else {
    TEST_PRINT("Channel::GetRawData(): status=%s\n", status.ToString().c_str());
  }
  // Get raw data fail or Channel has the others.
  cv::Mat image = MakeUnsupportedImage();
  DisplayData(image, image.cols, image.rows, "unknown");
}

/**
 * @brief Display frame receiving rate.
 */
void DisplayFrameReceivingRate() {
  cv::Mat image = MakeBlackBackImage();
  DisplayData(image, 0, 0, "");
}

/**
 * @brief Change the channel id of the drawing target.
 * @param[in] (frame) Target frame.
 * @param[in] (shift_direction) Target Channel id shift direction.
 * @return 0 is success, other failed.
 */
int8_t ChangeChannelId(
    senscord::Frame* frame, const ShiftDirection shift_direction) {
  if (channel_ids_.size() > 1) {
    std::set<uint32_t>::iterator itr = channel_ids_.find(display_channel_id_);
    if (itr != channel_ids_.end()) {
      if (shift_direction == kForwardShift) {
        ++itr;
        if (itr == channel_ids_.end()) {
          // to first
          display_channel_id_ = *(channel_ids_.begin());
        } else {
          // forward
          display_channel_id_ = *itr;
        }
      } else if (shift_direction == kBackwardShift) {
        if (itr == channel_ids_.begin()) {
          // to last
          display_channel_id_ = *(channel_ids_.rbegin());
        } else {
          // backward
          display_channel_id_ = *(--itr);
        }
      }
    } else {
      // illegal case
      TEST_PRINT("not found channel id: " PRIu32 "\n", display_channel_id_);
    }
  }
  return 0;
}

/**
 * @brief Increase the display magnification..
 */
void UpDisplayScaleFactor() {
  if (display_scale_factor_ < kMaxDisplayScaleFactor) {
    display_scale_factor_ *= 2;
  }
}

/**
 * @brief Decrease the display magnification.
 */
void DownDisplayScaleFactor() {
  if (display_scale_factor_ > kMinDisplayScaleFactor) {
    display_scale_factor_ /= 2;
  }
}

/**
 * @brief Record the stream.
 * @param[in] (stream) Stream instance.
 */
void RecordStream(senscord::Stream* stream) {
  senscord::RecordProperty rec = {};
  senscord::Status status = stream->GetProperty(
      senscord::kRecordPropertyKey, &rec);

  if (status.ok()) {
    if (!rec.enabled) {
      senscord::ChannelInfoProperty channel_info = {};
      status = stream->GetProperty(
          senscord::kChannelInfoPropertyKey, &channel_info);
      if (status.ok()) {
        rec.enabled = true;
        for (std::map<uint32_t, senscord::ChannelInfo>::iterator itr =
            channel_info.channels.begin(); itr != channel_info.channels.end();
            ++itr) {
          rec.formats.insert(
            std::make_pair(itr->first, senscord::kRecordingFormatRaw));
        }
      }
    } else {
      rec.enabled = false;
    }
  }

  if (status.ok()) {
    status = stream->SetProperty(senscord::kRecordPropertyKey, &rec);
  }

  if (status.ok() && rec.enabled) {
    status = stream->GetProperty(senscord::kRecordPropertyKey, &rec);
  }

  if (status.ok()) {
    TEST_PRINT("Recording %s: path=%s\n",
        rec.enabled ? "start" : "stop", rec.path.c_str());
  } else {
    TEST_PRINT("Recording %s: status=%s\n",
        rec.enabled ? "start" : "stop", status.ToString().c_str());
  }
}

/**
 * @brief Handle input keys from the user.
 * @param[in] (stream) Stream instance.
 * @param[in] (frame) Frame instance.
 * @return True is application termination
 */
bool HandleInputKey(senscord::Stream* stream, senscord::Frame* frame) {
  int32_t input_key = cv::waitKey(1);
  // Key codes
  const int32_t kAKey = 0x0061;
  const int32_t kDKey = 0x0064;
  const int32_t kWKey = 0x0077;
  const int32_t kSKey = 0x0073;
  const int32_t kRKey = 0x0072;
  const int32_t kVKey = 0x0076;
  const int32_t kQKey = 0x0071;

  switch (input_key) {
  case kAKey:  // Change Channel ID
    ChangeChannelId(frame, kBackwardShift);
    break;
  case kDKey:  // Change Channel ID
    ChangeChannelId(frame, kForwardShift);
    break;

  case kWKey:  // Up window scale
    UpDisplayScaleFactor();
    break;
  case kSKey:  // Down window scale
    DownDisplayScaleFactor();
    break;

  case kRKey:  // Recod start/stop
    RecordStream(stream);
    break;

  case kVKey:  // Switch view mode
    is_viewing_ = !is_viewing_;
    break;

  case kQKey:  // quit
    return true;

  default:
    break;
  }

  return false;
}

/**
 * @brief Process frame data.
 * @param[in] (core) Core instance.
 * @return 0 is success, other failed.
 */
int8_t ProcessFrame(senscord::Frame* frame) {
  // Get channel
  senscord::Channel* channel = NULL;
  senscord::Status status;

  status = frame->GetChannel(
      static_cast<uint32_t>(display_channel_id_), &channel);
  if (!status.ok()) {
    // TEST_PRINT("Frame::GetChannel(): status=%s\n",
    // status.ToString().c_str());
  }
  if (channel != NULL) {
    if (is_viewing_) {
      // Display converted raw data
      DisplayData(channel);
    } else {
      DisplayFrameReceivingRate();
    }
  } else {
    cv::Mat image = MakeUnsupportedImage();
    DisplayData(image, image.cols, image.rows, "not found");
  }

  return 0;
}

/**
 * @brief Display stream.
 * @param[in] (core) Core instance.
 * @return 0 is success, other failed.
 */
int8_t DisplayStream(senscord::Stream* stream) {
  bool quit_flag = false;
  DataRate frame_rate;

  TEST_PRINT("Display stream - start\n");

  while (!quit_flag) {
    // Get frame
    senscord::Frame* frame = NULL;
    senscord::Status status = stream->GetFrame(&frame, kGetFrameTimeoutMsec);
    if (!status.ok()) {
      TEST_PRINT("Stream::GetFrame(): status=%s\n", status.ToString().c_str());
      return -1;
    }

    // Mesure frame rate
    frame_rate.count();
    if (frame_rate.elapsed_sec() >= kFpsUpdateInterval) {
      display_fps_ = frame_rate.rate_per_sec();
      frame_rate.reset();
    }

    // Draw frame
    int8_t result = ProcessFrame(frame);

    // Receive input key
    if ((result < 0) || HandleInputKey(stream, frame)) {
      quit_flag = true;
    }

    // Release frame
    status = stream->ReleaseFrame(frame);
    if (!status.ok()) {
      TEST_PRINT("Stream::ReleaseFrame(): status=%s\n",
        status.ToString().c_str());
      return -1;
    }
  }

  TEST_PRINT("Display stream - stop\n");
  return 0;
}

/**
 * @brief Process stream.
 * @param[in] (core) Core instance.
 * @return 0 is success, other failed.
 */
int8_t ProcessStream(senscord::Core* core) {
  // Open stream
  senscord::Stream* stream = NULL;
  senscord::OpenStreamSetting settings = {};
  settings.frame_buffering.buffering = kFrameBuffering;
  settings.frame_buffering.num = kFrameBufferNum;
  settings.frame_buffering.format = kFrameBufferingFormat;
  senscord::Status status = core->OpenStream(
      stream_key_, settings, &stream);
  if (!status.ok()) {
    TEST_PRINT("Core::OpenStream(): status=%s\n", status.ToString().c_str());
    return -1;
  }

  // Get ChannelInfo
  {
    senscord::ChannelInfoProperty property = {};
    status = stream->GetProperty(senscord::kChannelInfoPropertyKey, &property);
    if (!status.ok()) {
      TEST_PRINT("Stream::GetProperty(): status=%s\n",
          status.ToString().c_str());
      return -1;
    }
    std::map<uint32_t, senscord::ChannelInfo>::iterator itr =
        property.channels.begin();
    std::map<uint32_t, senscord::ChannelInfo>::iterator end =
        property.channels.end();
    for (; itr != end; ++itr) {
      channel_ids_.insert(itr->first);
    }
    display_channel_id_ = *(channel_ids_.begin());
  }

  // Start stream
  status = stream->Start();
  if (!status.ok()) {
    TEST_PRINT("Stream::Start(): status=%s\n", status.ToString().c_str());
    return -1;
  }

  int8_t result = DisplayStream(stream);

  // Stop stream
  status = stream->Stop();
  if (!status.ok()) {
    TEST_PRINT("Stream::Stop(): status=%s\n", status.ToString().c_str());
    return -1;
  }

  // Close stream
  status = core->CloseStream(stream);
  if (!status.ok()) {
    TEST_PRINT("Core::CloseStream(): status=%s\n", status.ToString().c_str());
    return -1;
  }

  return result;
}

/**
 * @brief Showing control message.
 */
void ShowStartUpMessage() {
  TEST_PRINT("==================================================\n");
  TEST_PRINT("SensCordStreamMonitor\n");
  TEST_PRINT("==================================================\n");
  TEST_PRINT(" How to operate:\n");
  TEST_PRINT("  a / d : Change Channel ID(*)\n");
  TEST_PRINT("  w / s : Resize scale factor\n");
  TEST_PRINT("  r: Record start/stop\n");
  TEST_PRINT("  v: Switch display mode\n");
  TEST_PRINT("  q: Quit application\n");
  TEST_PRINT("\n");
  TEST_PRINT("  * It works if a Frame has multipul Channel IDs.\n");
  TEST_PRINT("==================================================\n");
}

}  // namespace

/**
 * @brief Parse the application's arguments.
 * @param[in] (argc) The number of arguments.
 * @param[in] (argv) The argumens.
 * @return 0 is success, other failed.
 */
static int8_t ParseArguments(uint32_t argc, const char* argv[]) {
  for (uint32_t i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    const char* next = NULL;

    if ((i + 1) < argc) {
      next = argv[i + 1];
    }

    if (arg == "-k") {
      if (next != NULL) {
        stream_key_ = next;
        ++i;
      }
    } else if (arg == "-cmax") {
      if (next != NULL) {
        confidence_max_value_ = strtod(next, NULL);
        ++i;
      }
    } else if (arg == "-cmin") {
      if (next != NULL) {
        confidence_min_value_ = strtod(next, NULL);
        ++i;
      }
    } else {
      return -1;
    }
  }
  return 0;
}

int main(int argc, const char* argv[]) {
  int8_t ret = ParseArguments(argc, argv);
  if (ret < 0) {
    TEST_PRINT(
        "Usage: %s [-k stream_key][-cmax confidence_max]"
        "[-cmin confidence_min]\n",
        argv[0]);
    return -1;
  }

  ShowStartUpMessage();

  // Init Core
  senscord::Core core;
  senscord::Status status = core.Init();
  if (!status.ok()) {
    TEST_PRINT("Core::Init(): status=%s\n", status.ToString().c_str());
    return -1;
  }

  int8_t result = ProcessStream(&core);

  // Exit Core
  status = core.Exit();
  if (!status.ok()) {
    TEST_PRINT("Core::Exit(): status=%s\n", status.ToString().c_str());
    return -1;
  }

  return result;
}

