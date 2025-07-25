/*
 * SPDX-FileCopyrightText: 2017-2024 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <stdio.h>     /* printf */
#include <stdlib.h>    /* malloc, free */
#include <string.h>    /* strcat, strcmp */
#include <inttypes.h>  /* PRIu64 */

#include "senscord/c_api/senscord_c_api.h"
#include "senscord/osal_inttypes.h"

struct PseudoImagePropertyC {
  int x;
  int y;
  char z[128];
};

/* ===============================================================
 * Test Configure
 * =============================================================== */
#define TEST_STREAM_KEY             ("pseudo_image_stream.0")
#define TEST_GET_FRAME_NUM          (20)
#define TEST_CHANGE_PROPERTY_FRAME  (10)
#define TEST_PROPERTY_KEY           ("PseudoImageProperty")
#define TEST_USER_DATA_SIZE         (16)

/* ===============================================================
 * Test Code
 * =============================================================== */
#ifndef __wasm__
#ifdef _WIN32
#define TEST_PRINT(...) \
  do { printf_s("[L%d] ", __LINE__); printf_s(__VA_ARGS__); } while (0)
#define TEST_STRCAT(dest, src) strcat_s(dest, sizeof(dest), src)
#else
#define TEST_PRINT(...) \
  do { printf("[L%d] ", __LINE__); printf(__VA_ARGS__); } while (0)
#define TEST_STRCAT(dest, src) strncat(dest, src, sizeof(src))
#endif  /* _WIN32 */
#else
#define TEST_PRINT(...) \
  do { printf("[L%d] ", __LINE__); printf(__VA_ARGS__); } while (0)
#define TEST_STRCAT(dest, src)
#endif  /* __wasm__ */

static void CallbackFrame(senscord_stream_t stream, void* priv);
static void CallbackEvent(
    const char* event, const void* param, void* priv);
static void CallbackEvent2(
    senscord_stream_t stream, const char* event,
    senscord_event_argument_t args, void* priv);
static int32_t DoFrame(senscord_frame_t frame);
static int32_t DoFrameChannel(senscord_channel_t channel);
static void PrintPseudoImageProperty(
    const struct PseudoImagePropertyC* property);
static int32_t GetUserdataProperty(senscord_stream_t stream, size_t size);
static int32_t SetUserdataProperty(senscord_stream_t stream,
                                   void* userdata, size_t size);
static void PrintUserdata(size_t size, const uint8_t* userdata);
static void DoRegisterAccess(senscord_stream_t stream);
static void PrintError(void);
static void PrintSensCordVersion(struct senscord_version_t* version);

/* main */
int main(int argc, const char* argv[]) {
  int32_t ret = 0;
  senscord_core_t core;
  senscord_stream_t image;
  senscord_stream_t image2;

  TEST_PRINT("=== SimpleStream Player ===\n");
  /* init Core */
  ret = senscord_core_init(&core);
  TEST_PRINT("senscord_core_init(): ret=%" PRId32 "\n", ret);
  if (ret != 0) {
    PrintError();
    return -1;
  }

  /* get version */
  {
    struct senscord_version_t version;
    ret = senscord_core_get_version(core, &version);
    TEST_PRINT("senscord_core_get_version(): ret=%" PRId32 "\n", ret);
    if (ret == 0) {
      PrintSensCordVersion(&version);
    } else {
      PrintError();
    }
  }

  /* get and print stream list */
  {
    uint32_t count = 0;
    uint32_t index = 0;
    ret = senscord_core_get_stream_count(core, &count);
    TEST_PRINT("senscord_core_get_stream_count(): ret=%" PRId32
               ", count=%" PRId32 "\n", ret, count);
    if (ret != 0) {
      PrintError();
      return -1;
    }
    for (index = 0; index < count; ++index) {
      char buffer[64];
      uint32_t length = 0;
      length = sizeof(buffer);
      ret = senscord_core_get_stream_info_string(
          core, index, SENSCORD_STREAM_INFO_STREAM_KEY, buffer, &length);
      if (ret == 0) {
        TEST_PRINT(" - key=%s\n", buffer);
        length = sizeof(buffer);
        ret = senscord_core_get_stream_info_string(
            core, index, SENSCORD_STREAM_INFO_STREAM_TYPE, buffer, &length);
        if (ret == 0) {
          TEST_PRINT("    - type=%s\n", buffer);
        }
      }
    }
  }

  /* get opened status #1 (no opened) */
  {
    uint32_t opened_count = 0;
    ret = senscord_core_get_opened_stream_count(
        core, TEST_STREAM_KEY, &opened_count);
    TEST_PRINT("senscord_core_get_opened_stream_count(): ret=%" PRId32
               ", opened=%" PRIu32 "\n", ret, opened_count);
    if (ret != 0) {
      PrintError();
      return -1;
    }
  }

  /* open image stream */
  ret = senscord_core_open_stream(core, TEST_STREAM_KEY, &image);
  TEST_PRINT("senscord_core_open_stream(): ret=%" PRId32 "\n", ret);
  if (ret != 0) {
    PrintError();
    return -1;
  }

  /* open image stream #2 */
  {
    struct senscord_open_stream_setting_t oepn_config = {0};
    oepn_config.frame_buffering.buffering = SENSCORD_BUFFERING_ON;
    oepn_config.frame_buffering.num = 5;
    oepn_config.frame_buffering.format = SENSCORD_BUFFERING_FORMAT_DISCARD;
    ret = senscord_core_open_stream_with_setting(
        core, TEST_STREAM_KEY, &oepn_config, &image2);
    TEST_PRINT("senscord_core_open_stream_with_setting(): ret=%" PRId32 "\n",
              ret);
    if (ret != 0) {
      PrintError();
      return -1;
    }
  }

  /* get opened status #2 (opened) */
  {
    uint32_t opened_count = 0;
    ret = senscord_core_get_opened_stream_count(
        core, TEST_STREAM_KEY, &opened_count);
    TEST_PRINT("senscord_core_get_opened_stream_count(): ret=%" PRId32
               ", opened=%" PRIu32 "\n", ret, opened_count);
    if (ret != 0) {
      PrintError();
      return -1;
    }
  }

  {
    /* Event callback register. */
    ret = senscord_stream_register_event_callback(
        image, SENSCORD_EVENT_ANY, CallbackEvent, (void*)0x100);
    TEST_PRINT("senscord_stream_register_event_callback(): ret=%" PRId32
               ", type=%s\n", ret, SENSCORD_EVENT_ANY);
    if (ret != 0) {
      PrintError();
      return -1;
    }
    ret = senscord_stream_register_event_callback2(
        image2, SENSCORD_EVENT_ANY, CallbackEvent2, (void*)0x200);
    TEST_PRINT("senscord_stream_register_event_callback2(): ret=%" PRId32
               ", type=%s\n", ret, SENSCORD_EVENT_ANY);
    if (ret != 0) {
      PrintError();
      return -1;
    }

    /* Get stream informations. */
    {
      /* StreamKeyProperty */
      struct senscord_stream_key_property_t property = {0};
      ret = senscord_stream_get_property(
          image, SENSCORD_STREAM_KEY_PROPERTY_KEY,
          &property, sizeof(property));
      TEST_PRINT("senscord_stream_get_property(StreamKey): ret=%" PRId32
                 ", key=%s\n",
                 ret, property.stream_key);
      if (ret != 0) {
        PrintError();
        return -1;
      }
    }
    {
      /* StreamTypeProperty */
      struct senscord_stream_type_property_t property = {0};
      ret = senscord_stream_get_property(
          image, SENSCORD_STREAM_TYPE_PROPERTY_KEY,
          &property, sizeof(property));
      TEST_PRINT("senscord_stream_get_property(StreamType): ret=%" PRId32
                 ", type=%s\n",
                 ret, property.type);
      if (ret != 0) {
        PrintError();
        return -1;
      }
    }
    {
      /* FrameBuffering image1 */
      struct senscord_frame_buffering_property_t config = {0};
      ret = senscord_stream_get_property(
          image, SENSCORD_FRAME_BUFFERING_PROPERTY_KEY,
          &config, sizeof(config));
      TEST_PRINT("senscord_stream_get_property(FrameBuffering[1]): ret=%"
                 PRId32 "\n", ret);
      if (ret != 0) {
        PrintError();
        return -1;
      }
      TEST_PRINT(" - buffering=%d, num=%" PRId32 ", format=%d\n",
                 config.buffering, config.num, config.format);
    }
    {
      /* FrameBuffering image2 */
      struct senscord_frame_buffering_property_t config = {0};
      ret = senscord_stream_get_property(
          image2, SENSCORD_FRAME_BUFFERING_PROPERTY_KEY,
          &config, sizeof(config));
      TEST_PRINT("senscord_stream_get_property(FrameBuffering[2]): ret=%"
                 PRId32 "\n", ret);
      if (ret != 0) {
        PrintError();
        return -1;
      }
      TEST_PRINT(" - buffering=%d, num=%" PRId32 ", format=%d\n",
                 config.buffering, config.num, config.format);
    }
    {
      /* StreamStateProperty */
      struct senscord_stream_state_property_t property = {0};
      ret = senscord_stream_get_property(
          image, SENSCORD_STREAM_STATE_PROPERTY_KEY,
          &property, sizeof(property));
      TEST_PRINT("senscord_stream_get_property(StreamState): ret=%" PRId32
                 ", state=%d\n", ret, property.state);
      if (ret != 0) {
        PrintError();
        return -1;
      }
    }

    {
      uint32_t count = 0;
      uint32_t index = 0;
      ret = senscord_stream_get_property_count(image, &count);
      TEST_PRINT("senscord_stream_get_property_count(): "
                 "ret=%" PRId32 ", count=%" PRIu32 "\n", ret, count);
      if (ret != 0) {
        PrintError();
        return -1;
      }
      for (index = 0; index < count; ++index) {
        char key[64];
        uint32_t length = sizeof(key);
        ret = senscord_stream_get_property_key_string(
            image, index, key, &length);
        if (ret == 0) {
          TEST_PRINT(" - %" PRIu32 ": key=%s\n", index, key);
        } else {
          TEST_PRINT(" - %" PRIu32 ": failed. ret=%" PRId32 "\n", index, ret);
          PrintError();
        }
      }
    }

    /*===========================================*/
    /* register callback. */
    ret = senscord_stream_register_frame_callback(
        image, CallbackFrame, (void*)0x300);
    TEST_PRINT("senscord_stream_register_frame_callback(): ret=%" PRId32 "\n",
               ret);
    if (ret != 0) {
      PrintError();
      return -1;
    }

    /*===========================================*/
    /* start stream. */
    ret = senscord_stream_start(image);
    TEST_PRINT("senscord_stream_start(): ret=%" PRId32 "\n", ret);
    if (ret != 0) {
      PrintError();
      return -1;
    }
    /* start stream. */
    ret = senscord_stream_start(image2);
    TEST_PRINT("senscord_stream_start(): ret=%" PRId32 "\n", ret);
    if (ret != 0) {
      PrintError();
      return -1;
    }

    {
      /* StreamStateProperty */
      struct senscord_stream_state_property_t property = {0};
      ret = senscord_stream_get_property(
          image, SENSCORD_STREAM_STATE_PROPERTY_KEY,
          &property, sizeof(property));
      TEST_PRINT("senscord_stream_get_property(StreamState): ret=%" PRId32
                 ", state=%d\n", ret, property.state);
      if (ret != 0) {
        PrintError();
        return -1;
      }
    }

    {
      /* Get property */
      struct PseudoImagePropertyC prop = {0};
      ret = senscord_stream_get_property(
          image, TEST_PROPERTY_KEY, &prop, sizeof(prop));
      TEST_PRINT("senscord_stream_get_property(Pseudo): ret=%" PRId32 "\n",
                 ret);
      if (ret == 0) {
        PrintPseudoImageProperty(&prop);
      } else {
        PrintError();
      }

      /* Set property */
      if (ret == 0) {
        prop.x = 300;
        prop.y = 400;
        TEST_STRCAT(prop.z, " fuga");
        ret = senscord_stream_set_property(
            image, TEST_PROPERTY_KEY, &prop, sizeof(prop));
        TEST_PRINT("senscord_stream_set_property(Pseudo): ret=%" PRId32 "\n",
                   ret);
        if (ret == 0) {
          PrintPseudoImageProperty(&prop);
        } else {
          PrintError();
        }
      }
    }

    /* Get property #2 */
    if (ret == 0) {
      struct PseudoImagePropertyC prop = {0};
      ret = senscord_stream_get_property(
          image, TEST_PROPERTY_KEY, &prop, sizeof(prop));
      TEST_PRINT("senscord_stream_get_property(Pseudo): ret=%" PRId32 "\n",
                 ret);
      if (ret == 0) {
        PrintPseudoImageProperty(&prop);
      } else {
        PrintError();
      }
    }

    /* general register property get. */
    DoRegisterAccess(image);

    {
      /* Userdata property */
      size_t data_size = TEST_USER_DATA_SIZE;
      ret = GetUserdataProperty(image, data_size);
      if (ret != 0) {
        PrintError();
        return -1;
      }

      uint8_t* userdata = (uint8_t*)(malloc(data_size));
      if (userdata == NULL) {
        return -1;
      }
      unsigned int i = 0;
      for (i = 0; i < data_size; ++i) {
        userdata[i] = (i);
      }
      ret = SetUserdataProperty(image, userdata, data_size);
      if (ret != 0) {
        free(userdata);
        PrintError();
        return -1;
      }
      free(userdata);

      ret = GetUserdataProperty(image, data_size);
      if (ret != 0) {
        PrintError();
        return -1;
      }
    }

    {
      /* Get property */
      struct senscord_current_frame_num_property_t prop = {0};
      ret = senscord_stream_get_property(
          image, SENSCORD_CURRENT_FRAME_NUM_PROPERTY_KEY,
          &prop, sizeof(prop));
      TEST_PRINT("senscord_stream_get_property(%s): ret=%" PRId32 "\n",
                 SENSCORD_CURRENT_FRAME_NUM_PROPERTY_KEY, ret);
      if (ret == 0) {
        TEST_PRINT(" - ariv: %" PRId32 "\n", prop.arrived_number);
        TEST_PRINT(" - resv: %" PRId32 "\n", prop.received_number);
      } else {
        PrintError();
      }
    }

    int cnt = 0;
    for (cnt = 0; cnt < TEST_GET_FRAME_NUM; ++cnt) {
      /* get frame */
      senscord_frame_t frame;
      ret = senscord_stream_get_frame(image, &frame,
                                      SENSCORD_TIMEOUT_FOREVER);
      if (ret == 0) {
        uint32_t count = 0;
        uint32_t index = 0;
        TEST_PRINT("senscord_stream_get_frame(): ret=%" PRId32 "\n", ret);
        DoFrame(frame);
        ret = senscord_frame_get_channel_count(frame, &count);
        TEST_PRINT("senscord_frame_get_channel_count(): ret=%" PRId32
                   ", count=%" PRIu32 "\n", ret, count);
        for (index = 0; index < count; ++index) {
          senscord_channel_t channel;
          ret = senscord_frame_get_channel(frame, index, &channel);
          TEST_PRINT("senscord_frame_get_channel(): ret=%" PRId32 "\n", ret);
          if (ret == 0) {
            DoFrameChannel(channel);
          }
        }

        ret = senscord_stream_release_frame(image, frame);
        TEST_PRINT("senscord_stream_release_frame(): ret=%" PRId32 "\n", ret);
      }

      ret = senscord_stream_get_frame(image2, &frame, 1000);
      TEST_PRINT("senscord_stream_get_frame(): ret=%" PRId32 "\n", ret);
      if (ret == 0) {
        ret = senscord_stream_release_frame(image2, frame);
        TEST_PRINT("senscord_stream_release_frame(): ret=%" PRId32 "\n", ret);
      }

      /* update property */
      if (cnt == TEST_CHANGE_PROPERTY_FRAME) {
        struct PseudoImagePropertyC prop = {0};
        ret = senscord_stream_get_property(
            image, TEST_PROPERTY_KEY, &prop, sizeof(prop));
        TEST_PRINT("senscord_stream_get_property(Pseudo): ret=%" PRId32 "\n",
                   ret);
        if (ret == 0) {
          prop.x += 100;
          prop.y += 100;
          TEST_STRCAT(prop.z, " piyo");
          ret = senscord_stream_set_property(
              image, TEST_PROPERTY_KEY, &prop, sizeof(prop));
          TEST_PRINT("senscord_stream_set_property(Pseudo): ret=%" PRId32 "\n",
                     ret);
        }

        /* clear user data */
        SetUserdataProperty(image, NULL, 0);

        ret = senscord_stream_unregister_frame_callback(image);
        TEST_PRINT("senscord_stream_unregister_frame_callback(): ret=%"
                   PRId32 "\n", ret);
      }
    }
    TEST_PRINT("senscord_stream_get_frame(s) done!\n");

    /* stop stream. */
    ret = senscord_stream_stop(image);
    TEST_PRINT("senscord_stream_stop(): ret=%" PRId32 "\n", ret);
    if (ret != 0) {
      PrintError();
      return -1;
    }

    {
      /* StreamStateProperty */
      struct senscord_stream_state_property_t property = {0};
      ret = senscord_stream_get_property(
          image, SENSCORD_STREAM_STATE_PROPERTY_KEY,
          &property, sizeof(property));
      TEST_PRINT("senscord_stream_get_property(StreamState): ret=%" PRId32
                 ", state=%d\n", ret, property.state);
      if (ret != 0) {
        PrintError();
        return -1;
      }
    }

    /* close stream. */
    ret = senscord_core_close_stream(core, image);
    TEST_PRINT("senscord_core_close_stream(): ret=%" PRId32 "\n", ret);
    if (ret != 0) {
      PrintError();
      return -1;
    }
  }

  /* stop stream. */
  ret = senscord_stream_stop(image2);
  TEST_PRINT("senscord_stream_stop(): ret=%" PRId32 "\n", ret);
  if (ret != 0) {
    PrintError();
    return -1;
  }

  /* close stream #2 */
  ret = senscord_core_close_stream(core, image2);
  TEST_PRINT("senscord_core_close_stream(): ret=%" PRId32 "\n", ret);
  if (ret != 0) {
    PrintError();
    return -1;
  }

  /* exit Core */
  ret = senscord_core_exit(core);
  TEST_PRINT("senscord_core_exit(): ret=%" PRId32 "\n", ret);
  if (ret != 0) {
    PrintError();
    return -1;
  }

  TEST_PRINT("=== End ===\n");

#if 0
  /* TODO: sleep */
  /* for valgrind, wait 1 sec for free all detached thread resources. */
  senscord::osal::OSSleep(static_cast<uint64_t>(1) * 1000 * 1000 * 1000);
#endif
  return 0;
}

static void CallbackFrame(senscord_stream_t stream, void* priv) {
  int32_t ret = 0;
  TEST_PRINT("Frame arrived!!: stream=%" PRIx64 ", priv=%p\n", stream, priv);

  {
    /* StreamKeyProperty */
    struct senscord_stream_key_property_t property = {0};
    ret = senscord_stream_get_property(
        stream, SENSCORD_STREAM_KEY_PROPERTY_KEY,
        &property, sizeof(property));
    TEST_PRINT(" - StreamKey: %s\n", property.stream_key);
    if (ret != 0) {
      PrintError();
    }
  }
  {
    /* StreamTypeProperty */
    struct senscord_stream_type_property_t property = {0};
    ret = senscord_stream_get_property(
        stream, SENSCORD_STREAM_TYPE_PROPERTY_KEY,
        &property, sizeof(property));
    TEST_PRINT(" - StreamType: %s\n", property.type);
    if (ret != 0) {
      PrintError();
    }
  }
}

static void CallbackEvent(const char* event, const void* param, void* priv) {
  TEST_PRINT("Event arrived!!: type=%s, param=%p, priv=%p\n",
      event, param, priv);
}

static void CallbackEvent2(
    senscord_stream_t stream, const char* event,
    senscord_event_argument_t args, void* priv) {
  TEST_PRINT("Event arrived!!: stream=%" PRIx64 ", type=%s, priv=%p\n",
      stream, event, priv);

  uint32_t count = 0;
  int32_t ret = senscord_event_argument_get_element_count(args, &count);

  for (uint32_t index = 0; index < count; ++index) {
    char key[64];
    uint32_t length = sizeof(key);
    ret = senscord_event_argument_get_key_string(args, index, key, &length);
    if (ret == 0) {
      TEST_PRINT("EventArgument key[%" PRIu32 "]=%s\n", index, key);
    }
  }
}

static int32_t DoFrame(senscord_frame_t frame) {
  int32_t ret = 0;
  uint64_t seq_num = 0;
  const char* type = NULL;
  struct senscord_user_data_t user_data = {0};
  if (frame == 0) {
    return -1;
  }
  ret = senscord_frame_get_sequence_number(frame, &seq_num);
  if (ret != 0) {
    TEST_PRINT("senscord_frame_get_sequence_number(): ret=%" PRId32 "\n", ret);
    PrintError();
  }
  ret = senscord_frame_get_type(frame, &type);
  if (ret != 0) {
    TEST_PRINT("senscord_frame_get_type(): ret=%" PRId32 "\n", ret);
    PrintError();
  }
  TEST_PRINT(" - Frame: number=%" PRIu64 " type=%s\n", seq_num, type);
  ret = senscord_frame_get_user_data(frame, &user_data);
  if (ret == 0) {
    PrintUserdata(user_data.size, (const uint8_t*)user_data.address);
  } else {
    TEST_PRINT("senscord_frame_get_user_data(): ret=%" PRId32 "\n", ret);
    PrintError();
  }
  return 0;
}

static void PrintVector3Data(
    const char* type, const struct senscord_vector3f_t* data) {
  TEST_PRINT("   - RawData(%s):\n", type);
  TEST_PRINT("     - %f, %f, %f\n", data->x, data->y, data->z);
}

static void PrintRotationData(const struct senscord_rotation_data_t* data) {
  TEST_PRINT("   - RawData(RotationData):\n");
  TEST_PRINT("     - roll=%f, pitch=%f, yaw=%f\n",
             data->roll, data->pitch, data->yaw);
}

static void PrintPoseQuaternionData(
    const struct senscord_pose_quaternion_data_t* rawdata) {
  TEST_PRINT("   - RawData(PoseQuaternionData):\n");
  TEST_PRINT("     - position(%f, %f, %f)\n",
      rawdata->position.x, rawdata->position.y, rawdata->position.z);
  TEST_PRINT("     - orientation(%f, %f, %f, %f)\n",
      rawdata->orientation.x, rawdata->orientation.y, rawdata->orientation.z,
      rawdata->orientation.w);
}

static void PrintPoseMatrixData(
    const struct senscord_pose_matrix_data_t* rawdata) {
  TEST_PRINT("   - RawData(PoseMatrixData):\n");
  TEST_PRINT("     - position(%f, %f, %f)\n",
      rawdata->position.x, rawdata->position.y, rawdata->position.z);
  TEST_PRINT("     - rotation(%f, %f, %f, %f, %f, %f, %f, %f, %f)\n",
      rawdata->rotation.element[0][0], rawdata->rotation.element[0][1],
      rawdata->rotation.element[0][2], rawdata->rotation.element[1][0],
      rawdata->rotation.element[1][1], rawdata->rotation.element[1][2],
      rawdata->rotation.element[2][0], rawdata->rotation.element[2][1],
      rawdata->rotation.element[2][2]);
}

static int32_t DoFrameChannel(senscord_channel_t channel) {
  int32_t ret = 0;
  uint32_t channel_id = 0;
  struct senscord_raw_data_t raw_data = {0};
  if (channel == 0) {
    return -1;
  }
  ret = senscord_channel_get_channel_id(channel, &channel_id);
  if (ret != 0) {
    TEST_PRINT("senscord_channel_get_channel_id(): ret=%" PRId32 "\n",
               ret);
    PrintError();
  }
  ret = senscord_channel_get_raw_data(channel, &raw_data);
  if (ret != 0) {
    TEST_PRINT("senscord_channel_get_raw_data(): ret=%" PRId32 "\n",
               ret);
    PrintError();
  }

  TEST_PRINT(" - Channel[%" PRIu32 "]: type=%s, raw=%p, size=%" PRIuS
             ", cap_ts=%" PRIu64 "\n",
             channel_id,
             raw_data.type,
             raw_data.address,
             raw_data.size,
             raw_data.timestamp);

  /* rawdata */
  if (raw_data.address != NULL) {
    if ((strcmp(raw_data.type, SENSCORD_RAW_DATA_TYPE_ACCELERATION) == 0) ||
        (strcmp(raw_data.type, SENSCORD_RAW_DATA_TYPE_ANGULAR_VELOCITY) == 0) ||
        (strcmp(raw_data.type, SENSCORD_RAW_DATA_TYPE_MAGNETIC_FIELD) == 0)) {
      struct senscord_vector3f_t value = { 0 };
      ret = senscord_channel_convert_rawdata(channel, &value, sizeof(value));
      if (ret == 0) {
        PrintVector3Data(raw_data.type, &value);
      } else {
        PrintError();
      }
    }
    if (strcmp(raw_data.type, SENSCORD_RAW_DATA_TYPE_ROTATION) == 0) {
      struct senscord_rotation_data_t value = { 0 };
      ret = senscord_channel_convert_rawdata(channel, &value, sizeof(value));
      if (ret == 0) {
        PrintRotationData(&value);
      } else {
        PrintError();
      }
    }
  }

  /* property */
  {
    uint32_t count = 0;
    ret = senscord_channel_get_property_count(channel, &count);
    TEST_PRINT("   - senscord_channel_get_property_count(): "
               "ret=%" PRId32 ", count=%" PRIu32 "\n", ret, count);
    if (ret == 0) {
      uint32_t index = 0;
      for (index = 0; index < count; ++index) {
        char key[64];
        uint32_t length = sizeof(key);
        ret = senscord_channel_get_property_key_string(
            channel, index, key, &length);
        if (ret == 0) {
          TEST_PRINT("     - Stored[%" PRIu32 "]: %s\n",
                     index, key);
          if (strcmp(key, TEST_PROPERTY_KEY) == 0) {
            struct PseudoImagePropertyC prop = {0};
            ret = senscord_channel_get_property(
                channel, key, &prop, sizeof(prop));
            if (ret == 0) {
              PrintPseudoImageProperty(&prop);
            } else {
              TEST_PRINT("     - GetProperty error!\n");
              PrintError();
            }
          }
        } else {
          TEST_PRINT("     - Stored[%" PRIu32 "]: failed. ret=%" PRId32 "\n",
                     index, ret);
          PrintError();
        }
      }
    }
  }
  {
    uint32_t count = 0;
    ret = senscord_channel_get_updated_property_count(channel, &count);
    TEST_PRINT("   - senscord_channel_get_updated_property_count(): "
               "ret=%" PRId32 ", count=%" PRIu32 "\n", ret, count);
    if (ret == 0) {
      uint32_t index = 0;
      for (index = 0; index < count; ++index) {
        char key[64];
        uint32_t length = sizeof(key);
        ret = senscord_channel_get_updated_property_key_string(
            channel, index, key, &length);
        if (ret == 0) {
          TEST_PRINT("     - Updated[%" PRIu32 "]: %s\n",
                     index, key);
        } else {
          TEST_PRINT("     - Updated[%" PRIu32 "]: failed. ret=%" PRId32 "\n",
                     index, ret);
          PrintError();
        }
      }
    }
  }

  return 0;
}

static void PrintPseudoImageProperty(
    const struct PseudoImagePropertyC* property) {
  TEST_PRINT("     - PseudoImageProperty: x=%" PRIu32 ", y=%" PRIu32
             ", z=%s\n", property->x, property->y, property->z);
}

static int32_t GetUserdataProperty(senscord_stream_t stream, size_t size) {
  int32_t ret = 0;
  void* property = NULL;
  if (stream == 0) {
    return -1;
  }
  property = malloc(size);
  if (property == NULL) {
    return -1;
  }
  ret = senscord_stream_get_userdata_property(stream, property, size);
  TEST_PRINT("senscord_stream_get_userdata_property(): ret=%" PRId32 "\n",
             ret);
  if (ret == 0) {
    PrintUserdata(size, (uint8_t*)property);
  } else {
    PrintError();
  }
  free(property);
  return 0;
}

static int32_t SetUserdataProperty(senscord_stream_t stream,
                                   void* userdata,
                                   size_t size) {
  int32_t ret = 0;
  if (stream == 0) {
    return -1;
  }
  ret = senscord_stream_set_userdata_property(stream, userdata, size);
  TEST_PRINT("senscord_stream_set_userdata_property(): ret=%" PRId32 "\n",
             ret);
  if (ret != 0) {
    PrintError();
  }
  return ret;
}

static void PrintUserdata(size_t size, const uint8_t* userdata) {
  unsigned int i = 0;
  TEST_PRINT("  - size=%" PRIuS "\n", size);
  for (i = 0; i < size;) {
    TEST_PRINT("%02" PRIx8 " ", userdata[i]);
    ++i;
    if ((i % 16) == 0) {
      TEST_PRINT("\n");
    }
  }
}

#define REGISTER_ELEMENT_DATA_SIZE 64

static void DoRegisterAccess(senscord_stream_t stream) {
  /* create property */
  struct senscord_register_access_64_property_t prop1 = {0};
  struct senscord_register_access_64_property_t prop2 = {0};

  prop1.id = 1000;
  prop1.address = 0x01234567;
  prop1.data = 0x50505050;

  prop2.id = 1000;
  prop2.address = 0x89ABCDEF;
  prop2.data = 0x50505050;

  /* getting */
  int32_t ret = senscord_stream_get_property(stream,
      SENSCORD_REGISTER_ACCESS_64_PROPERTY_KEY, &prop1, sizeof(prop1));
  TEST_PRINT("stream_get_property(%s): ret=%" PRId32 "\n",
             SENSCORD_REGISTER_ACCESS_64_PROPERTY_KEY, ret);
  if (ret == 0) {
    // print result
    TEST_PRINT(" - id: %" PRIu32 "\n", prop1.id);
    TEST_PRINT(" - addr: 0x%" PRIx64 "\n", prop1.address);
    TEST_PRINT(" - data: 0x%08" PRIx64 "\n", prop1.data);
  } else {
    PrintError();
  }

  ret = senscord_stream_get_property(stream,
      SENSCORD_REGISTER_ACCESS_64_PROPERTY_KEY, &prop2, sizeof(prop2));
  TEST_PRINT("stream_get_property(%s): ret=%" PRId32 "\n",
             SENSCORD_REGISTER_ACCESS_64_PROPERTY_KEY, ret);
  if (ret == 0) {
    // print result
    TEST_PRINT(" - id: %" PRIu32 "\n", prop2.id);
    TEST_PRINT(" - addr: 0x%" PRIx64 "\n", prop2.address);
    TEST_PRINT(" - data: 0x%08" PRIx64 "\n", prop2.data);
  } else {
    PrintError();
  }
}

static void PrintError(void) {
  enum senscord_error_cause_t cause = senscord_get_last_error_cause();
  if (cause == SENSCORD_ERROR_NONE) {
    TEST_PRINT("status: OK\n");
  } else {
    enum senscord_error_level_t level = senscord_get_last_error_level();
    TEST_PRINT("status: level=%d, cause=%d\n", level, cause);
    char buffer[256];
    uint32_t length = 0;
    // message
    length = sizeof(buffer);
    if (senscord_get_last_error_string(
        SENSCORD_STATUS_PARAM_MESSAGE, buffer, &length) == 0) {
      TEST_PRINT(" - message: %s\n", buffer);
    }
    // block
    length = sizeof(buffer);
    if (senscord_get_last_error_string(
        SENSCORD_STATUS_PARAM_BLOCK, buffer, &length) == 0) {
      TEST_PRINT(" - block  : %s\n", buffer);
    }
    // trace
    length = sizeof(buffer);
    if (senscord_get_last_error_string(
        SENSCORD_STATUS_PARAM_TRACE, buffer, &length) == 0) {
      TEST_PRINT(" - trace  : %s\n", buffer);
    }
  }
}

static void PrintVersion(struct senscord_version_property_t* version) {
  TEST_PRINT("  - name : %s\n", version->name);
  TEST_PRINT("  - major: %" PRIu32 "\n", version->major);
  TEST_PRINT("  - minor: %" PRIu32 "\n", version->minor);
  TEST_PRINT("  - patch: %" PRIu32 "\n", version->patch);
  TEST_PRINT("  - description: %s\n", version->description);
}

static void PrintStreamVersion(struct senscord_version_t* version) {
  uint32_t i = 0;
  uint32_t j = 0;
  for (i = 0; i < version->stream_count; ++i) {
    struct senscord_stream_version_t* stream = &version->stream_versions[i];
    TEST_PRINT(" [stream(%s)]\n", stream->stream_key);
    PrintVersion(&stream->stream_version);
    TEST_PRINT("  - destination id: %" PRId32 "\n", stream->destination_id);
    for (j = 0; j < stream->linkage_count; ++j) {
      TEST_PRINT(" --- linkage_version(%" PRIu32 "/%" PRIu32 "):\n",
          (j + 1), stream->linkage_count);
      PrintVersion(&stream->linkage_versions[j]);
    }
  }
}

static void PrintSensCordVersion(struct senscord_version_t* version) {
  uint32_t i = 0;
  TEST_PRINT(" [senscord]\n");
  PrintVersion(&version->senscord_version);
  TEST_PRINT(" [project]\n");
  PrintVersion(&version->project_version);
  PrintStreamVersion(version);
  for (i = 0; i < version->server_count; ++i) {
    struct senscord_version_t* server = &version->server_versions[i];
    TEST_PRINT("---[Server id: %" PRId32 " (%" PRIu32 "/%" PRIu32 ")]---\n",
        server->destination_id, (i + 1), version->server_count);
    PrintSensCordVersion(server);
  }
}
