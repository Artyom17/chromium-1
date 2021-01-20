// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_PROJECTION_LAYER_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_PROJECTION_LAYER_H_

#include "third_party/blink/renderer/modules/xr/xr_composition_layer.h"

namespace blink {
class WebGLTexture;
class XRViewport;

class XRProjectionLayer : public XRCompositionLayer {
  DEFINE_WRAPPERTYPEINFO();

 public:
  XRProjectionLayer(XRSession*,
                    WebGLRenderingContextBase*,
                    WebGLTexture* color_texture,
                    WebGLTexture* depth_stencil_texture,
                    XRViewport* left_viewport,
                    XRViewport* right_viewport,
                    bool as_texture_array);
  ~XRProjectionLayer() override = default;

  bool IsProjectionLayer() const override { return true; }

  bool ignoreDepthValues() const {
    return true;
  }

  void Trace(Visitor* visitor) const override;

  // XRLayer implementation
  bool CanDiscardDepthStencil() const override { return ignoreDepthValues(); }
  ::device::mojom::blink::XRLayerPtr GetLayerMojoObject() const override;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_PROJECTION_LAYER_H_
