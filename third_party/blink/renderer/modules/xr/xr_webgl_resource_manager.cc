// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

#include "third_party/blink/renderer/modules/xr/xr_webgl_resource_manager.h"
#include "third_party/blink/renderer/modules/webgl/webgl2_rendering_context.h"
#include "third_party/blink/renderer/modules/webgl/webgl_framebuffer.h"
#include "third_party/blink/renderer/modules/webgl/webgl_rendering_context.h"
#include "third_party/blink/renderer/modules/webgl/webgl_texture.h"
#include "third_party/blink/renderer/modules/xr/xr_session.h"

namespace blink {

XRWebGLResourceManager::XRWebGLResourceManager(XRSession* session)
    : session_(session) {
  DVLOG(1) << __func__ << " this = " << this;
}

XRWebGLResourceManager::~XRWebGLResourceManager() {
  DVLOG(1) << __func__ << " this = " << this;
}

void XRWebGLResourceManager::BeginDestruction() {
  DVLOG(1) << __func__ << " this = " << this;
  for (WebGLObject* object : objects_) {
    object->DeleteObject(nullptr);
  }
  objects_.clear();
  session_ = nullptr;
}

WebGLFramebuffer* XRWebGLResourceManager::CreateFramebufferSwapChain(
    WebGLRenderingContextBase* webgl_context,
    const IntSize& desired_size,
    bool want_alpha_channel,
    bool want_depth_buffer,
    bool want_stencil_buffer,
    bool want_antialiasing,
    bool want_foveation,
    bool can_discard_depth) {
  VLOG(1) << __func__ << "size = " << desired_size << ", "
          << "want_alpha_channel = " << int(want_alpha_channel) << ", "
          << "want_depth_buffer = " << int(want_depth_buffer) << ", "
          << "want_stencil_buffer = " << int(want_stencil_buffer) << ", "
          << "want_antialiasing = " << int(want_antialiasing) << ", "
          << "want_foveation = " << int(want_foveation) << ", "
          << "can_discard_depth = " << int(can_discard_depth);

  // Create an opaque WebGL Framebuffer
  WebGLFramebuffer* framebuffer =
      WebGLFramebuffer::CreateOpaque(webgl_context, want_stencil_buffer);

  // TODO
  NOTIMPLEMENTED();

  objects_.insert(framebuffer);
  return framebuffer;
}

WebGLTexture* XRWebGLResourceManager::CreateTextureSwapChain(
    WebGLRenderingContextBase* webgl_context,
    GLenum texture_target,
    const IntSize& desired_size,
    bool want_alpha_channel,
    bool want_foveation,
    bool can_discard_depth,
    bool is_static) {
  VLOG(1) << __func__
          << ((texture_target == GL_TEXTURE_2D_ARRAY)
                  ? " texture array"
                  : ((texture_target == GL_TEXTURE_CUBE_MAP)
                         ? " cube_map_texture"
                         : " texture"))
          << ", size = " << desired_size
          << ", want_alpha_channel = " << int(want_alpha_channel)
          << ", want_foveation = " << int(want_foveation)
          << ", can_discard_depth = " << int(can_discard_depth)
          << ", is_static = " << int(is_static);

  WebGLTexture* texture =
      MakeGarbageCollected<WebGLTexture>(webgl_context);

  // TODO
  NOTIMPLEMENTED();

  objects_.insert(texture);
  return texture;
}

// static
WebGLTexture* XRWebGLResourceManager::CreateMatchedDepthStencilTexture(
    WebGLRenderingContextBase* webgl_context,
    WebGLTexture* swap_chain_texture,
    bool include_stencil) {
  DCHECK(webgl_context);
  DCHECK(swap_chain_texture);

  WebGLTexture* texture = webgl_context->createTexture();
  
  // TODO
  NOTIMPLEMENTED();

  return texture;
}

void XRWebGLResourceManager::OnFrameStart() {
  DVLOG(2) << __func__;
  // TODO
  NOTIMPLEMENTED();
}

void XRWebGLResourceManager::OnFrameEnd() {
  DVLOG(2) << __func__;
  // TODO
  NOTIMPLEMENTED();
}

bool XRWebGLResourceManager::NeedToUpdateLayers() {
  // TODO
  NOTIMPLEMENTED();
  return false;
}

void XRWebGLResourceManager::OnSwapChainsDiscarded() {
  VLOG(1) << __func__;
  // TODO
  NOTIMPLEMENTED();
}

bool XRWebGLResourceManager::UpdateSwapChains() {
  VLOG(1) << __func__;
  // TODO
  NOTIMPLEMENTED();
  return false;
}

void XRWebGLResourceManager::Trace(Visitor* visitor) const {
  visitor->Trace(objects_);
  visitor->Trace(session_);
}

}  // namespace blink
