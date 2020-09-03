// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "include/condition_variable.h"

#include <windows.h>

namespace utils {

ConditionVariable::ConditionVariable(Lock* user_lock)
    : srwlock_(user_lock->lock_.native_handle())
{
  assert(user_lock);
  InitializeConditionVariable(reinterpret_cast<PCONDITION_VARIABLE>(&cv_));
}

ConditionVariable::~ConditionVariable() = default;

void ConditionVariable::Wait() {
  TimedWait(INFINITE);
}

void ConditionVariable::TimedWait(const uint32_t& max_time) {
  DWORD timeout = static_cast<DWORD>(max_time);

  if (!SleepConditionVariableSRW(reinterpret_cast<PCONDITION_VARIABLE>(&cv_),
                                 reinterpret_cast<PSRWLOCK>(srwlock_), timeout,
                                 0)) {
    // On failure, we only expect the CV to timeout. Any other error value means
    // that we've unexpectedly woken up.
    // Note that WAIT_TIMEOUT != ERROR_TIMEOUT. WAIT_TIMEOUT is used with the
    // WaitFor* family of functions as a direct return value. ERROR_TIMEOUT is
    // used with GetLastError().
    assert(static_cast<DWORD>(ERROR_TIMEOUT) == GetLastError());
  }
}

void ConditionVariable::Broadcast() {
  WakeAllConditionVariable(reinterpret_cast<PCONDITION_VARIABLE>(&cv_));
}

void ConditionVariable::Signal() {
  WakeConditionVariable(reinterpret_cast<PCONDITION_VARIABLE>(&cv_));
}

}  // namespace base
