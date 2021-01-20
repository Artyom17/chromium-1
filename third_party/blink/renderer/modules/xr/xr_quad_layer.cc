// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

#include "third_party/blink/renderer/modules/xr/xr_quad_layer.h"

#include "third_party/blink/renderer/core/dom/dom_exception.h"
#include "third_party/blink/renderer/core/html/media/html_video_element.h"
#include "third_party/blink/renderer/modules/xr/xr_reference_space.h"
#include "third_party/blink/renderer/modules/xr/xr_rigid_transform.h"
#include "third_party/blink/renderer/modules/xr/xr_session.h"
#include "third_party/blink/renderer/platform/bindings/exception_state.h"

namespace blink {

XRQuadLayer::XRQuadLayer(XRSession* session,
                         XRSpace* space,
                         WebGLRenderingContextBase* webgl_context,
                         WebGLTexture* color_texture,
                         WebGLTexture* depth_stencil_texture,
                         XRViewport* left_viewport,
                         XRViewport* right_viewport,
                         bool as_texture_array,
                         HTMLVideoElement* media)
    : XRStereoLayoutLayerBase(session,
                              space,
                              webgl_context,
                              color_texture,
                              depth_stencil_texture,
                              left_viewport,
                              right_viewport,
                              as_texture_array,
                              media) {}

float XRQuadLayer::width() const {
  return width_;
}

void XRQuadLayer::setWidth(float width) {
  width_ = width;
  session()->NotifyLayersChanged();
}

float XRQuadLayer::height() const {
  return height_;
}

void XRQuadLayer::setHeight(float height) {
  height_ = height;
  session()->NotifyLayersChanged();
}

::device::mojom::blink::XRLayerPtr XRQuadLayer::GetLayerMojoObject() const {
  VLOG(1) << __func__ << ": w = " << width_ << ", h = " << height_;
  gfx::RectF left_coords;
  gfx::RectF right_coords;

  ::device::mojom::blink::XRLayerPtr xrlayer =
      GetBasicLayerMojoObject(&left_coords, &right_coords);
  if (xrlayer) {
    auto pose = GetDevicePose();

    ::device::mojom::blink::XRQuadLayerInfoPtr base =
        ::device::mojom::blink::XRQuadLayerInfo::New(
            std::move(pose), left_coords, right_coords, width_, height_);
    xrlayer->data =
        ::device::mojom::blink::XRLayerUnion::NewQuad(std::move(base));
  }
  return xrlayer;
}

void XRQuadLayer::updateLayerParams() {
  setWidth(layout() == kLayoutStereoLeftRight ? media_->videoWidth() / 2
                                              : media_->videoWidth());
  setHeight(layout() == kLayoutStereoTopBottom ? media_->videoHeight() / 2
                                               : media_->videoHeight());
}

void XRQuadLayer::Trace(Visitor* visitor) const {
  XRStereoLayoutLayerBase::Trace(visitor);
}

}  // namespace blink
