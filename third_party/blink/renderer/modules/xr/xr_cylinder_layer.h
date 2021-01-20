// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_CYLINDER_LAYER_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_CYLINDER_LAYER_H_

#include "third_party/blink/renderer/modules/xr/xr_stereo_layout_layer_base.h"

namespace blink {
class HTMLVideoElement;
class WebGLTexture;
class XRView;
class XRViewport;
class XRRigidTransform;
class XRSpace;

class XRCylinderLayer : public XRStereoLayoutLayerBase {
  DEFINE_WRAPPERTYPEINFO();

 public:
  XRCylinderLayer(XRSession*,
                  XRSpace* space,
                  WebGLRenderingContextBase*,
                  WebGLTexture* color_texture,
                  WebGLTexture* depth_stencil_texture,
                  XRViewport* left_viewport,
                  XRViewport* right_viewport,
                  bool as_texture_array,
                  HTMLVideoElement* media);

  DEFINE_ATTRIBUTE_EVENT_LISTENER(redraw, kRedraw)

  float radius() const;
  void setRadius(float radius);
  float centralAngle() const;
  void setCentralAngle(float angle);
  float aspectRatio() const;
  void setAspectRatio(float aspectRatio);

  ::device::mojom::blink::XRLayerPtr GetLayerMojoObject() const override;

  void Trace(Visitor* visitor) const override;

 private:
  float radius_ = 2.0f;
  float central_angle_ = 45.0f / 180.0f * M_PI;
  float aspect_ratio_ = 2.0f;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_CYLINDER_LAYER_H_
