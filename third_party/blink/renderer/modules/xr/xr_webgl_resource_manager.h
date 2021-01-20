// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_WEBGL_RESOURCE_MANAGER_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_WEBGL_RESOURCE_MANAGER_H_

#include "third_party/blink/renderer/modules/webgl/webgl_rendering_context_base.h"

namespace blink {

class XRSession;

// This class is a bridge between WebGL and Platform swap chains. Platform
// graphics doesn't have access to WebGL objects like framebuffers or textures,
// therefore all creation of such objects should be done here.
class XRWebGLResourceManager final
    : public GarbageCollected<XRWebGLResourceManager> {
 public:
  XRWebGLResourceManager(XRSession* session);
  ~XRWebGLResourceManager();

  WebGLTexture* CreateTextureSwapChain(WebGLRenderingContextBase*,
                                       GLenum texture_target,
                                       const IntSize& desired_size,
                                       bool want_alpha_channel,
                                       bool want_foveation,
                                       bool can_discard_depth,
                                       bool is_static);
  WebGLFramebuffer* CreateFramebufferSwapChain(WebGLRenderingContextBase*,
                                               const IntSize& desired_size,
                                               bool want_alpha_channel,
                                               bool want_depth_buffer,
                                               bool want_stencil_buffer,
                                               bool want_antialiasing,
                                               bool want_foveation,
                                               bool can_discard_depth);
  void OnFrameStart();
  void OnFrameEnd();

  void BeginDestruction();
  void Trace(Visitor*) const;

  static WebGLTexture* CreateMatchedDepthStencilTexture(
      WebGLRenderingContextBase*,
      WebGLTexture* swap_chain_texture,
      bool include_stencil);

  bool NeedToUpdateLayers();

  void OnSwapChainsDiscarded();
  bool UpdateSwapChains();

 private:
  HeapHashSet<Member<WebGLObject>> objects_;
  Member<XRSession> session_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_WEBGL_RESOURCE_MANAGER_H_
