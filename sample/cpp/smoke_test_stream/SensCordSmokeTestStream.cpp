/*
 * SPDX-FileCopyrightText: 2017-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <inttypes.h>  // PRIu64
#include <vector>
#include <string>
#include <map>

#include "senscord/senscord.h"
#include "senscord/osal.h"
#include "senscord/osal_inttypes.h"
#include "senscord/rawdata_types.h"
#include "senscord/pseudo_image/pseudo_image_types.h"

// using
using senscord::Core;
using senscord::StreamTypeInfo;
using senscord::Stream;
using senscord::Frame;
using senscord::Channel;
using senscord::UserDataProperty;
using senscord::Status;

/////////////////////////////////////////////////////////////////////
// Test Configure
/////////////////////////////////////////////////////////////////////
#define TEST_STREAM_KEY             ("pseudo_image_stream.0")
#define TEST_GET_FRAME_NUM          (20)
#define TEST_CHANGE_PROPERTY_FRAME  (10)
#define TEST_PROPERTY_KEY           ("PseudoImageProperty")
#define TEST_USER_DATA_SIZE         (16)


/////////////////////////////////////////////////////////////////////
// Test Code
/////////////////////////////////////////////////////////////////////
#define TEST_PRINT(...) \
  do { senscord::osal::OSPrintf("[L%d] ", __LINE__); \
       senscord::osal::OSPrintf(__VA_ARGS__); } while (0)

static void CallbackFrame(Stream* stream, void* priv);
static void CallbackEvent(const std::string& event,
                          const void* param,
                          void* priv);
static Status DoFrame(Frame* frame);
static Status DoFrameChannel(Channel* channel);
static void PrintPseudoImageProperty(const PseudoImageProperty& property);
static Status GetUserdataProperty(Stream* stream, size_t size);
static Status SetUserdataProperty(Stream* stream, void* userdata, size_t size);
static void PrintUserdata(size_t size, const uint8_t* userdata);
static void DoRegisterAccess(Stream* stream);
static void PrintVersion(const senscord::Version& version);
static void PrintStreamVersion(const senscord::SensCordVersion& version);
static void PrintSensCordVersion(const senscord::SensCordVersion& version);

// main
int main(int argc, const char* argv[]) {
  TEST_PRINT("=== SensCordSmokeTestStream Player ===\n");

  Status status;
  Core core;

  // init Core
  status = core.Init();
  TEST_PRINT("Init(): status=%s\n", status.ToString().c_str());
  if (!status.ok()) {
    return -1;
  }

  // get version
  {
    senscord::SensCordVersion version;
    status = core.GetVersion(&version);
    TEST_PRINT("GetVersion(): status=%s\n", status.ToString().c_str());
    if (!status.ok()) {
      return -1;
    }
    PrintSensCordVersion(version);
  }

  // get and print stream list
  {
    std::vector<StreamTypeInfo> list;
    status = core.GetStreamList(&list);
    TEST_PRINT("GetStreamList(): status=%s, size=%" PRIuS "\n",
               status.ToString().c_str(), list.size());
    if (!status.ok()) {
      return -1;
    }
    for (uint32_t i = 0; i < list.size(); ++i) {
      TEST_PRINT(" - %" PRIu32 ": type=%s, key=%s\n",
        i, list[i].type.c_str(), list[i].key.c_str());
    }
  }

  // get opened status #1 (no opened)
  {
    uint32_t count = 0;
    status = core.GetOpenedStreamCount(TEST_STREAM_KEY, &count);
    TEST_PRINT("GetOpenedStreamCount(%s): status=%s, opened=%" PRIu32 "\n",
        TEST_STREAM_KEY, status.ToString().c_str(), count);
    if (!status.ok()) {
      return -1;
    }
  }

  // open image stream.
  Stream* image = NULL;
  status = core.OpenStream(TEST_STREAM_KEY, &image);
  TEST_PRINT("OpenStream(): status=%s, image=%p\n",
             status.ToString().c_str(), image);
  if (!status.ok()) {
    return -1;
  }

  // open image stream #2
  Stream* image2 = NULL;
  senscord::OpenStreamSetting open_config;
  open_config.frame_buffering.buffering = senscord::kBufferingOn;
  open_config.frame_buffering.num = 5;
  open_config.frame_buffering.format = senscord::kBufferingFormatDiscard;
  status = core.OpenStream(TEST_STREAM_KEY, open_config, &image2);
  TEST_PRINT("OpenStream(): status=%s, image=%p\n",
             status.ToString().c_str(), image2);
  if (!status.ok()) {
    return -1;
  }

  // get opened status #2 (opened)
  {
    uint32_t count = 0;
    status = core.GetOpenedStreamCount(TEST_STREAM_KEY, &count);
    TEST_PRINT("GetOpenedStreamCount(%s): status=%s, opened=%" PRIu32 "\n",
        TEST_STREAM_KEY, status.ToString().c_str(), count);
    if (!status.ok()) {
      return -1;
    }
  }

  if (image != NULL) {
    // Event callback register.
    status = image->RegisterEventCallback(senscord::kEventError,
        CallbackEvent, reinterpret_cast<void*>(0x100));
    TEST_PRINT("RegisterEventCallback(): status=%s, type=%s\n",
               status.ToString().c_str(), senscord::kEventError);
    if (!status.ok()) {
      return -1;
    }
    status = image->RegisterEventCallback(senscord::kEventPropertyUpdated,
        CallbackEvent, reinterpret_cast<void*>(0x200));
    TEST_PRINT("RegisterEventCallback(): status=%s, type=%s\n",
               status.ToString().c_str(), senscord::kEventPropertyUpdated);
    if (!status.ok()) {
      return -1;
    }
    status = image->RegisterEventCallback(senscord::kEventFrameDropped,
        CallbackEvent, reinterpret_cast<void*>(0x300));
    TEST_PRINT("RegisterEventCallback(): status=%s, type=%s\n",
               status.ToString().c_str(), senscord::kEventFrameDropped);
    if (!status.ok()) {
      return -1;
    }

    // Get stream informations.
    {
      senscord::StreamKeyProperty property = {};
      status = image->GetProperty(senscord::kStreamKeyPropertyKey, &property);
      TEST_PRINT("GetProperty(StreamKey): status=%s, key=%s\n",
                 status.ToString().c_str(), property.stream_key.c_str());
      if (!status.ok()) {
        return -1;
      }
      if (property.stream_key != TEST_STREAM_KEY) {
        return -1;
      }
    }
    {
      senscord::StreamTypeProperty property = {};
      status = image->GetProperty(senscord::kStreamTypePropertyKey, &property);
      TEST_PRINT("GetProperty(StreamType): status=%s, type=%s\n",
                 status.ToString().c_str(), property.type.c_str());
      if (!status.ok()) {
        return -1;
      }
    }
    {
      senscord::FrameBuffering config = {};
      status = image->GetProperty(senscord::kFrameBufferingPropertyKey,
          &config);
      TEST_PRINT("GetProperty(FrameBuffering): buffering=%d, "
          "num=%" PRId32 ", format=%d\n",
          config.buffering, config.num, config.format);
      if (!status.ok()) {
        return -1;
      }
    }
    {
      senscord::FrameBuffering config = {};
      status = image2->GetProperty(senscord::kFrameBufferingPropertyKey,
          &config);
      TEST_PRINT("GetProperty(FrameBuffering): buffering=%d, "
          "num=%" PRId32 ", format=%d\n",
          config.buffering, config.num, config.format);
      if (!status.ok()) {
        return -1;
      }
    }
    {
      senscord::StreamStateProperty property = {};
      status = image->GetProperty(senscord::kStreamStatePropertyKey, &property);
      TEST_PRINT("GetProperty(StreamState): status=%s, state=%d\n",
                 status.ToString().c_str(), static_cast<int>(property.state));
      if (!status.ok()) {
        return -1;
      }
    }

    {
      std::vector<std::string> list;
      status = image->GetPropertyList(&list);
      TEST_PRINT("GetPropertyList(): status=%s, size=%" PRIuS "\n",
                 status.ToString().c_str(), list.size());
      if (!status.ok()) {
        return -1;
      }
      for (uint32_t i = 0; i < list.size(); ++i) {
        TEST_PRINT(" - %" PRIu32 ": key=%s\n", i, list[i].c_str());
      }
    }

    //===========================================
    // register callback.
    status = image->RegisterFrameCallback(CallbackFrame,
        reinterpret_cast<void*>(0x300));
    TEST_PRINT("RegisterFrameCallback(): status=%s\n",
               status.ToString().c_str());
    if (!status.ok()) {
      return -1;
    }

    //===========================================
    // start stream.
    status = image->Start();
    TEST_PRINT("Start(): status=%s\n", status.ToString().c_str());
    if (!status.ok()) {
      return -1;
    }
    // start stream.
    status = image2->Start();
    TEST_PRINT("Start(): status=%s\n", status.ToString().c_str());
    if (!status.ok()) {
      return -1;
    }

    {
      senscord::StreamStateProperty property = {};
      status = image->GetProperty(senscord::kStreamStatePropertyKey, &property);
      TEST_PRINT("GetProperty(StreamState): status=%s, state=%d\n",
                 status.ToString().c_str(), static_cast<int>(property.state));
      if (!status.ok()) {
        return -1;
      }
    }

    {
      // lock property
      status = image->LockProperty(senscord::kTimeoutForever);
      TEST_PRINT("LockProperty(): status=%s\n", status.ToString().c_str());
      if (!status.ok()) {
        return -1;
      }
    }

    {
      // Get property
      PseudoImageProperty prop;
      status = image->GetProperty(TEST_PROPERTY_KEY, &prop);
      TEST_PRINT("GetProperty(): status=%s\n", status.ToString().c_str());
      if (status.ok()) {
        PrintPseudoImageProperty(prop);
      }

      // Set property
      if (status.ok()) {
        prop.x = 300;
        prop.y = 400;
        prop.z += " fuga";
        status = image->SetProperty(TEST_PROPERTY_KEY, &prop);
        TEST_PRINT("SetProperty(): status=%s\n", status.ToString().c_str());
        if (status.ok()) {
          PrintPseudoImageProperty(prop);
        }
      }
    }

    // Get property #2
    if (status.ok()) {
      PseudoImageProperty prop;
      status = image->GetProperty(TEST_PROPERTY_KEY, &prop);
      TEST_PRINT("GetProperty(): status=%s\n", status.ToString().c_str());
      if (status.ok()) {
        PrintPseudoImageProperty(prop);
      }
    }

    {
      // unlock property
      status = image->UnlockProperty();
      TEST_PRINT("UnlockProperty(): status=%s\n", status.ToString().c_str());
      if (!status.ok()) {
        return -1;
      }
    }

    // general register property get.
    DoRegisterAccess(image);

    {
      // Userdata property
      size_t data_size = TEST_USER_DATA_SIZE;
      status = GetUserdataProperty(image, data_size);
      if (!status.ok()) {
        return -1;
      }

      uint8_t* userdata =
          reinterpret_cast<uint8_t*>(senscord::osal::OSMalloc(data_size));
      if (userdata == NULL) {
        return -1;
      }
      for (uint8_t i = 0; i < data_size; ++i) {
        userdata[i] = (i);
      }
      status = SetUserdataProperty(image, userdata, data_size);
      senscord::osal::OSFree(userdata);
      if (!status.ok()) {
        return -1;
      }

      status = GetUserdataProperty(image, data_size);
      if (!status.ok()) {
        return -1;
      }
    }

    {
      // Get property
      senscord::CurrentFrameNumProperty prop = {};
      status = image->GetProperty(senscord::kCurrentFrameNumPropertyKey, &prop);
      TEST_PRINT("GetProperty(%s): status=%s\n",
                 senscord::kCurrentFrameNumPropertyKey,
                 status.ToString().c_str());
      if (status.ok()) {
        TEST_PRINT(" - ariv: %" PRId32 "\n", prop.arrived_number);
        TEST_PRINT(" - resv: %" PRId32 "\n", prop.received_number);
      }
    }


    for (int cnt = 0; cnt < TEST_GET_FRAME_NUM; ++cnt) {
      // get frame
      Frame* frame = NULL;
      status = image->GetFrame(&frame, senscord::kTimeoutForever);
      if (status.ok()) {
        TEST_PRINT("GetFrame(): status=%s\n", status.ToString().c_str());
        DoFrame(frame);
        senscord::ChannelList list;
        status = frame->GetChannelList(&list);
        TEST_PRINT("GetChannelList(): status=%s, size=%" PRIuS "\n",
                   status.ToString().c_str(), list.size());
        for (senscord::ChannelList::iterator itr = list.begin();
             itr != list.end(); ++itr) {
          DoFrameChannel(itr->second);
        }

        status = image->ReleaseFrame(frame);
        TEST_PRINT("ReleaseFrame(): status=%s\n", status.ToString().c_str());
      }

      status = image2->GetFrame(&frame, 1000);
      TEST_PRINT("GetFrame(): status=%s\n", status.ToString().c_str());
      if (status.ok()) {
        status = image2->ReleaseFrame(frame);
        TEST_PRINT("ReleaseFrame(): status=%s\n", status.ToString().c_str());
      }

      // update property
      if (cnt == TEST_CHANGE_PROPERTY_FRAME) {
        PseudoImageProperty prop;
        status = image->GetProperty(TEST_PROPERTY_KEY, &prop);
        if (status.ok()) {
          TEST_PRINT("GetProperty(): status=%s\n", status.ToString().c_str());
          prop.x += 100;
          prop.y += 100;
          prop.z += " piyo";
          status = image->SetProperty(TEST_PROPERTY_KEY, &prop);
          TEST_PRINT("SetProperty(): status=%s\n", status.ToString().c_str());
        }

        // clear user data
        SetUserdataProperty(image, NULL, 0);

        status = image->UnregisterFrameCallback();
        TEST_PRINT("UnregisterFrameCallback(): status=%s\n",
                   status.ToString().c_str());
      }
    }
    TEST_PRINT("GetFrame(s) done!\n");

    // stop stream.
    status = image->Stop();
    TEST_PRINT("Stop(): status=%s\n", status.ToString().c_str());
    if (!status.ok()) {
      return -1;
    }
    //===========================================

    {
      senscord::StreamStateProperty property = {};
      status = image->GetProperty(senscord::kStreamStatePropertyKey, &property);
      TEST_PRINT("GetProperty(StreamState): status=%s, state=%d\n",
                 status.ToString().c_str(), static_cast<int>(property.state));
      if (!status.ok()) {
        return -1;
      }
    }

    // close stream.
    status = core.CloseStream(image);
    TEST_PRINT("CloseStream(): status=%s\n", status.ToString().c_str());
    if (!status.ok()) {
      return -1;
    }
    image = NULL;
  }

  // stop stream.
  status = image2->Stop();
  TEST_PRINT("Stop(): status=%s\n", status.ToString().c_str());
  if (!status.ok()) {
    return -1;
  }

  // close stream #2
  status = core.CloseStream(image2);
  TEST_PRINT("CloseStream(): status=%s, image=%p\n",
             status.ToString().c_str(), image2);
  if (!status.ok()) {
    return -1;
  }

  // exit Core
  status = core.Exit();
  TEST_PRINT("Exit(): status=%s\n", status.ToString().c_str());
  if (!status.ok()) {
    return -1;
  }

  TEST_PRINT("=== End ===\n");

  // for valgrind, wait 1 sec for free all detached thread resources.
  senscord::osal::OSSleep(static_cast<uint64_t>(1) * 1000 * 1000 * 1000);
  return 0;
}

static void CallbackFrame(Stream* stream, void* priv) {
  TEST_PRINT("Frame arrived!!: stream=%p, priv=%p\n", stream, priv);
}

static void CallbackEvent(const std::string& event, const void* param,
                         void* priv) {
  TEST_PRINT("Event arrived!!: type=%s, param=%p, priv=%p\n",
      event.c_str(), param, priv);
}

static Status DoFrame(Frame* frame) {
  Status status;
  if (frame == NULL) {
    return SENSCORD_STATUS_FAIL("", Status::kCauseUnknown,
        "message");
  }
  {
    uint64_t sequence_number;
    frame->GetSequenceNumber(&sequence_number);
    std::string type;
    frame->GetType(&type);
    TEST_PRINT(" - Frame: number=%" PRIu64 " type=%s\n",
        sequence_number, type.c_str());
  }
  {
    Frame::UserData user_data = {};
    frame->GetUserData(&user_data);
    PrintUserdata(user_data.size,
        static_cast<const uint8_t*>(user_data.address));
  }
  return Status::OK();
}

static void PrintAccelerationData(const senscord::AccelerationData* rawdata) {
  TEST_PRINT("   - RawData(AccelerationData):\n");
  TEST_PRINT("     - acceleration(%f, %f, %f)\n",
      rawdata->x, rawdata->y, rawdata->z);
}

static void PrintAngularVelocityData(
    const senscord::AngularVelocityData* rawdata) {
  TEST_PRINT("   - RawData(AngularVelocityData):\n");
  TEST_PRINT("     - angular_velocity(%f, %f, %f)\n",
      rawdata->x, rawdata->y, rawdata->z);
}

static void PrintMagneticFieldData(const senscord::MagneticFieldData* rawdata) {
  TEST_PRINT("   - RawData(MagneticFieldData):\n");
  TEST_PRINT("     - magnetic_field(%f, %f, %f)\n",
      rawdata->x, rawdata->y, rawdata->z);
}

static void PrintPoseQuaternionData(
    const senscord::PoseQuaternionData* rawdata) {
  TEST_PRINT("   - RawData(PoseData):\n");
  TEST_PRINT("     - position(%f, %f, %f)\n",
      rawdata->position.x, rawdata->position.y, rawdata->position.z);
  TEST_PRINT("     - orientation(%f, %f, %f, %f)\n",
      rawdata->orientation.x, rawdata->orientation.y, rawdata->orientation.z,
      rawdata->orientation.w);
}

static void PrintPoseMatrixData(const senscord::PoseMatrixData* rawdata) {
  TEST_PRINT("   - RawData(PoseData):\n");
  TEST_PRINT("     - position(%f, %f, %f)\n",
      rawdata->position.x, rawdata->position.y, rawdata->position.z);
  TEST_PRINT("     - rotation(%f, %f, %f, %f, %f, %f, %f, %f, %f)\n",
      rawdata->rotation.element[0][0], rawdata->rotation.element[0][1],
      rawdata->rotation.element[0][2], rawdata->rotation.element[1][0],
      rawdata->rotation.element[1][1], rawdata->rotation.element[1][2],
      rawdata->rotation.element[2][0], rawdata->rotation.element[2][1],
      rawdata->rotation.element[2][2]);
}

static Status DoFrameChannel(Channel* channel) {
  Status status;
  if (channel == NULL) {
    return SENSCORD_STATUS_FAIL("", Status::kCauseUnknown,
        "message");
  }

  Channel::RawData raw_data = {};
  {
    uint32_t channel_id;
    channel->GetChannelId(&channel_id);
    channel->GetRawData(&raw_data);
    TEST_PRINT(" - Channel[%" PRIu32 "]: "
        "ch=%p type=%s raw=%p size=%" PRIuS " cap_ts=%" PRIu64 "\n",
        channel_id, channel, raw_data.type.c_str(), raw_data.address,
        raw_data.size, raw_data.timestamp);
  }
  // rawdata
  if (raw_data.address != NULL) {
    senscord::serialize::Decoder decoder(raw_data.address, raw_data.size);
    if (raw_data.type == senscord::kRawDataTypeAcceleration) {
      senscord::AccelerationData deserialized_raw = {};
      decoder.Pop(deserialized_raw);
      PrintAccelerationData(&deserialized_raw);
    } else if (raw_data.type == senscord::kRawDataTypeAngularVelocity) {
      senscord::AngularVelocityData deserialized_raw = {};
      decoder.Pop(deserialized_raw);
      PrintAngularVelocityData(&deserialized_raw);
    } else if (raw_data.type == senscord::kRawDataTypeMagneticField) {
      senscord::MagneticFieldData deserialized_raw = {};
      decoder.Pop(deserialized_raw);
      PrintMagneticFieldData(&deserialized_raw);
    } else if (raw_data.type == senscord::kRawDataTypePose) {
      senscord::PoseDataProperty posedataProperty = {};
      status = channel->GetProperty(
          senscord::kPoseDataPropertyKey, &posedataProperty);
      TEST_PRINT("GetProperty(%s): status=%s\n",
          senscord::kPoseDataPropertyKey, status.ToString().c_str());
      if (status.ok()) {
        TEST_PRINT("PoseDataFormat(%s)\n",
            posedataProperty.data_format.c_str());
        if (posedataProperty.data_format == senscord::kPoseDataFormatMatrix) {
          senscord::PoseMatrixData deserialized_raw = {};
          decoder.Pop(deserialized_raw);
          PrintPoseMatrixData(&deserialized_raw);
        } else if (posedataProperty.data_format ==
                   senscord::kPoseDataFormatQuaternion) {
          senscord::PoseQuaternionData deserialized_raw = {};
          decoder.Pop(deserialized_raw);
          PrintPoseQuaternionData(&deserialized_raw);
        }
      }
    } else {
    // for debug, enable only when necessary
#if 0
      TEST_PRINT("   - Raw :");
      for (size_t i = 0; i < raw_data.size; ++i) {
        senscord::osal::OSPrintf(" %02" PRIX8,
            reinterpret_cast<uint8_t*>(raw_data.address)[i]);
      }
      senscord::osal::OSPrintf("\n");
#endif
    }
  }

  // property
  {
    std::vector<std::string> key_list;
    status = channel->GetPropertyList(&key_list);
    TEST_PRINT("   - GetPropertyList(): status=%s, stored=%" PRIuS "\n",
               status.ToString().c_str(), key_list.size());

    for (uint32_t i = 0; i < key_list.size(); ++i) {
      TEST_PRINT("     - Stored[%" PRIu32 "]: %s\n", i, key_list[i].c_str());
      if (key_list[i] == TEST_PROPERTY_KEY) {
        PseudoImageProperty prop;
        status = channel->GetProperty(key_list[i], &prop);
        if (status.ok()) {
          PrintPseudoImageProperty(prop);
        } else {
          TEST_PRINT("     - GetProperty error!\n");
        }
      }
    }
  }
  {
    std::vector<std::string> key_list;
    status = channel->GetUpdatedPropertyList(&key_list);
    TEST_PRINT("   - GetUpdatedPropertyList(): status=%s, updated=%" PRIuS "\n",
               status.ToString().c_str(), key_list.size());

    for (uint32_t i = 0; i < key_list.size(); ++i) {
      TEST_PRINT("     - Updated[%" PRIu32 "]: %s\n", i, key_list[i].c_str());
    }
  }

  return Status::OK();
}

static void PrintPseudoImageProperty(const PseudoImageProperty& property) {
  TEST_PRINT("     - PseudoImageProperty: x=%" PRIu32 ", y=%" PRIu32 ", z=%s\n",
      property.x, property.y, property.z.c_str());
}

static Status GetUserdataProperty(Stream* stream, size_t size) {
  if (stream == NULL) {
    return SENSCORD_STATUS_FAIL("", Status::kCauseUnknown,
        "message");
  }
  UserDataProperty property;
  Status status =
      stream->GetProperty(senscord::kUserDataPropertyKey, &property);
  TEST_PRINT("GetProperty(userdata): status=%s\n", status.ToString().c_str());
  if (status.ok()) {
    uint8_t* address = NULL;
    if (property.data.size() > 0) {
      address = &property.data[0];
    }
    PrintUserdata(property.data.size(), address);
  }
  return Status::OK();
}

static Status SetUserdataProperty(Stream* stream,
                                  void* userdata,
                                  size_t size) {
  if (stream == NULL) {
    return SENSCORD_STATUS_FAIL("", Status::kCauseUnknown,
        "message");
  }
  Status status;
  if ((userdata != NULL) && (size > 0)) {
    UserDataProperty property;
    property.data.resize(size);
    senscord::osal::OSMemcpy(&property.data[0], size, userdata, size);
    status = stream->SetProperty(senscord::kUserDataPropertyKey, &property);
  } else {
    status = stream->SetProperty(senscord::kUserDataPropertyKey,
                                 reinterpret_cast<UserDataProperty*>(NULL));
  }
  TEST_PRINT("SetProperty(userdata): status=%s\n", status.ToString().c_str());
  return status;
}

static void PrintUserdata(size_t size, const uint8_t* userdata) {
  TEST_PRINT("  - userdata size=%" PRIuS "\n", size);
  for (unsigned int i = 0; i < size; ++i) {
    TEST_PRINT("%02" PRIx8 "\n", userdata[i]);
  }
}

static void DoRegisterAccess(Stream* stream) {
  // create property
  senscord::RegisterAccess64Property prop;

  prop.id = 1000;
  {
    senscord::RegisterAccessElement<uint64_t> elem;
    elem.address = 0x01234567;
    elem.data = 0x50505050;
    prop.element.push_back(elem);
  }
  {
    senscord::RegisterAccessElement<uint64_t> elem;
    elem.address = 0x89ABCDEF;
    elem.data = 0x50505050;
    prop.element.push_back(elem);
  }

  // getting
  Status status = stream->GetProperty(
      senscord::kRegisterAccess64PropertyKey, &prop);
  TEST_PRINT("GetProperty(%s): status=%s\n",
      senscord::kRegisterAccess64PropertyKey, status.ToString().c_str());
  if (status.ok()) {
    // print result
    TEST_PRINT(" - id: %" PRIu32 "\n", prop.id);
    TEST_PRINT(" - element.size: %" PRIuS "\n", prop.element.size());
    std::vector<senscord::RegisterAccessElement<uint64_t> >::iterator it =
        prop.element.begin();
    for (; it != prop.element.end(); ++it) {
      TEST_PRINT("   - adr: 0x%" PRIxPTR "\n", (*it).address);
      TEST_PRINT("      - 0x%08" PRIx64 "\n", (*it).data);
    }
  }
}

static void PrintVersion(const senscord::Version& version) {
  TEST_PRINT(" - name : %s\n", version.name.c_str());
  TEST_PRINT(" - major: %" PRIu32 "\n", version.major);
  TEST_PRINT(" - minor: %" PRIu32 "\n", version.minor);
  TEST_PRINT(" - patch: %" PRIu32 "\n", version.patch);
  TEST_PRINT(" - description: %s\n", version.description.c_str());
}

static void PrintStreamVersion(const senscord::SensCordVersion& version) {
  typedef std::map<std::string, senscord::StreamVersion>::const_iterator sitr;
  sitr itr = version.stream_versions.begin();
  sitr end = version.stream_versions.end();
  for (; itr != end; ++itr) {
    TEST_PRINT(" [stream(%s)]\n", itr->first.c_str());
    PrintVersion(itr->second.stream_version);
    TEST_PRINT(" - destination id: %" PRId32 "\n", itr->second.destination_id);
    typedef std::vector<senscord::Version>::const_iterator linkage_itr;
    linkage_itr link_itr = itr->second.linkage_versions.begin();
    linkage_itr link_end = itr->second.linkage_versions.end();
    for (uint32_t i = 1 ; link_itr != link_end; ++link_itr, ++i) {
      TEST_PRINT(" --- linkage_version(%" PRIu32 "/%" PRIuS "):\n",
          i, itr->second.linkage_versions.size());
      PrintVersion(*link_itr);
    }
  }
}

static void PrintSensCordVersion(const senscord::SensCordVersion& version) {
  TEST_PRINT(" [senscord]\n");
  PrintVersion(version.senscord_version);
  TEST_PRINT(" [project]\n");
  PrintVersion(version.project_version);
  PrintStreamVersion(version);
  typedef std::map<int32_t, senscord::SensCordVersion>::const_iterator sitr;
  sitr itr = version.server_versions.begin();
  sitr end = version.server_versions.end();
  for (uint32_t i = 1; itr != end; ++itr, ++i) {
    TEST_PRINT("---[Server id: %" PRId32 " (%" PRIu32 "/%" PRIuS ")]---\n",
        itr->first, i, version.server_versions.size());
    PrintSensCordVersion(itr->second);
  }
}
