// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_WEBGL_BINDING_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_WEBGL_BINDING_H_

#include "third_party/blink/renderer/modules/webgl/webgl2_rendering_context.h"
#include "third_party/blink/renderer/modules/webgl/webgl_rendering_context.h"
#include "third_party/blink/renderer/modules/xr/xr_layer_init.h"
#include "third_party/blink/renderer/modules/xr/xr_projection_layer_init.h"
#include "third_party/blink/renderer/modules/xr/xr_webgl_rendering_context.h"
#include "third_party/blink/renderer/platform/bindings/script_wrappable.h"

namespace blink {

class ExceptionState;
class WebGLRenderingContextBase;
class WebGLTexture;
class XRCompositionLayer;
class XRCubeLayer;
class XRCubeLayerInit;
class XRCylinderLayer;
class XRCylinderLayerInit;
class XREquirectLayer;
class XREquirectLayerInit;
class XRFrame;
class XRLightProbe;
class XRProjectionLayer;
class XRQuadLayer;
class XRQuadLayerInit;
class XRSession;
class XRView;
class XRViewport;
class XRWebGLSubImage;

class XRWebGLBinding final : public ScriptWrappable {
  DEFINE_WRAPPERTYPEINFO();

 public:
  XRWebGLBinding(XRSession*, WebGLRenderingContextBase*, bool webgl2);
  ~XRWebGLBinding() override = default;

  static XRWebGLBinding* Create(XRSession*,
                                const XRWebGLRenderingContext&,
                                ExceptionState&);

  XRSession* session() const { return session_; }

  WebGLTexture* getReflectionCubeMap(XRLightProbe*, ExceptionState&);
  WebGLTexture* getCameraImage(XRFrame*, XRView*);

  static bool IsValidXRSpace(XRSession* session,
                             XRSpace* space,
                             bool allow_viewer_and_non_reference_space);

  struct XRViewportPair {
    STACK_ALLOCATED();

   public:
    XRViewport* left_viewport;
    XRViewport* right_viewport;
  };

  static XRViewportPair createViewportPair(const IntSize& desired_size,
                                           int x_mult,
                                           int y_mult,
                                           bool as_texture_array = false);

  double nativeProjectionScaleFactor() const;

  XRProjectionLayer* createProjectionLayer(ScriptState* script_state,
                                           XRProjectionLayerInit* init,
                                           ExceptionState& exception_state);
  XRQuadLayer* createQuadLayer(ScriptState* script_state,
                               XRQuadLayerInit* init,
                               ExceptionState& exception_state,
                               HTMLVideoElement* media = nullptr);
  XRCylinderLayer* createCylinderLayer(ScriptState* script_state,
                                       XRCylinderLayerInit* init,
                                       ExceptionState& exception_state,
                                       HTMLVideoElement* media = nullptr);
  XREquirectLayer* createEquirectLayer(ScriptState* script_state,
                                       XREquirectLayerInit* init,
                                       ExceptionState& exception_state,
                                       HTMLVideoElement* media = nullptr);
  XRCubeLayer* createCubeLayer(ScriptState* script_state,
                               XRCubeLayerInit* init,
                               ExceptionState& exception_state);

  XRWebGLSubImage* getSubImage(XRCompositionLayer* layer,
                               XRFrame* frame,
                               const String& eye,
                               ExceptionState& exception_state);
  XRWebGLSubImage* getViewSubImage(XRCompositionLayer* layer,
                                   XRView* view,
                                   ExceptionState& exception_state);

  Vector<GLenum> supportedColorFormats() const;
  Vector<GLenum> supportedDepthStencilFormats() const;

  void Trace(Visitor*) const override;

 protected:
  struct TexturesInfo {
    STACK_ALLOCATED();

   public:
    WebGLTexture* color_texture;
    WebGLTexture* depth_stencil_texture;
    XRViewport* left_viewport;
    XRViewport* right_viewport;
    bool as_texture_array;
  };

  template <typename T> bool createTextures(TexturesInfo* out_info,
                                            ScriptState* script_state,
                                            T* initializer,
                                            bool is_static,
                                            ExceptionState& exception_state);

 protected:
  const Member<XRSession> session_;

  using LayerToSubImage =
      HeapHashMap<WeakMember<XRCompositionLayer>, WeakMember<XRWebGLSubImage>>;
  LayerToSubImage subimage_cache_;
  HeapHashMap<WeakMember<XRView>, Member<LayerToSubImage>> subimage_cache_per_view_;

  Member<WebGLRenderingContextBase> webgl_context_;

 private:
  bool webgl2_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_WEBGL_BINDING_H_
