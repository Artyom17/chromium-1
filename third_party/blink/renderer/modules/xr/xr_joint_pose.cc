// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/xr/xr_joint_pose.h"
#include "third_party/blink/renderer/modules/xr/xr_pose.h"
#include "third_party/blink/renderer/modules/xr/xr_session.h"

namespace blink {

XRJointPose::XRJointPose(XRSession* session,
                         const TransformationMatrix& transform,
                         bool emulatedPosition,
                         float radius)
    : XRPose(transform, emulatedPosition), session_(session), radius_(radius) {}

void XRJointPose::Trace(blink::Visitor* visitor) const {
  visitor->Trace(session_);
  XRPose::Trace(visitor);
}

}  // namespace blink
