// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "include/scoped_com_initializer.h"
#include <assert.h>

namespace utils {

ScopedCOMInitializer::ScopedCOMInitializer() {
  Initialize(COINIT_APARTMENTTHREADED);
}

ScopedCOMInitializer::ScopedCOMInitializer(SelectMTA mta) {
  Initialize(COINIT_MULTITHREADED);
}

ScopedCOMInitializer::~ScopedCOMInitializer() {
  if (Succeeded())
    CoUninitialize();
}

bool ScopedCOMInitializer::Succeeded() const {
  return SUCCEEDED(hr_);
}

void ScopedCOMInitializer::Initialize(COINIT init) {
  // COINIT_DISABLE_OLE1DDE is always added based on:
  // https://docs.microsoft.com/en-us/windows/desktop/learnwin32/initializing-the-com-library
  hr_ = CoInitializeEx(nullptr, init | COINIT_DISABLE_OLE1DDE);
  assert((RPC_E_CHANGED_MODE != hr_) && "Invalid COM thread model change");
}

}  // namespace base
