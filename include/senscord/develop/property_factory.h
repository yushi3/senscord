/*
 * SPDX-FileCopyrightText: 2023 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef SENSCORD_DEVELOP_PROPERTY_FACTORY_H_
#define SENSCORD_DEVELOP_PROPERTY_FACTORY_H_

#include "senscord/config.h"

#ifndef SENSCORD_SERIALIZE

#include "senscord/noncopyable.h"

namespace senscord {

/**
 * @brief Property factory interface class.
 */
class PropertyFactoryBase : private util::Noncopyable {
 public:
  virtual ~PropertyFactoryBase() {}

  /**
   * @brief Clone the factory.
   * @return Cloned factory.
   */
  virtual PropertyFactoryBase* CloneFactory() const = 0;

  /**
   * @brief Create the property.
   * @return Created property.
   */
  virtual void* Create() const = 0;

  /**
   * @brief Delete the property.
   * @param[in] (property) Property to delete.
   */
  virtual void Delete(void* property) const = 0;

  /**
   * @brief Copy the property.
   * @param[in] (src) Source property.
   * @param[out] (dst) Destination property.
   */
  virtual void Copy(const void* src, void* dst) const = 0;
};

template<typename T>
class PropertyFactory : public PropertyFactoryBase {
 public:
  virtual PropertyFactoryBase* CloneFactory() const {
    return new PropertyFactory<T>;
  }

  virtual void* Create() const {
    return new T;
  }

  virtual void Delete(void* property) const {
    T* tmp = reinterpret_cast<T*>(property);
    delete tmp;
  }

  virtual void Copy(const void* src, void* dst) const {
    const T* tmp_src = reinterpret_cast<const T*>(src);
    T* tmp_dst = reinterpret_cast<T*>(dst);
    *tmp_dst = *tmp_src;
  }
};

}  // namespace senscord

#endif  // SENSCORD_SERIALIZE
#endif  // SENSCORD_DEVELOP_PROPERTY_FACTORY_H_
