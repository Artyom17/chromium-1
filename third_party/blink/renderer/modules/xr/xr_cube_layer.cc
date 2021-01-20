// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

#include "third_party/blink/renderer/modules/xr/xr_cube_layer.h"

#include "device/vr/public/mojom/xr_layers.mojom-blink.h"
#include "third_party/blink/renderer/core/dom/dom_exception.h"
#include "third_party/blink/renderer/modules/xr/xr_reference_space.h"
#include "third_party/blink/renderer/modules/xr/xr_rigid_transform.h"
#include "third_party/blink/renderer/modules/xr/xr_utils.h"
#include "third_party/blink/renderer/platform/bindings/exception_state.h"
#include "third_party/blink/renderer/platform/transforms/transformation_matrix.h"

namespace blink {

XRCubeLayer::XRCubeLayer(XRSession* session,
                         XRSpace* space,
                         WebGLRenderingContextBase* webgl_context,
                         WebGLTexture* color_texture,
                         WebGLTexture* depth_stencil_texture,
                         XRViewport* left_viewport,
                         XRViewport* right_viewport)
    : XRStereoLayoutLayerBase(session,
                              space,
                              webgl_context,
                              color_texture,
                              depth_stencil_texture,
                              left_viewport,
                              right_viewport,
                              false /* as_texture_array */) {
  setOrientation(nullptr);
}

XRCubeLayer::XRCubeLayer(XRSession* session,
                         XRSpace* space,
                         WebGLRenderingContextBase* webgl_context,
                         WebGLTexture* left_color_texture,
                         WebGLTexture* right_color_texture,
                         WebGLTexture* left_depth_stencil_texture,
                         WebGLTexture* right_depth_stencil_texture,
                         XRViewport* left_viewport,
                         XRViewport* right_viewport)
    : XRStereoLayoutLayerBase(session,
                              space,
                              webgl_context,
                              left_color_texture,
                              right_color_texture,
                              left_depth_stencil_texture,
                              right_depth_stencil_texture,
                              left_viewport,
                              right_viewport) {
  setOrientation(nullptr);
}

const String XRCubeLayer::layout() const {
  if (left_viewport_ && right_viewport_) {
    return kLayoutStereo;
  } else {
    return kLayoutMono;
  }
}

DOMPointReadOnly* XRCubeLayer::orientation() const {
  return transform_->orientation();
}

void XRCubeLayer::setOrientation(const DOMPointReadOnly* orientation) {
  DVLOG(2) << __func__;
  constexpr const DOMPointReadOnly* position = nullptr;
  transform_ = XRRigidTransform::Create(position, orientation);
}

::device::mojom::blink::XRLayerPtr XRCubeLayer::GetLayerMojoObject() const {
  VLOG(1) << __func__;
  gfx::RectF left_coords;
  gfx::RectF right_coords;

  ::device::mojom::blink::XRLayerPtr xrlayer =
      GetBasicLayerMojoObject(&left_coords, &right_coords);
  if (xrlayer) {
    auto pose = GetDevicePose();

    ::device::mojom::blink::XRCubeLayerInfoPtr base =
        ::device::mojom::blink::XRCubeLayerInfo::New(std::move(pose),
                                                     left_coords, right_coords);
    xrlayer->data =
        ::device::mojom::blink::XRLayerUnion::NewCube(std::move(base));
  }
  return xrlayer;
}

void XRCubeLayer::Trace(Visitor* visitor) const {
  XRStereoLayoutLayerBase::Trace(visitor);
}

}  // namespace blink
