// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_CUBE_LAYER_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_CUBE_LAYER_H_

#include "third_party/blink/renderer/modules/xr/xr_stereo_layout_layer_base.h"

namespace blink {

class ExceptionState;
class DOMPointReadOnly;

class XRCubeLayer : public XRStereoLayoutLayerBase {
  DEFINE_WRAPPERTYPEINFO();

 public:
  XRCubeLayer(XRSession*,
              XRSpace* space,
              WebGLRenderingContextBase*,
              WebGLTexture* color_texture,
              WebGLTexture* depth_stencil_texture,
              XRViewport* left_viewport,
              XRViewport* right_viewport);
  XRCubeLayer(XRSession*,
              XRSpace* space,
              WebGLRenderingContextBase*,
              WebGLTexture* left_color_texture,
              WebGLTexture* right_color_texture,
              WebGLTexture* left_depth_stencil_texture,
              WebGLTexture* right_depth_stencil_texture,
              XRViewport* left_viewport,
              XRViewport* right_viewport);

  DEFINE_ATTRIBUTE_EVENT_LISTENER(redraw, kRedraw)

  const String layout() const override;

  DOMPointReadOnly* orientation() const;
  void setOrientation(const DOMPointReadOnly* orientation);

  ::device::mojom::blink::XRLayerPtr GetLayerMojoObject() const override;

 protected:
  bool AllowViewerAndNonReferenceSpace() override { return false; }

  void Trace(Visitor* visitor) const override;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_CUBE_LAYER_H_
