// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_QUAD_LAYER_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_QUAD_LAYER_H_

#include "third_party/blink/renderer/modules/xr/xr_stereo_layout_layer_base.h"

namespace blink {

class ExceptionState;
class HTMLVideoElement;
class XRRigidTransform;
class XRSpace;

class XRQuadLayer : public XRStereoLayoutLayerBase {
  DEFINE_WRAPPERTYPEINFO();

 public:
  XRQuadLayer(XRSession*,
              XRSpace* space,
              WebGLRenderingContextBase*,
              WebGLTexture* color_texture,
              WebGLTexture* depth_stencil_texture,
              XRViewport* left_viewport,
              XRViewport* right_viewport,
              bool as_texture_array,
              HTMLVideoElement* media = nullptr);

  DEFINE_ATTRIBUTE_EVENT_LISTENER(redraw, kRedraw)

  float width() const;
  void setWidth(float width);
  float height() const;
  void setHeight(float height);

  ::device::mojom::blink::XRLayerPtr GetLayerMojoObject() const override;

  void Trace(Visitor* visitor) const override;

 private:
  void updateLayerParams() override;

  float width_ = 1.0f;   // in meters
  float height_ = 1.0f;  // in meters
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_QUAD_LAYER_H_
