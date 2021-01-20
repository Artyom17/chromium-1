// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

#include "third_party/blink/renderer/modules/xr/xr_stereo_layout_layer_base.h"

#include "third_party/blink/renderer/core/dom/dom_exception.h"
#include "third_party/blink/renderer/core/geometry/dom_point_read_only.h"
#include "third_party/blink/renderer/modules/xr/xr_reference_space.h"
#include "third_party/blink/renderer/modules/xr/xr_rigid_transform.h"
#include "third_party/blink/renderer/modules/xr/xr_viewport.h"
#include "third_party/blink/renderer/platform/bindings/exception_state.h"

namespace blink {

XRStereoLayoutLayerBase::XRStereoLayoutLayerBase(
    XRSession* session,
    XRSpace* space,
    WebGLRenderingContextBase* webgl_context,
    WebGLTexture* color_texture,
    WebGLTexture* depth_stencil_texture,
    XRViewport* left_viewport,
    XRViewport* right_viewport,
    bool as_texture_array,
    HTMLVideoElement* media)
    : XRCompositionLayer(session,
                         webgl_context,
                         color_texture,
                         depth_stencil_texture,
                         left_viewport,
                         right_viewport,
                         as_texture_array,
                         media),
      space_(space),
      transform_(
          MakeGarbageCollected<XRRigidTransform>(TransformationMatrix())) {}

XRStereoLayoutLayerBase::XRStereoLayoutLayerBase(
    XRSession* session,
    XRSpace* space,
    WebGLRenderingContextBase* webgl_context,
    WebGLTexture* left_color_texture,
    WebGLTexture* right_color_texture,
    WebGLTexture* left_depth_stencil_texture,
    WebGLTexture* right_depth_stencil_texture,
    XRViewport* left_viewport,
    XRViewport* right_viewport)
    : XRCompositionLayer(session,
                         webgl_context,
                         left_color_texture,
                         right_color_texture,
                         left_depth_stencil_texture,
                         right_depth_stencil_texture,
                         left_viewport,
                         right_viewport),
      space_(space),
      transform_(
          MakeGarbageCollected<XRRigidTransform>(TransformationMatrix())) {}

XRSpace* XRStereoLayoutLayerBase::space(ExceptionState&) const {
  return space_;
}

void XRStereoLayoutLayerBase::setSpace(XRSpace* space,
                                       ExceptionState& exception_state) {
  if (!IsValidXRSpace(space)) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      XRCompositionLayer::kBadSpaceMessage);
    return;
  }
  space_ = space;
}

XRRigidTransform* XRStereoLayoutLayerBase::transform() const {
  return transform_;
}

void XRStereoLayoutLayerBase::setTransform(XRRigidTransform* transform) {
  transform_ =
      MakeGarbageCollected<XRRigidTransform>(transform->TransformMatrix());
  DCHECK(transform_->IsValid());
}

device::Pose XRStereoLayoutLayerBase::GetDevicePose() const {
  XRRigidTransform* const offset_space_from_layer_transform = transform_;
  if (!offset_space_from_layer_transform) {
    LOG(ERROR) << __func__ << ": no transform";
    return device::Pose();
  }

  if (!space_) {
    LOG(ERROR) << __func__ << ": no space";
    return device::Pose();
  }
  DVLOG(3) << __func__ << ": orientation = x: "
            << offset_space_from_layer_transform->orientation()->x()
            << ", y: " << offset_space_from_layer_transform->orientation()->y()
            << ", z: " << offset_space_from_layer_transform->orientation()->z()
            << ", w: " << offset_space_from_layer_transform->orientation()->w();
  DVLOG(3) << __func__ << ": position = x: "
            << offset_space_from_layer_transform->position()->x()
            << ", y: " << offset_space_from_layer_transform->position()->y()
            << ", z: " << offset_space_from_layer_transform->position()->z()
            << ", w: " << offset_space_from_layer_transform->position()->w();
  DCHECK(offset_space_from_layer_transform->IsValid());

  // Transformation from passed in |space| to mojo space.
  auto mojo_from_native = space_->MojoFromNative();

  if (!mojo_from_native) {
    LOG(ERROR) << __func__ << ": no mojo_from_native";
    return device::Pose();
  }

  DVLOG(3) << __func__
            << ": mojo_from_native = " << mojo_from_native->ToString(true);

  // Transformation from passed in pose to |space|.
  const auto offset_space_from_layer =
      offset_space_from_layer_transform->TransformMatrix();
  const auto native_space_from_offset_space = space_->NativeFromOffsetMatrix();
  const auto native_space_from_layer =
      native_space_from_offset_space * offset_space_from_layer;

  const auto mojo_from_native_space = *mojo_from_native;
  const auto mojo_from_layer = mojo_from_native_space * native_space_from_layer;

  DVLOG(4) << __func__ << ": offset_space_from_layer = "
            << offset_space_from_layer.ToString(true);
  DVLOG(4) << __func__ << ": native_space_from_offset_space = "
            << native_space_from_offset_space.ToString(true);
  DVLOG(4) << __func__ << ": native_space_from_layer = "
            << native_space_from_layer.ToString(true);
  DVLOG(4) << __func__ << ": mojo_from_native_space = "
            << mojo_from_native_space.ToString(true);
  DVLOG(3) << __func__
            << ": mojo_from_layer = " << mojo_from_layer.ToString(true);

  TransformationMatrix::DecomposedType decomposed;
  if (!mojo_from_layer.Decompose(decomposed)) {
    LOG(ERROR) << __func__ << ": can't decompose, " << mojo_from_layer.ToString(true);
    return device::Pose();
  }

  // TODO(https://crbug.com/929841): Remove negation in quaternion once the bug
  // is fixed.
  device::Pose pose = device::Pose(
      blink::FloatPoint3D(decomposed.translate_x, decomposed.translate_y,
                          decomposed.translate_z),
      gfx::Quaternion(-decomposed.quaternion_x, -decomposed.quaternion_y,
                      -decomposed.quaternion_z, decomposed.quaternion_w));

  DVLOG(3) << __func__
           << ": poser.orientation = " << pose.orientation().ToString()
           << ", pose.position = " << pose.position().ToString();

  return pose;
}

::device::mojom::blink::XRLayerUpdateInfoPtr
XRStereoLayoutLayerBase::GetLayerUpdateInfo() const {
  DVLOG(2) << __func__;
  return XRCompositionLayer::GetLayerUpdateInfo();
}

void XRStereoLayoutLayerBase::Trace(Visitor* visitor) const {
  visitor->Trace(space_);
  visitor->Trace(transform_);
  XRCompositionLayer::Trace(visitor);
}

}  // namespace blink
