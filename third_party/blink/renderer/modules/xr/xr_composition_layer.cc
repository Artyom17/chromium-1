// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

#include "third_party/blink/renderer/modules/xr/xr_composition_layer.h"

#include "third_party/blink/renderer/modules/xr/xr_session.h"
#include "third_party/blink/renderer/modules/xr/xr_view.h"
#include "third_party/blink/renderer/modules/xr/xr_viewport.h"

#include "third_party/blink/renderer/core/dom/events/native_event_listener.h"
#include "third_party/blink/renderer/core/html/media/html_video_element.h"
#include "third_party/blink/renderer/core/imagebitmap/image_bitmap.h"

#include "third_party/blink/renderer/modules/webgl/webgl2_rendering_context.h"

#include "third_party/blink/renderer/modules/xr/xr_space.h"
#include "third_party/blink/renderer/modules/xr/xr_webgl_binding.h"
#include "third_party/blink/renderer/platform/graphics/graphics_types_3d.h"

namespace blink {

namespace {

size_t viewEyeToIndex(XRView::XREye eye) {
  switch (eye) {
    case XRView::kEyeNone:
    case XRView::kEyeLeft:
      return 0;

    case XRView::kEyeRight:
      return 1;

    default:
      NOTREACHED();
  }
}

class MediaChangeListener final : public NativeEventListener {
 public:
  explicit MediaChangeListener(XRCompositionLayer* layer) : layer_(layer) {}

  void Invoke(ExecutionContext*, Event*) override { layer_->onMediaChange(); }

  void Trace(Visitor* visitor) const override {
    visitor->Trace(layer_);
    NativeEventListener::Trace(visitor);
  }

 private:
  Member<XRCompositionLayer> layer_;
};

}  // namespace

XRCompositionLayer::XRCompositionLayer(XRSession* session,
                                       WebGLRenderingContextBase* webgl_context,
                                       WebGLTexture* color_texture,
                                       WebGLTexture* depth_stencil_texture,
                                       XRViewport* left_viewport,
                                       XRViewport* right_viewport,
                                       bool as_texture_array,
                                       HTMLVideoElement* media)
    : XRLayer(session, webgl_context),
      left_viewport_(left_viewport),
      right_viewport_(right_viewport),
      media_(media),
      as_texture_array_(as_texture_array) {
  DCHECK(color_texture);
  color_textures_.push_back(color_texture);
  if (depth_stencil_texture) {
    depth_stencil_textures_.push_back(depth_stencil_texture);
  }

  blendTextureSourceAlpha_ = false; //TODO

  if (media) {
    MediaChangeListener* listener =
        MakeGarbageCollected<MediaChangeListener>(this);
    media->addEventListener(event_type_names::kLoadedmetadata, listener);
  }
}

XRCompositionLayer::XRCompositionLayer(
    XRSession* session,
    WebGLRenderingContextBase* webgl_context,
    WebGLTexture* left_color_texture,
    WebGLTexture* right_color_texture,
    WebGLTexture* left_depth_stencil_texture,
    WebGLTexture* right_depth_stencil_texture,
    XRViewport* left_viewport,
    XRViewport* right_viewport)
    : XRLayer(session, webgl_context),
      left_viewport_(left_viewport),
      right_viewport_(right_viewport),
      as_texture_array_(false) {
  DCHECK(left_color_texture);
  color_textures_.push_back(left_color_texture);
  if (right_color_texture && right_color_texture != left_color_texture) {
    color_textures_.push_back(right_color_texture);
  }
  DCHECK((!left_depth_stencil_texture && !right_depth_stencil_texture) ||
         (right_depth_stencil_texture != left_depth_stencil_texture));
  if (left_depth_stencil_texture) {
    depth_stencil_textures_.push_back(left_depth_stencil_texture);
    if (right_depth_stencil_texture &&
        right_depth_stencil_texture != left_depth_stencil_texture) {
      DCHECK(right_color_texture);
      depth_stencil_textures_.push_back(right_depth_stencil_texture);
    }
  }

  blendTextureSourceAlpha_ = false; // TODO
}

const String XRCompositionLayer::layout() const {
  if (left_viewport_ && right_viewport_) {
    if (asTextureArray()) {
      if (IsProjectionLayer()) {
        return kLayoutDefault;
      } else {
        return kLayoutStereo;
      }
    } else if (right_viewport_->x() != 0.0) {
      return kLayoutStereoLeftRight;
    } else if (right_viewport_->y() != 0.0) {
      return kLayoutStereoTopBottom;
    } else {
      DCHECK(0) << __func__ << ": unsupported layout";
    }
  }
  return kLayoutMono;
}

uint32_t XRCompositionLayer::viewPixelWidth() const {
  // TODO
  return 0;
}

uint32_t XRCompositionLayer::viewPixelHeight() const {
  // TODO
  return 0;
}

bool XRCompositionLayer::blendTextureSourceAlpha() const {
  return blendTextureSourceAlpha_;
}

void XRCompositionLayer::setBlendTextureSourceAlpha(bool value) {
  // TODO
  blendTextureSourceAlpha_ = value;
}

bool XRCompositionLayer::chromaticAberrationCorrection() const {
  return chromaticAberrationCorrection_;
}

void XRCompositionLayer::setChromaticAberrationCorrection(bool value) {
  chromaticAberrationCorrection_ = value;
}

void XRCompositionLayer::destroy() {
  color_textures_.clear();
  depth_stencil_textures_.clear();
}

void XRCompositionLayer::OnFrameStart() {
  DVLOG(2) << __func__ << ": " << this << ", mod_cnt = " << modification_counter_;

  for (auto texture : color_textures_) {
    // TODO, call OnFrameStart on texture if necessary
  }
  if (hasMediaElement()) {
    updateMedia();
  }
}

void XRCompositionLayer::OnFrameEnd() {
  if (session()->ended()) {
    return;
  }
  DVLOG(2) << __func__ << ": " << this << ", mod_cnt = " << modification_counter_;

  if (context()->isContextLost()) {
    LOG(ERROR) << __func__ << ": context is lost";
    return;
  }

  needs_redraw_ = false;
  for (auto texture : color_textures_) {
    // TODO, call OnFrameEnd on texture if necessary
  }
  // Check if the layer is not pending
  if (GetLayerIndex() != ~0u && !media_) {
    session()->AddLayerToSubmission(this);
  }
  ++modification_counter_;
}

::device::mojom::blink::XRLayerPtr XRCompositionLayer::GetBasicLayerMojoObject(
    gfx::RectF* left_coords,
    gfx::RectF* right_coords) const {
  #if 1
  NOTIMPLEMENTED();
  return nullptr;
  #else
  XRSwapChain* const left_swap_chain = GetSwapChain(XRView::kEyeLeft);
  XRSwapChain* const right_swap_chain = GetSwapChain(XRView::kEyeRight);
  if (left_swap_chain && right_swap_chain) {
    const XRViewport* const left = left_viewport_;
    const XRViewport* const right = right_viewport_;
    const float width = left_swap_chain->GetInfo().size_.Width();
    const float height = left_swap_chain->GetInfo().size_.Height();
    DCHECK(width == right_swap_chain->GetInfo().size_.Width());
    DCHECK(height == right_swap_chain->GetInfo().size_.Height());

    *left_coords =
        left ? gfx::RectF(static_cast<float>(left->x()) / width,
                          static_cast<float>(left->y()) / height,
                          static_cast<float>(left->width()) / width,
                          static_cast<float>(left->height()) / height)
             : gfx::RectF();
    *right_coords =
        right ? gfx::RectF(static_cast<float>(right->x()) / width,
                             static_cast<float>(right->y()) / height,
                             static_cast<float>(right->width()) / width,
                             static_cast<float>(right->height()) / height)
              // if no right viewport - use the same as left one
              : *left_coords;

    ::device::mojom::blink::XRLayerPtr xrlayer =
        ::device::mojom::blink::XRLayer::New();
    xrlayer->header = ::device::mojom::blink::XRLayerHeaderInfo::New();
    xrlayer->header->image_source_handles.push_back(
        left_swap_chain->GetImageSourceHandle());
    if (left_swap_chain != right_swap_chain) {
      xrlayer->header->image_source_handles.push_back(
          right_swap_chain->GetImageSourceHandle());
    }
    xrlayer->header->eye_visibility =
        (right && left_swap_chain == right_swap_chain)
            ? ::device::mojom::blink::XRLayerEyeVisibility::SIDE_BY_SIDE_STEREO
            : ::device::mojom::blink::XRLayerEyeVisibility::BOTH;
    xrlayer->header->chromatic_aberration_correction =
        chromaticAberrationCorrection();
    xrlayer->header->blend_texture_source_alpha = blendTextureSourceAlpha();

    return xrlayer;
  } else {
    return nullptr;
  }
  #endif
}

bool XRCompositionLayer::IsValidXRSpace(XRSpace* space) {
  return XRWebGLBinding::IsValidXRSpace(session(), space,
                                        AllowViewerAndNonReferenceSpace());
}

::device::mojom::blink::XRLayerUpdateInfoPtr
XRCompositionLayer::GetLayerUpdateInfo() const {
  VLOG(1) << __func__;
  #if 1
  NOTIMPLEMENTED();
  return nullptr;
  #else
  XRSwapChain* const swap_chain = GetSwapChain(XRView::kEyeNone);
  if (swap_chain) {
#if DCHECK_IS_ON()
    // all swapchains must have the same indices
    for (auto texture : color_textures_) {
      DCHECK(texture->GetSwapChain());
      DCHECK(swap_chain->GetSwapChainIndex() ==
             texture->GetSwapChain()->GetSwapChainIndex());
    }
#endif
    device::mojom::blink::XRLayerUpdateInfoPtr update_info =
        device::mojom::blink::XRLayerUpdateInfo::New();
    update_info->layer_index = index_;
    update_info->swap_chain_index = swap_chain->GetSwapChainIndex();
    return update_info;
  } else {
    return nullptr;
  }
  #endif
}

XRViewport* XRCompositionLayer::getViewport(XRView* view) const {
  if (!view) {
    if (!stereo()) {
      return left_viewport_;
    } else {
      // TODO(AB): do we need to return combined viewports for stereo when view
      // is not specified?
      const double x1 = std::min(left_viewport_->x(), right_viewport_->x());
      const double y1 = std::min(left_viewport_->y(), right_viewport_->y());
      const double x2 =
          std::max(left_viewport_->x() + left_viewport_->width(),
                   right_viewport_->x() + right_viewport_->width());
      const double y2 =
          std::max(left_viewport_->y() + left_viewport_->height(),
                   right_viewport_->y() + right_viewport_->height());
      return MakeGarbageCollected<XRViewport>(x1, y1, x2 - x1, y2 - y1);
    }
  }

  return getViewportForEye(view->EyeValue());
}

uint32_t XRCompositionLayer::getImageIndex(XRView* view) const {
  if (!view)
    return 0;

  return viewEyeToIndex(view->EyeValue());
}

void XRCompositionLayer::MarkContentInvalidated() {
  needs_redraw_ = true;
}

bool XRCompositionLayer::needsRedraw() const {
  return needs_redraw_ && !media_;
}

WebGLTexture* XRCompositionLayer::getTextureForEye(XRView::XREye eye) const {
  if (color_textures_.size() == 1) {
    return color_textures_[0];
  } else if (color_textures_.size() > 1) {
    const size_t i = viewEyeToIndex(eye);
    DCHECK(i < color_textures_.size());
    return color_textures_[i];
  }
  return nullptr;
}

WebGLTexture* XRCompositionLayer::getDepthStencilTextureForEye(
    XRView::XREye eye) const {
  if (depth_stencil_textures_.size() == 1) {
    return depth_stencil_textures_[0];
  } else if (depth_stencil_textures_.size() > 1) {
    const size_t i = viewEyeToIndex(eye);
    if (i < depth_stencil_textures_.size()) {
      return depth_stencil_textures_[i];
    }
  }
  return nullptr;
}

XRViewport* XRCompositionLayer::getViewportForEye(XRView::XREye eye) const {
  if (eye == XRView::kEyeRight)
    return right_viewport_;

  // This code path also handles an eye of "none".
  return left_viewport_;
}

WebGLTexture* XRCompositionLayer::getTexture(XRView* view) const {
  return getTextureForEye((view) ? view->EyeValue() : XRView::kEyeNone);
}

WebGLTexture* XRCompositionLayer::getDepthStencilTexture(XRView* view) const {
  return getDepthStencilTextureForEye((view) ? view->EyeValue()
                                             : XRView::kEyeNone);
}

bool XRCompositionLayer::IsStatic() const {
  NOTIMPLEMENTED();
  return false;
}

void XRCompositionLayer::updateMedia() const {
  DCHECK(media_);

  if (!media_->HasAvailableVideoFrame()) {
    return;
  }

  auto* gl = context();

  ExceptionState exception_state(nullptr, ExceptionState::kConstructionContext,
                                 nullptr);

  gl->bindTexture(GL_TEXTURE_2D, getTextureForEye(XRView::kEyeNone));
  gl->pixelStorei(GC3D_UNPACK_FLIP_Y_WEBGL, true);
  gl->texSubImage2D(session()->GetExecutionContext(), GL_TEXTURE_2D, 0, 0, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, media_, exception_state);

  exception_state.ClearException();
}

void XRCompositionLayer::onMediaChange() {
  if (session()->ended() || !session()->immersive()) {
    return;
  }
  DCHECK(media_);
  LOG(INFO) << __func__ << ": w = " << media_->videoWidth() << ", h = " << media_->videoHeight();

  int x_mult = layout() == kLayoutStereoLeftRight ? 2 : 1;
  int y_mult = layout() == kLayoutStereoTopBottom ? 2 : 1;
  IntSize new_size(media_->videoWidth() * x_mult,
                   media_->videoHeight() * y_mult);

  NOTIMPLEMENTED();
  //auto* swap_chain = GetSwapChain(XRView::kEyeNone);
  //auto* drawing_buffer_ = swap_chain->drawing_buffer();
  //DCHECK(drawing_buffer_);
  //drawing_buffer_->ClearFramebuffers(GL_COLOR_BUFFER_BIT);
  //swap_chain->Resize(new_size);

  auto viewport_pair =
      XRWebGLBinding::createViewportPair(new_size, x_mult, y_mult);
  left_viewport_ = viewport_pair.left_viewport;
  right_viewport_ = viewport_pair.right_viewport;

  updateLayerParams();
}

void XRCompositionLayer::Trace(Visitor* visitor) const {
  visitor->Trace(left_viewport_);
  visitor->Trace(right_viewport_);
  visitor->Trace(media_);
  visitor->Trace(color_textures_);
  visitor->Trace(depth_stencil_textures_);
  XRLayer::Trace(visitor);
}

}  // namespace blink
