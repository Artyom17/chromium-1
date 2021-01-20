// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

#include "third_party/blink/renderer/modules/xr/xr_projection_layer.h"

#include "third_party/blink/renderer/modules/webgl/webgl_framebuffer.h"
#include "third_party/blink/renderer/modules/webgl/webgl_rendering_context_base.h"
#include "third_party/blink/renderer/modules/webgl/webgl_texture.h"
#include "third_party/blink/renderer/modules/xr/xr_view.h"
#include "third_party/blink/renderer/modules/xr/xr_viewport.h"

namespace blink {

XRProjectionLayer::XRProjectionLayer(XRSession* session,
                                     WebGLRenderingContextBase* webgl_context,
                                     WebGLTexture* color_texture,
                                     WebGLTexture* depth_stencil_texture,
                                     XRViewport* left_viewport,
                                     XRViewport* right_viewport,
                                     bool as_texture_array)
    : XRCompositionLayer(session,
                         webgl_context,
                         color_texture,
                         depth_stencil_texture,
                         left_viewport,
                         right_viewport,
                         as_texture_array) {}

::device::mojom::blink::XRLayerPtr XRProjectionLayer::GetLayerMojoObject()
    const {
  VLOG(1) << __func__;
  gfx::RectF left_coords;
  gfx::RectF right_coords;

  ::device::mojom::blink::XRLayerPtr xrlayer =
      GetBasicLayerMojoObject(&left_coords, &right_coords);
  if (xrlayer) {
    ::device::mojom::blink::XRBaseLayerInfoPtr base =
        ::device::mojom::blink::XRBaseLayerInfo::New(left_coords, right_coords);
    xrlayer->data =
        ::device::mojom::blink::XRLayerUnion::NewBase(std::move(base));
  }
  return xrlayer;
}

void XRProjectionLayer::Trace(Visitor* visitor) const {
  XRCompositionLayer::Trace(visitor);
}

}  // namespace blink
