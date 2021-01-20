// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

#include "third_party/blink/renderer/modules/xr/xr_webgl_sub_image.h"

#include "third_party/blink/renderer/modules/webgl/webgl_rendering_context_base.h"
#include "third_party/blink/renderer/modules/webgl/webgl_texture.h"
#include "third_party/blink/renderer/modules/xr/xr_viewport.h"

namespace blink {

void XRWebGLSubImage::assign(WebGLTexture* colorTexture,
                             WebGLTexture* depthStencilTexture,
                             XRViewport* viewport,
                             uint32_t image_index) {
  viewport_ = viewport;
  image_index_ = image_index;
  colorTexture_ = colorTexture;
  depthStencilTexture_ = depthStencilTexture;
  
  NOTIMPLEMENTED();
  // TODO
  texture_width_ = 0;
  texture_height_ = 0;
}

WebGLTexture* XRWebGLSubImage::colorTexture() const {
  return colorTexture_;
}

WebGLTexture* XRWebGLSubImage::depthStencilTexture() const {
  return depthStencilTexture_;
}

uint32_t XRWebGLSubImage::imageIndex() const {
  return image_index_;
}

uint32_t XRWebGLSubImage::textureWidth() const {
  return texture_width_;
}

uint32_t XRWebGLSubImage::textureHeight() const {
  return texture_height_;
}

void XRWebGLSubImage::Trace(Visitor* visitor) const {
  visitor->Trace(colorTexture_);
  visitor->Trace(depthStencilTexture_);
  XRSubImage::Trace(visitor);
}

}  // namespace blink
