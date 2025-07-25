/*
 * SPDX-FileCopyrightText: 2022-2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_DEVELOP_EXTENSION_H_
#define SENSCORD_DEVELOP_EXTENSION_H_

#include <map>
#include <string>
#include <vector>

#include "senscord/config.h"
#include "senscord/noncopyable.h"
#include "senscord/status.h"
#include "senscord/stream.h"
#include "senscord/develop/property_accessor.h"
#include "senscord/develop/deserialized_property_accessor.h"
#include "senscord/develop/common_types.h"

/**
 * @brief Core extension registration macro.
 */
#define REGISTER_CORE_EXTENSION(class_name) \
  extern "C" void RegisterCoreExtension(void* library) { \
    senscord::ExtensionLibrary* extension = \
        reinterpret_cast<senscord::ExtensionLibrary*>(library); \
    extension->RegisterClass<senscord::CoreExtension, class_name>( \
        "CoreExtension"); \
  }

/**
 * @brief Stream extension registration macro.
 */
#define REGISTER_STREAM_EXTENSION(class_name) \
  extern "C" void RegisterStreamExtension(void* library) { \
    senscord::ExtensionLibrary* extension = \
        reinterpret_cast<senscord::ExtensionLibrary*>(library); \
    extension->RegisterClass<senscord::StreamExtension, class_name>( \
        "StreamExtension"); \
  }

namespace senscord {

// pre-define
class PropertyHistoryBook;
class FrameExtension;

/**
 * @brief Interface for core extension.
 */
class CoreExtension : private util::Noncopyable {
 public:
  /**
   * @brief Destructor.
   */
  virtual ~CoreExtension() {}

  /**
   * @brief Extension of Core::Init processing.
   *
   * Called only once during Core::Init.
   * Please implement the user initialization process.
   *
   * @return Status object.
   */
  virtual Status Init() {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseNotSupported,
        "not implemented");
  }

  /**
   * @brief Extension of Core::Init processing.
   *
   * Called only once during Core::Init.
   * Please implement the user initialization process.
   *
   * @param[in] (arguments) Extension arguments.
   * @return Status object.
   */
  virtual Status Init(const std::map<std::string, std::string>& arguments) {
    // Override if necessary.
    return Init();
  }

  /**
   * @brief Extension of Core::Exit processing.
   *
   * Called only once during Core::Exit.
   * Please implement the user termination process.
   *
   * @return Status object.
   */
  virtual Status Exit() = 0;
};


/* ------------------------------------------------------------------------- */

/**
 * @brief ChannelRawData for extension.
 */
typedef ChannelRawData ExtensionChannelRawData;

/**
 * @brief FrameInfo for extension.
 */
struct ExtensionFrameInfo {
  uint64_t sequence_number;     /**< Sequential number of frame */
  std::vector<ExtensionChannelRawData> channels;    /**< Channel data list. */
};

/**
 * @brief Type of frame extension.
 */
enum FrameExtensionType {
  /** Executed on the client side in multi-process. */
  kFrameExtensionNormal,
  /** Executed on the Server side in multi-process. */
  kFrameExtensionShared,
};

/**
 * @brief Frame extension adapter.
 */
class FrameExtensionAdapter : private util::Noncopyable {
 public:
  /**
   * @brief Constructor
   */
  FrameExtensionAdapter();

  /**
   * @brief Destructor
   */
  virtual ~FrameExtensionAdapter();

  /**
   * @brief Initialize of the this class.
   * @param[in] (frame_extension) FrameExtension class.
   * @param[in] (type) Type of frame extension.
   * @param[in] (channels) Channel information to extend.
   * @param[in] (arguments) Arguments for FrameExtension.
   * @param[in] (allocators) Allocators for FrameExtension.
   * @return Status object.
   */
  Status Init(
      FrameExtension* frame_extension,
      FrameExtensionType type,
      const std::map<uint32_t, ChannelInfo>& channels,
      const std::map<std::string, std::string>& arguments,
      const std::map<std::string, MemoryAllocator*>& allocators);

  /**
   * @brief Extension of Stream::GetFrame processing.
   * @param[in] (frame) Frame obtained by GetFrame
   * @param[out] (frameinfo) The information about extension frames.
   */
  void ExtendFrame(const Frame* frame, ExtensionFrameInfo* frameinfo);

  /**
   * @brief Extension of Stream::ReleaseFrame processing.
   * @param[in] (frameinfo) The information about used extension frames.
   */
  void ReleaseFrame(const ExtensionFrameInfo& frameinfo);

  /**
   * @brief Return whether FrameExtension is registered.
   * @return If FrameExtension is registered, return True.
   */
  bool IsRegistered() const {
    return (frame_extension_ != NULL);
  }

  /**
   * @brief Get the memory allocator by the name.
   * @param[in] (name) The memory allocator name.
   * @param[out] (allocator) The accessor of memory allocator.
   * @return Status object.
   */
  Status GetAllocator(
      const std::string& name, MemoryAllocator** allocator) const;

  /**
   * @brief Get frame extension type.
   * @return Frame extension type.
   */
  FrameExtensionType GetFrameExtensionType() const {
    return frame_extension_type_;
  }

  /**
   * @brief Get extension channel info.
   * @return Extension channel info.
   */
  std::map<uint32_t, ChannelInfo> GetChannelInfo() const {
    return channel_info_;
  }

  /**
   * @brief Get extension arguments.
   * @return Extension arguments.
   */
  Status GetArguments(std::map<std::string, std::string>* arguments) const {
    if (arguments == NULL) {
      return SENSCORD_STATUS_FAIL(kStatusBlockCore,
          Status::kCauseInvalidArgument, "arguments is NULL");
    }
    *arguments = arguments_;
    return Status::OK();
  }

#ifdef SENSCORD_SERIALIZE
  /**
   * @brief Update frame channel property.
   * @param[in] (channel_id) Target channel ID.
   * @param[in] (key) Property key to updated.
   * @param[in] (property) Property to updated.
   * @return Status object.
   */
  template<typename T>
  Status UpdateChannelProperty(
      uint32_t channel_id, const std::string& key, const T* property) {
    Status status;
    serialize::SerializedBuffer buffer;
    if (property != NULL) {
      serialize::Encoder encoder(&buffer);
      status = encoder.Push(*property);
      SENSCORD_STATUS_TRACE(status);
    }
    if (status.ok()) {
      status = SetUpdateChannelProperty(
          channel_id, key, buffer.data(), buffer.size());
      SENSCORD_STATUS_TRACE(status);
    }
    return status;
  }

  /**
   * @brief Set property to PropertyHistoryBook.
   * @param[in] (channel_id) Target channel ID.
   * @param[in] (key) Property key to updated.
   * @param[in] (property) Property to updated.
   * @param[in] (size) Size of property.
   * @return Status object.
   */
  Status SetUpdateChannelProperty(
      uint32_t channel_id, const std::string& key,
      const void* property, size_t size);
#else
  /**
   * @brief Update frame channel property.
   * @param[in] (channel_id) Target channel ID.
   * @param[in] (key) Property key to updated.
   * @param[in] (property) Property to updated.
   * @return Status object.
   */
  template<typename T>
  Status UpdateChannelProperty(
      uint32_t channel_id, const std::string& key, const T* property) {
    PropertyFactory<T> factory;
    Status status = SetUpdateChannelProperty(
        channel_id, key, property, factory);
    return SENSCORD_STATUS_TRACE(status);
  }

  /**
   * @brief Set property to PropertyHistoryBook.
   * @param[in] (channel_id) Target channel ID.
   * @param[in] (key) Property key to updated.
   * @param[in] (property) Property to updated.
   * @param[in] (factory) Property factory.
   * @return Status object.
   */
  Status SetUpdateChannelProperty(
      uint32_t channel_id, const std::string& key,
      const void* property, const PropertyFactoryBase& factory);
#endif

  /**
   * @brief Get PropertyHistoryBook.
   * @return PropertyHistoryBook object.
   */
  PropertyHistoryBook* GetPropertyHistoryBook() const {
    return history_book_;
  }

 private:
  FrameExtensionType frame_extension_type_;
  std::map<std::string, MemoryAllocator*> allocators_;
  std::map<uint32_t, ChannelInfo> channel_info_;
  std::map<std::string, std::string> arguments_;
  PropertyHistoryBook* history_book_;
  FrameExtension* frame_extension_;
};

class FrameExtension : private util::Noncopyable {
 public:
  /**
   * @Constructor
   */
  FrameExtension() : parent_() {}

  /**
   * @Destructor
   */
  virtual ~FrameExtension() {}

  /**
   * @brief Initializes frame extension.
   * @param[in] (parent) Parent object.
   */
  void Init(FrameExtensionAdapter* parent) {
    parent_ = parent;
  }

  /**
   * @brief Extension of Stream::GetFrame processing.
   *
   * Called every time Stream::GetFrame.
   * Only when the Frame can be acquired.
   * Please implement the frame extension process.
   *
   * @param[in] (frame) Frame obtained by GetFrame
   * @param[out] (frameinfo) The information about extension frames.
   */
  virtual void ExtendFrame(
      const Frame* frame, ExtensionFrameInfo* frameinfo) = 0;

  /**
   * @brief Extension of Stream::ReleaseFrame processing.
   *
   * Called every time Stream::ReleaseFrame.
   * Please implement the frame extension process.
   *
   * @param[in] (frameinfo) The information about used extension frames.
   */
  virtual void ReleaseFrame(const ExtensionFrameInfo& frameinfo) = 0;

 protected:
  /**
   * @brief Get the memory allocator by the name.
   * @param[in] (name) The memory allocator name.
   * @param[out] (allocator) The accessor of memory allocator.
   * @return Status object.
   */
  Status GetAllocator(const std::string& name, MemoryAllocator** allocator) {
    return SENSCORD_STATUS_TRACE(parent_->GetAllocator(name, allocator));
  }

  /**
   * @brief Get the memory allocator by the name.
   * @param[in] (name) The memory allocator name.
   * @param[out] (allocator) The accessor of memory allocator.
   * @return Status object.
   */
  Status GetArguments(std::map<std::string, std::string>* arguments) {
    return SENSCORD_STATUS_TRACE(parent_->GetArguments(arguments));
  }

  /**
   * @brief Update frame channel property.
   * @param[in] (channel_id) Target channel ID.
   * @param[in] (key) Property key to updated.
   * @param[in] (property) Property to updated.
   * @return Status object.
   */
  template<typename T>
  Status UpdateChannelProperty(
      uint32_t channel_id, const std::string& key, const T* property) {
    return SENSCORD_STATUS_TRACE(
        parent_->UpdateChannelProperty(channel_id, key, property));
  }

 private:
  FrameExtensionAdapter* parent_;
};

/**
 * @brief Interface for stream extension.
 */
class StreamExtension : private util::Noncopyable {
 public:
  /**
   * @brief Constructor.
   */
  StreamExtension();

  /**
   * @brief Destructor.
   */
  virtual ~StreamExtension();

  /**
   * @brief Initializes stream extension.
   * @param[in] (stream) stream to bind.
   * @param[in] (allocators) FrameExtension used in allocators.
   */
  void Init(
      Stream* stream,
      const std::map<std::string, MemoryAllocator*>& allocators,
      FrameExtensionAdapter* adapter);

  /**
   * @brief Extension of Core::OpenStream processing.
   *
   * Called every time Core::OpenStream.
   * Please implement the user open process.
   *
   * @return Status object.
   */
  virtual Status Open() {
    return SENSCORD_STATUS_FAIL(
        kStatusBlockCore, Status::kCauseNotSupported,
        "not implemented");
  }

  /**
   * @brief Extension of Core::OpenStream processing.
   *
   * Called every time Core::OpenStream.
   * Please implement the user open process.
   *
   * @param[in] (arguments) Extension arguments.
   * @return Status object.
   */
  virtual Status Open(const std::map<std::string, std::string>& arguments) {
    // // Override if necessary.
    return Open();
  }

  /**
   * @brief Extension of Core::CloseStream processing.
   *
   * Called every time Core::OpenStream.
   * Please implement the user close process.
   *
   * @return Status object.
   */
  virtual Status Close() = 0;

 protected:
  /**
   * @brief Returns the stream pointer.
   */
  Stream* GetStream() const;

  /**
   * @brief Type of stream property.
   */
  enum StreamPropertyType {
    /** Properties are assigned to each stream. */
    kNormal,
    /** Properties are shared by stream with the same key. */
    kShared,
  };

  /**
   * @brief Registers the property in the stream.
   * @param[in] (target) target class.
   * @param[in] (property_key) property key to register.
   * @param[in] (type) type of stream property.
   * @return Status object.
   */
  template<typename T, typename C>
  Status RegisterProperty(
      C* target, const std::string& property_key, StreamPropertyType type) {
    PropertyAccessor* accessor =
        new DeserializedPropertyAccessor<C, T>(property_key, target);
    Status status = RegisterPropertyAccessor(type, accessor);
    if (!status.ok()) {
      delete accessor;
    }
    return SENSCORD_STATUS_TRACE(status);
  }

  /**
   * @brief Registers the frame extension.
   * @param[in] (type) Type of frame extension.
   * @param[in] (channels) Channel information to extend.
   * @return Status object.
   */
  template<typename C>
  Status RegisterFrameExtension(
      FrameExtensionType type,
      const std::map<uint32_t, ChannelInfo>& channels) {
    std::map<std::string, std::string> empty_args;
    return RegisterFrameExtension<C>(type, channels, empty_args);
  }

  /**
   * @brief Registers the frame extension.
   * @param[in] (type) Type of frame extension.
   * @param[in] (channels) Channel information to extend.
   * @param[in] (arguments) Arguments for FrameExtension.
   * @return Status object.
   */
  template<typename C>
  Status RegisterFrameExtension(
      FrameExtensionType type,
      const std::map<uint32_t, ChannelInfo>& channels,
      const std::map<std::string, std::string>& arguments) {
    FrameExtension* impl = new C();
    Status status = adapter_->Init(
        impl, type, channels, arguments, allocators_);
    if (!status.ok()) {
      delete impl;
    }
    return SENSCORD_STATUS_TRACE(status);
  }

 private:
  /**
   * @brief Registers the property in the stream.
   * @param[in] (type) type of stream property.
   * @param[in] (accessor) property accessor to register.
   * @return Status object.
   */
  Status RegisterPropertyAccessor(
      StreamPropertyType type, PropertyAccessor* accessor);

 private:
  Stream* stream_;
  FrameExtensionAdapter* adapter_;
  std::map<std::string, MemoryAllocator*> allocators_;
};

/* ------------------------------------------------------------------------- */

/**
 * @brief Extension library object.
 *
 * Corresponds to one extension library.
 */
class ExtensionLibrary : private util::Noncopyable {
 public:
  /**
   * @brief Loads an extension library.
   * @param[in] (library_name) the name of the library to load.
   * @return an instance of extension library.
   */
  static ExtensionLibrary* Load(const std::string& library_name);

  /**
   * @brief Destructor.
   *
   * Unloads an extension library.
   */
  ~ExtensionLibrary();

  /**
   * @brief Returns the name of the library.
   */
  std::string GetLibraryName() const;

  /**
   * @brief Creates an instance of the specified class.
   * @param[in] (class_name) the name of the class to create.
   * @return an instance of the created class.
   */
  template<typename Base>
  Base* CreateInstance(const std::string& class_name) const {
    Base* instance = NULL;
    const Factory* factory = GetFactory(class_name);
    if (factory != NULL) {
      instance = static_cast<const FactoryBase<Base>*>(factory)->Create();
    }
    return instance;
  }

  /**
   * @brief Registers the class in the extension library.
   * @param[in] (class_name) the name of the class to register.
   */
  template<typename Base, typename Derived>
  void RegisterClass(const std::string& class_name) {
    Factory* factory = new FactoryDerived<Base, Derived>;
    RegisterFactory(class_name, factory);
  }

 private:
  ExtensionLibrary();

  /**
   * @brief A factory that creates an instance of a class.
   */
  struct Factory {
    virtual ~Factory() {}
  };
  template<typename Base>
  struct FactoryBase : public Factory {
    virtual Base* Create() const = 0;
  };
  template<typename Base, typename Derived>
  struct FactoryDerived : public FactoryBase<Base> {
    virtual Base* Create() const { return new Derived; }
  };

  /**
   * @brief Gets the factory.
   * @param[in] (class_name) the name of the class.
   * @return the factory object. (NULL if does not exist)
   */
  const Factory* GetFactory(const std::string& class_name) const;

  /**
   * @brief Registers the factory.
   * @param[in] (class_name) the name of the class to register.
   * @param[in] (factory) the factory to register.
   */
  void RegisterFactory(const std::string& class_name, Factory* factory);

 private:
  struct Impl;
  Impl* pimpl_;
};

}  // namespace senscord

#endif  // SENSCORD_DEVELOP_EXTENSION_H_
