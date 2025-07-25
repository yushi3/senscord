/*
 * SPDX-FileCopyrightText: 2017-2021 Sony Semiconductor Solutions Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "util/observer.h"

#include <vector>

#include "util/mutex.h"
#include "util/autolock.h"

namespace senscord {
namespace util {

/**
 * @brief Constructor.
 */
ObservedSubject::ObservedSubject() {
}

/**
 * @brief Destructor.
 */
ObservedSubject::~ObservedSubject() {
}

/**
 * @brief Add the observer.
 * @param[in] (observer) Observer to be added.
 * @return Status object.
 */
Status ObservedSubject::AddObserver(Observer* observer) {
  if (observer == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  ObserverElement elem;
  elem.enabled = true;
  elem.observer = observer;

  util::AutoLock lock(&mutex_);
  observer_list_.push_back(elem);
  return Status::OK();
}

/**
 * @brief Remove the observer.
 * @param[in] (observer) Observer to be removed.
 * @return Status object.
 */
Status ObservedSubject::RemoveObserver(Observer* observer) {
  if (observer == NULL) {
    return SENSCORD_STATUS_FAIL(kStatusBlockCore,
        Status::kCauseInvalidArgument, "invalid parameter");
  }
  util::AutoLock lock(&mutex_);
  ObserverList::iterator itr = observer_list_.begin();
  ObserverList::const_iterator end = observer_list_.end();
  for (; itr != end; ++itr) {
    if (((*itr).observer == observer) && (*itr).enabled) {
      (*itr).enabled = false;
      return Status::OK();
    }
  }
  return SENSCORD_STATUS_FAIL(kStatusBlockCore,
      Status::kCauseNotFound, "observer not found");
}

/**
 * @brief Notify to all observers.
 * @param[in] (param) Parameter to notify.
 * @return Status object.
 */
Status ObservedSubject::NotifyObservers(const void* param) {
  util::AutoLock lock(&mutex_);
  ObserverList::iterator itr = observer_list_.begin();
  while (itr != observer_list_.end()) {
    if ((*itr).enabled) {
      // notify to active notifier.
      // if error occured but no stopped.
      (*itr).observer->Notify(param);
      ++itr;
    } else {
      // remove non-active notifier.
      itr = observer_list_.erase(itr);
    }
  }
  return Status::OK();
}

}   // namespace util
}   // namespace senscord
