// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "include/observer.h"

namespace utils {
namespace internal {

CheckedObserverAdapter::CheckedObserverAdapter(const CheckedObserver* observer)
    : weak_ptr_(observer->factory_.GetWeakPtr()) {}

CheckedObserverAdapter::CheckedObserverAdapter(CheckedObserverAdapter&& other) =
    default;
CheckedObserverAdapter& CheckedObserverAdapter::operator=(
    CheckedObserverAdapter&& other) = default;
CheckedObserverAdapter::~CheckedObserverAdapter() = default;

}  // namespace internal
}  // namespace base
