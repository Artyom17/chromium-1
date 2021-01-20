// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_COMPOSITION_LAYER_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_COMPOSITION_LAYER_H_

#include "device/vr/public/mojom/xr_layers.mojom-blink.h"
#include "third_party/blink/renderer/modules/xr/xr_layer.h"
#include "third_party/blink/renderer/modules/xr/xr_view.h"

namespace viz {
class SingleReleaseCallback;
}

namespace blink {
class HTMLVideoElement;
class StaticBitmapImage;
class WebGLFramebuffer;
class WebGLRenderingContextBase;
class WebGLTexture;
class XRSession;
class XRSpace;
class XRViewport;

class XRCompositionLayer : public XRLayer {
  DEFINE_WRAPPERTYPEINFO();

 public:
  constexpr static const char* const kLayoutMono = "mono";
  constexpr static const char* const kLayoutStereo = "stereo";
  constexpr static const char* const kLayoutDefault = "default";
  constexpr static const char* const kLayoutStereoLeftRight =
      "stereo-left-right";
  constexpr static const char* const kLayoutStereoTopBottom =
      "stereo-top-bottom";

  XRCompositionLayer(XRSession*,
                     WebGLRenderingContextBase*,
                     WebGLTexture* color_texture,
                     WebGLTexture* depth_stencil_texture,
                     XRViewport* left_viewport,
                     XRViewport* right_viewport,
                     bool as_texture_array,
                     HTMLVideoElement* media = nullptr);
  XRCompositionLayer(XRSession*,
                     WebGLRenderingContextBase*,
                     WebGLTexture* left_color_texture,
                     WebGLTexture* right_color_texture,
                     WebGLTexture* left_depth_stencil_texture,
                     WebGLTexture* right_depth_stencil_texture,
                     XRViewport* left_viewport,
                     XRViewport* right_viewport);
  ~XRCompositionLayer() override = default;

  bool IsXRWebGLLayer() const override { return false; }
  XRWebGLLayer* GetAsXRWebGLLayer() override { return nullptr; }
  bool IsXRLayer() const override { return true; }
  XRCompositionLayer* GetAsXRLayer() override { return this; }
  virtual bool IsProjectionLayer() const { return false; }

  virtual const String layout() const;
  uint32_t viewPixelWidth() const;
  uint32_t viewPixelHeight() const;
  bool blendTextureSourceAlpha() const;
  void setBlendTextureSourceAlpha(bool value);
  bool chromaticAberrationCorrection() const;
  void setChromaticAberrationCorrection(bool value);

  void destroy();

  void OnFrameStart();
  void OnFrameEnd();

  bool stereo() const { return right_viewport_ != nullptr; }
  bool asTextureArray() const { return as_texture_array_; }

  void Trace(Visitor* visitor) const override;

  // XRLayer implementation
  bool CanDiscardDepthStencil() const override { return true; }
  void SetLayerIndex(unsigned index) override { index_ = index; }
  unsigned GetLayerIndex() const override { return index_; }
  ::device::mojom::blink::XRLayerUpdateInfoPtr GetLayerUpdateInfo()
      const override;
  device::Pose GetDevicePose() const override { return device::Pose(); }
  bool NeedUpdatePose() const override { return false; }
  void MarkContentInvalidated() override;
  bool needsRedraw() const override;

  WebGLTexture* getTexture(XRView* view) const;
  WebGLTexture* getDepthStencilTexture(XRView* view) const;
  XRViewport* getViewport(XRView* view) const;
  uint32_t getImageIndex(XRView* view) const;

  bool IsStatic() const;

  WebGLTexture* getTextureForEye(XRView::XREye eye) const;
  WebGLTexture* getDepthStencilTextureForEye(XRView::XREye eye) const;
  XRViewport* getViewportForEye(XRView::XREye eye) const;

  bool hasMediaElement() const { return media_; }
  void updateMedia() const;
  void onMediaChange();

 protected:
  ::device::mojom::blink::XRLayerPtr GetBasicLayerMojoObject(gfx::RectF* left_coords,
                                                             gfx::RectF* right_coords) const;
  bool IsValidXRSpace(XRSpace* space);
  virtual bool AllowViewerAndNonReferenceSpace() { return true; }
  constexpr static const char* const kBadSpaceMessage = "Invalid XRSpace.";

  Member<XRViewport> left_viewport_;
  Member<XRViewport> right_viewport_;

  Member<HTMLVideoElement> media_;

  // Texture should hold a swapchain
  HeapVector<Member<WebGLTexture>> color_textures_;
  HeapVector<Member<WebGLTexture>> depth_stencil_textures_;
  bool as_texture_array_;
  unsigned index_ = ~0u;
  bool blendTextureSourceAlpha_ = true;
  bool chromaticAberrationCorrection_ = true;
  bool needs_redraw_ = true;
  unsigned modification_counter_ = 0;

 private:
  virtual void updateLayerParams() {}
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_COMPOSITION_LAYER_H_
