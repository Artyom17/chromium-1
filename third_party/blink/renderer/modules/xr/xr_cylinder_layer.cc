// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

#include "third_party/blink/renderer/modules/xr/xr_cylinder_layer.h"

#include "third_party/blink/renderer/core/geometry/dom_point_read_only.h"
#include "third_party/blink/renderer/modules/webgl/webgl_framebuffer.h"
#include "third_party/blink/renderer/modules/webgl/webgl_rendering_context_base.h"
#include "third_party/blink/renderer/modules/webgl/webgl_texture.h"
#include "third_party/blink/renderer/modules/xr/xr_reference_space.h"
#include "third_party/blink/renderer/modules/xr/xr_rigid_transform.h"
#include "third_party/blink/renderer/modules/xr/xr_session.h"
#include "third_party/blink/renderer/modules/xr/xr_view.h"
#include "third_party/blink/renderer/modules/xr/xr_viewport.h"
#include "ui/gfx/geometry/point_f.h"

#include <math.h>

namespace blink {

XRCylinderLayer::XRCylinderLayer(XRSession* session,
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

float XRCylinderLayer::radius() const {
  return radius_;
}

void XRCylinderLayer::setRadius(float radius) {
  radius_ = radius;
  session()->NotifyLayersChanged();
}

float XRCylinderLayer::centralAngle() const {
  return central_angle_;
}

void XRCylinderLayer::setCentralAngle(float angle) {
  central_angle_ = angle;
  session()->NotifyLayersChanged();
}

float XRCylinderLayer::aspectRatio() const {
  return aspect_ratio_;
}

void XRCylinderLayer::setAspectRatio(float aspectRatio) {
  aspect_ratio_ = aspectRatio;
  session()->NotifyLayersChanged();
}

::device::mojom::blink::XRLayerPtr XRCylinderLayer::GetLayerMojoObject() const {
  VLOG(1) << __func__;
  gfx::RectF left_coords;
  gfx::RectF right_coords;

  ::device::mojom::blink::XRLayerPtr xrlayer =
      GetBasicLayerMojoObject(&left_coords, &right_coords);
  if (xrlayer) {
    auto pose = GetDevicePose();

    ::device::mojom::blink::XRCylinderLayerInfoPtr base =
        ::device::mojom::blink::XRCylinderLayerInfo::New(
            std::move(pose), left_coords, right_coords, radius_, central_angle_,
            aspect_ratio_);
    xrlayer->data =
        ::device::mojom::blink::XRLayerUnion::NewCylinder(std::move(base));
  }
  return xrlayer;
}

void XRCylinderLayer::Trace(Visitor* visitor) const {
  XRStereoLayoutLayerBase::Trace(visitor);
}

}  // namespace blink
