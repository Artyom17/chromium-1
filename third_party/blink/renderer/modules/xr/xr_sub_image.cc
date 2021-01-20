// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

#include "third_party/blink/renderer/modules/xr/xr_sub_image.h"

#include "third_party/blink/renderer/modules/xr/xr_viewport.h"

namespace blink {

XRViewport* XRSubImage::viewport() const {
  return viewport_;
}

void XRSubImage::Trace(Visitor* visitor) const {
  visitor->Trace(viewport_);
  ScriptWrappable::Trace(visitor);
}

}  // namespace blink
