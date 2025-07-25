/*
 * SPDX-FileCopyrightText: 2017-2018 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef LIB_CORE_UTIL_OBSERVER_H_
#define LIB_CORE_UTIL_OBSERVER_H_

#include <stdint.h>
#include <vector>

#include "senscord/status.h"
#include "util/mutex.h"

namespace senscord {
namespace util {

/**
 * @brief Observer interface class.
 */
class Observer {
 public:
  /**
   * @brief Abstruct Notify function.
   * @param[in] (param) Parameter to notify.
   * @return Status object.
   */
  virtual Status Notify(const void* param) = 0;

  /**
   * @brief Destructor.
   */
  virtual ~Observer() {}
};

/**
 * @brief Observed subject base definition class.
 */
class ObservedSubject {
 public:
  /**
   * @brief Add the observer.
   * @param[in] (observer) Observer to be added.
   * @return Status object.
   */
  virtual Status AddObserver(Observer* observer);

  /**
   * @brief Remove the observer.
   * @param[in] (observer) Observer to be removed.
   * @return Status object.
   */
  virtual Status RemoveObserver(Observer* observer);

  /**
   * @brief Notify to all observers.
   * @param[in] (param) Parameter to notify.
   * @return Status object.
   */
  virtual Status NotifyObservers(const void* param);

  /**
   * @brief Constructor.
   */
  ObservedSubject();

  /**
   * @brief Destructor.
   */
  virtual ~ObservedSubject();

 private:
  /**
   * Observer management information.
   */
  struct ObserverElement {
    bool enabled;         /**< active status for observer. */
    Observer* observer;   /**< observer address. */
  };
  typedef std::vector<ObserverElement> ObserverList;

  /**
   * Observer list.
   */
  ObserverList observer_list_;

  /**
   * Mutex lock for list access.
   */
  util::Mutex mutex_;
};

}   // namespace util
}   // namespace senscord
#endif  // LIB_CORE_UTIL_OBSERVER_H_
