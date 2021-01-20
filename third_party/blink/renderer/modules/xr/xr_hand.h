// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_HAND_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_HAND_H_

#include "base/macros.h"
#include "device/vr/public/mojom/vr_service.mojom-blink-forward.h"
#include "third_party/blink/renderer/bindings/core/v8/iterable.h"
#include "third_party/blink/renderer/platform/bindings/script_wrappable.h"
#include "third_party/blink/renderer/platform/wtf/text/wtf_string.h"
#include "third_party/blink/renderer/platform/heap/heap_allocator.h"

namespace blink {

class XRInputSource;
class XRJointSpace;
class XRSession;

class XRHand : public ScriptWrappable,
               public PairIterable<String, Member<XRJointSpace>> {
  DEFINE_WRAPPERTYPEINFO();

 public:
  XRHand(XRSession* session);
  ~XRHand() override = default;

  uint32_t size() const { return joints_.size(); }
  XRJointSpace* get(const String& jointName) const;

  void setJointSpace(unsigned index, XRJointSpace* joint_space);

  void updateFromJointsState(
      XRInputSource*,
      const Vector<device::mojom::blink::XRJointSpacePtr>& hand_joints);

  void Trace(Visitor*) const override;

 private:
  using Iterationsource =
      PairIterable<String, Member<XRJointSpace>>::IterationSource;
  IterationSource* StartIteration(ScriptState*, ExceptionState&) override;

  Member<XRSession> session_;
  HeapVector<Member<XRJointSpace>> joints_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_HAND_H_
