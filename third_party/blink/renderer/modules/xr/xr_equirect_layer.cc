// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

#include "third_party/blink/renderer/modules/xr/xr_equirect_layer.h"
#include "third_party/blink/renderer/modules/xr/xr_reference_space.h"
#include "third_party/blink/renderer/modules/xr/xr_rigid_transform.h"
#include "third_party/blink/renderer/modules/xr/xr_session.h"
#include "ui/gfx/geometry/point_f.h"

namespace blink {

XREquirectLayer::XREquirectLayer(XRSession* session,
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

float XREquirectLayer::radius() const {
  return radius_;
}

void XREquirectLayer::setRadius(float radius) {
  radius_ = std::max(0.0f, radius);
  session()->NotifyLayersChanged();
}

float XREquirectLayer::centralHorizontalAngle() const {
  return centralHorizontalAngle_;
}

void XREquirectLayer::setCentralHorizontalAngle(float angle) {
  centralHorizontalAngle_ = std::min(float(M_PI * 2.0f), std::max(0.0f, angle));
  session()->NotifyLayersChanged();
}

float XREquirectLayer::upperVerticalAngle() const {
  return upperVerticalAngle_;
}

void XREquirectLayer::setUpperVerticalAngle(float angle) {
  upperVerticalAngle_ = std::min(float(M_PI * 0.5f), std::max(float(-M_PI * 0.5f), angle));
  session()->NotifyLayersChanged();
}

float XREquirectLayer::lowerVerticalAngle() const {
  return lowerVerticalAngle_;
}

void XREquirectLayer::setLowerVerticalAngle(float angle) {
  lowerVerticalAngle_ = std::min(float(M_PI * 0.5f), std::max(float(-M_PI * 0.5f), angle));
  session()->NotifyLayersChanged();
}

::device::mojom::blink::XRLayerPtr XREquirectLayer::GetLayerMojoObject() const {
  VLOG(1) << __func__;
  gfx::RectF left_coords;
  gfx::RectF right_coords;

  ::device::mojom::blink::XRLayerPtr xrlayer =
      GetBasicLayerMojoObject(&left_coords, &right_coords);
  if (xrlayer) {
    auto pose = GetDevicePose();

    const float scaleX = (2.0f * M_PI) / centralHorizontalAngle_;
    const float scaleY =
        (std::fabs(upperVerticalAngle_ - lowerVerticalAngle_) >
         std::numeric_limits<float>::epsilon())
            ? M_PI / (upperVerticalAngle_ - lowerVerticalAngle_)
            : 0.0f;
    const float biasX = 0.5f - scaleX * 0.5f;
    // the image origin is in the upper left
    const float upperFrac = 0.5f - (upperVerticalAngle_ / M_PI);
    const float biasY = -scaleY * upperFrac;

    ::device::mojom::blink::XREquirectLayerInfoPtr base =
        ::device::mojom::blink::XREquirectLayerInfo::New(
            std::move(pose), left_coords, right_coords, gfx::PointF(scaleX, scaleY),
            gfx::PointF(biasX, biasY), radius_);
    xrlayer->data =
        ::device::mojom::blink::XRLayerUnion::NewEquirect(std::move(base));
  }
  return xrlayer;
}

void XREquirectLayer::Trace(Visitor* visitor) const {
  XRStereoLayoutLayerBase::Trace(visitor);
}

}  // namespace blink
