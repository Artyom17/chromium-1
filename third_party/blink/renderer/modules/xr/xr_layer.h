// (c) Facebook Technologies, LLC and its affiliates.
// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_LAYER_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_LAYER_H_

#include "device/vr/public/mojom/xr_layers.mojom-blink.h"
#include "third_party/blink/renderer/core/dom/events/event_target.h"

namespace blink {
class WebGLRenderingContextBase;
class XRCompositionLayer;
class XRSession;
class XRSystem;
class XRWebGLLayer;


class XRLayer : public EventTargetWithInlineData {
  DEFINE_WRAPPERTYPEINFO();

 public:
  explicit XRLayer(XRSession*, WebGLRenderingContextBase*);
  ~XRLayer() override = default;

  XRSession* session() const { return session_; }

  // EventTarget overrides.
  ExecutionContext* GetExecutionContext() const override;
  const AtomicString& InterfaceName() const override;

  virtual bool IsXRWebGLLayer() const = 0;
  virtual XRWebGLLayer* GetAsXRWebGLLayer() = 0;
  virtual bool IsXRLayer() const = 0;
  virtual XRCompositionLayer* GetAsXRLayer() = 0;

  // Returns WebGL context for the layer
  WebGLRenderingContextBase* context() const { return webgl_context_; }

  // Returns true if depth / stencil buffers can be discarded.
  virtual bool CanDiscardDepthStencil() const = 0;

  // Returns a mojo object to use for sending layers for setup.
  virtual ::device::mojom::blink::XRLayerPtr GetLayerMojoObject() const = 0;

  // Returns mojo object for updating the layer during frame submission.
  virtual ::device::mojom::blink::XRLayerUpdateInfoPtr GetLayerUpdateInfo()
      const = 0;

  // Setter/getter for layer's index.
  virtual void SetLayerIndex(unsigned index) = 0;
  virtual unsigned GetLayerIndex() const = 0;

  // Returns pose in mojo (device) space.
  virtual device::Pose GetDevicePose() const = 0;

  // Returns true for layers which require pose updating for the frame (world
  // locked Quads & Cylinders, Cubes).
  virtual bool NeedUpdatePose() const = 0;

  // Mark content invalidated. May happen when swapchains got discarded.
  virtual void MarkContentInvalidated() = 0;

  // Returns true if the layer needs to be redrawn (i.e. if
  // getSubImage/getViewSubImage wasn't called after MarkContentInvalidated was
  // called.
  virtual bool needsRedraw() const = 0;

  void Trace(Visitor*) const override;

 private:
  const Member<XRSession> session_;
  const Member<XRSystem> xr_;
  Member<WebGLRenderingContextBase> webgl_context_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_LAYER_H_
