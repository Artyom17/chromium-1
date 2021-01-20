// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_SUB_IMAGE_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_SUB_IMAGE_H_

#include "third_party/blink/renderer/platform/bindings/script_wrappable.h"
#include "third_party/blink/renderer/platform/heap/member.h"

namespace blink {

class XRViewport;

class XRSubImage : public ScriptWrappable {
  DEFINE_WRAPPERTYPEINFO();

 public:
  XRSubImage() = default;
  ~XRSubImage() override = default;

  XRViewport* viewport() const;

  void Trace(Visitor*) const override;

 protected:
  Member<XRViewport> viewport_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_SUB_IMAGE_H_
