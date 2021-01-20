// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/xr/xr_hand.h"

#include "third_party/blink/renderer/bindings/modules/v8/v8_xr_hand_joint.h"
#include "third_party/blink/renderer/modules/xr/xr_joint_space.h"
#include "third_party/blink/renderer/modules/xr/xr_session.h"

namespace blink {

namespace {
// There are always 25 joints per hand
// see https://immersive-web.github.io/webxr-hand-input/#dom-xrhand-size
const wtf_size_t kNumJoints = 25;
}

class XRHandIterationSource final
    : public PairIterable<String, Member<XRJointSpace>>::IterationSource {
 public:
  explicit XRHandIterationSource(
      HeapVector<Member<XRJointSpace>> joints)
      : index_(0), joints_(joints) {}

  bool Next(ScriptState*,
            String& key,
            Member<XRJointSpace>& value,
            ExceptionState&) override {
    if (index_ >= joints_.size())
      return false;

    key = V8XRHandJoint(static_cast<V8XRHandJoint::Enum>(index_));
    value = joints_.at(index_++);
    return true;
  }

  void Trace(Visitor* visitor) const override {
    visitor->Trace(joints_);
    PairIterable<String, Member<XRJointSpace>>::IterationSource::Trace(visitor);
  }

 private:
  wtf_size_t index_;
  const HeapVector<Member<XRJointSpace>> joints_;
};

XRHand::XRHand(XRSession* session) : session_(session), joints_(kNumJoints) {}

XRJointSpace* XRHand::get(const String& jointName) const {
  const auto index = V8XRHandJoint::Create(jointName);
  CHECK(index != base::nullopt);

  return joints_[static_cast<unsigned int>(index->AsEnum())];
}

void XRHand::setJointSpace(unsigned index, XRJointSpace* joint_space) {
  if (index < size()) {
    joints_[index] = joint_space;
  }
}

void XRHand::updateFromJointsState(
    XRInputSource* input_source,
    const Vector<device::mojom::blink::XRJointSpacePtr>& hand_joints) {

  size_t index = 0;
  for (const auto& joint : hand_joints) {
    std::unique_ptr<TransformationMatrix> mojo_from_native =
        joint->is_tracked ? std::make_unique<TransformationMatrix>(
                                joint->mojo_from_native.matrix())
                          : nullptr;
    auto* joint_space = MakeGarbageCollected<XRJointSpace>(
        session_, input_source, std::move(mojo_from_native),
        V8XRHandJoint(static_cast<V8XRHandJoint::Enum>(index)),
        joint->radius);
    index++;
    setJointSpace(joint->joint_index, joint_space);
  }
}

XRHand::IterationSource*
XRHand::StartIteration(ScriptState* script_state, ExceptionState& exception_state) {
  return MakeGarbageCollected<XRHandIterationSource>(joints_);
}

void XRHand::Trace(Visitor* visitor) const {
  visitor->Trace(session_);
  visitor->Trace(joints_);
  ScriptWrappable::Trace(visitor);
}

}  // namespace blink
