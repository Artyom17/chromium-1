// (c) Facebook Technologies, LLC and its affiliates.
// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/xr/xr_layer.h"

#include "device/vr/public/mojom/xr_layers.mojom-blink.h"
#include "third_party/blink/renderer/core/dom/events/event_target.h"
#include "third_party/blink/renderer/modules/event_target_modules.h"
#include "third_party/blink/renderer/modules/xr/xr_session.h"
#include "third_party/blink/renderer/modules/xr/xr_system.h"
#include "third_party/blink/renderer/platform/bindings/script_wrappable.h"

namespace blink {

XRLayer::XRLayer(XRSession* session, WebGLRenderingContextBase* webgl_context)
    : session_(session),
      webgl_context_(webgl_context) {}

ExecutionContext* XRLayer::GetExecutionContext() const {
  return session_ ? session_->GetExecutionContext() : nullptr;
}

const AtomicString& XRLayer::InterfaceName() const {
  return event_target_names::kXRLayer;
}

void XRLayer::Trace(Visitor* visitor) const {
  visitor->Trace(session_);
  visitor->Trace(webgl_context_);
  visitor->Trace(xr_);
  ScriptWrappable::Trace(visitor);
}

}  // namespace blink
