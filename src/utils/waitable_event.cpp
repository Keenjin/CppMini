// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "include/waitable_event.h"

#include <windows.h>
#include <stddef.h>

#include <algorithm>
#include <utility>

namespace utils {

WaitableEvent::WaitableEvent(ResetPolicy reset_policy,
                             InitialState initial_state)
    : handle_(CreateEvent(nullptr,
                          reset_policy == ResetPolicy::MANUAL,
                          initial_state == InitialState::SIGNALED,
                          nullptr)) {
  // We're probably going to crash anyways if this is ever NULL, so we might as
  // well make our stack reports more informative by crashing here.
  assert(!handle_.Invalid());
}

WaitableEvent::WaitableEvent(utils::WinHandle handle)
    : handle_(std::move(handle)) {
  assert(!handle_.Invalid() && "Tried to create WaitableEvent from NULL handle");
}

WaitableEvent::~WaitableEvent() = default;

void WaitableEvent::Reset() {
  ResetEvent(handle_);
}

void WaitableEvent::Signal() {
  SetEvent(handle_);
}

bool WaitableEvent::IsSignaled() {
  DWORD result = WaitForSingleObject(handle_, 0);
  assert((result == WAIT_OBJECT_0 || result == WAIT_TIMEOUT) && "Unexpected WaitForSingleObject");
  return result == WAIT_OBJECT_0;
}

void WaitableEvent::Wait() {
  DWORD result = WaitForSingleObject(handle_, INFINITE);
  // It is most unexpected that this should ever fail.  Help consumers learn
  // about it if it should ever fail.
  assert(result != WAIT_FAILED);
  assert(WAIT_OBJECT_0 == result);
}

bool WaitableEvent::TimedWait(const uint32_t& wait_delta) {
  if (wait_delta <= 0)
    return IsSignaled();

    const DWORD result = WaitForSingleObject(handle_, wait_delta);
    assert((result == WAIT_OBJECT_0 || result == WAIT_TIMEOUT) && "Unexpected WaitForSingleObject");
    switch (result) {
      case WAIT_OBJECT_0:
        return true;
      case WAIT_TIMEOUT:
        // TimedWait can time out earlier than the specified |timeout| on
        // Windows. To make this consistent with the posix implementation we
        // should guarantee that TimedWait doesn't return earlier than the
        // specified |max_time| and wait again for the remaining time.
        break;
    }
  return false;
}

// static
size_t WaitableEvent::WaitMany(WaitableEvent** events, size_t count) {
  HANDLE handles[MAXIMUM_WAIT_OBJECTS];
  assert(count <= static_cast<size_t>(MAXIMUM_WAIT_OBJECTS));

  for (size_t i = 0; i < count; ++i)
    handles[i] = events[i]->handle();

  // The cast is safe because count is small - see the CHECK above.
  DWORD result =
      WaitForMultipleObjects(static_cast<DWORD>(count),
                             handles,
                             FALSE,      // don't wait for all the objects
                             INFINITE);  // no timeout
  if (result >= WAIT_OBJECT_0 + count) {
    return 0;
  }

  return result - WAIT_OBJECT_0;
}

}  // namespace base
