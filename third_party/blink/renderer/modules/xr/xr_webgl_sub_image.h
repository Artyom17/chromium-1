// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_WEBGL_SUB_IMAGE_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_WEBGL_SUB_IMAGE_H_

#include "third_party/blink/renderer/modules/xr/xr_sub_image.h"

namespace blink {
class WebGLTexture;
class XRViewport;

class XRWebGLSubImage final : public XRSubImage {
  DEFINE_WRAPPERTYPEINFO();

 public:
  XRWebGLSubImage() = default;

  void assign(WebGLTexture* colorTexture,
              WebGLTexture* depthStencilTexture,
              XRViewport* viewport,
              uint32_t image_index);

  ~XRWebGLSubImage() override = default;

  WebGLTexture* colorTexture() const;
  WebGLTexture* depthStencilTexture() const;
  uint32_t imageIndex() const;
  uint32_t textureWidth() const;
  uint32_t textureHeight() const;

  void Trace(Visitor*) const override;

 private:
  Member<WebGLTexture> colorTexture_;
  Member<WebGLTexture> depthStencilTexture_;
  uint32_t image_index_ = 0;
  uint32_t texture_width_ = 0;
  uint32_t texture_height_ = 0;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_WEBGL_SUB_IMAGE_H_
