// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_STEREO_LAYOUT_LAYER_BASE_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_STEREO_LAYOUT_LAYER_BASE_H_

#include "third_party/blink/renderer/modules/xr/xr_composition_layer.h"

namespace blink {

class ExceptionState;
class HTMLVideoElement;
class XRRigidTransform;
class XRSpace;

// A base class for all layers which support stereo layer layout.
// Projection layer is not one of them.
class XRStereoLayoutLayerBase : public XRCompositionLayer {
 public:
  XRStereoLayoutLayerBase(XRSession*,
                          XRSpace* space,
                          WebGLRenderingContextBase*,
                          WebGLTexture* color_texture,
                          WebGLTexture* depth_stencil_texture,
                          XRViewport* left_viewport,
                          XRViewport* right_viewport,
                          bool as_texture_array,
                          HTMLVideoElement* media = nullptr);
  XRStereoLayoutLayerBase(XRSession*,
                          XRSpace* space,
                          WebGLRenderingContextBase*,
                          WebGLTexture* left_color_texture,
                          WebGLTexture* right_color_texture,
                          WebGLTexture* left_depth_stencil_texture,
                          WebGLTexture* right_depth_stencil_texture,
                          XRViewport* left_viewport,
                          XRViewport* right_viewport);

  XRSpace* space(ExceptionState& exception_state) const;
  void setSpace(XRSpace* referenceSpace, ExceptionState& exception_state);
  XRRigidTransform* transform() const;
  void setTransform(XRRigidTransform* transform);

  ::device::mojom::blink::XRLayerUpdateInfoPtr GetLayerUpdateInfo()
      const override;
  // need to update the pose every frame
  bool NeedUpdatePose() const override { return true; }

  void Trace(Visitor* visitor) const override;

 protected:
  device::Pose GetDevicePose() const override;

 protected:
  Member<XRSpace> space_;
  Member<XRRigidTransform> transform_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_STEREO_LAYOUT_LAYER_BASE_H_
