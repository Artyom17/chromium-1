// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_EQUIRECT_LAYER_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_EQUIRECT_LAYER_H_

#include "third_party/blink/renderer/modules/xr/xr_stereo_layout_layer_base.h"
#include "ui/gfx/geometry/point_f.h"

namespace blink {
class HTMLVideoElement;
class XRReferenceSpace;
class XRRigidTransform;

class XREquirectLayer : public XRStereoLayoutLayerBase {
  DEFINE_WRAPPERTYPEINFO();

 public:
  XREquirectLayer(XRSession*,
                  XRSpace* space,
                  WebGLRenderingContextBase*,
                  WebGLTexture* color_texture,
                  WebGLTexture* depth_stencil_texture,
                  XRViewport* left_viewport,
                  XRViewport* right_viewport,
                  bool as_texture_array,
                  HTMLVideoElement* media = nullptr);

  DEFINE_ATTRIBUTE_EVENT_LISTENER(redraw, kRedraw)

  float radius() const;
  void setRadius(float radius);
  float centralHorizontalAngle() const;
  void setCentralHorizontalAngle(float angle);
  float upperVerticalAngle() const;
  void setUpperVerticalAngle(float angle);
  float lowerVerticalAngle() const;
  void setLowerVerticalAngle(float angle);

  ::device::mojom::blink::XRLayerPtr GetLayerMojoObject() const override;

  void Trace(Visitor* visitor) const override;

 protected:
  bool AllowViewerAndNonReferenceSpace() override { return false; }
  float radius_ = 0.0f;
  float centralHorizontalAngle_ = M_PI * 2.0f;
  float upperVerticalAngle_ = M_PI / 2.0f;
  float lowerVerticalAngle_ = -M_PI / 2.0f;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_EQUIRECT_LAYER_H_
