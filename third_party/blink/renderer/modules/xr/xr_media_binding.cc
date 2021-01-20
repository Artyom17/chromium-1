// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

#include "third_party/blink/renderer/modules/xr/xr_media_binding.h"

#include "third_party/blink/renderer/core/html/media/html_video_element.h"
#include "third_party/blink/renderer/modules/xr/xr_composition_layer.h"
#include "third_party/blink/renderer/modules/xr/xr_cylinder_layer_init.h"
#include "third_party/blink/renderer/modules/xr/xr_equirect_layer_init.h"
#include "third_party/blink/renderer/modules/xr/xr_media_cylinder_layer_init.h"
#include "third_party/blink/renderer/modules/xr/xr_media_equirect_layer_init.h"
#include "third_party/blink/renderer/modules/xr/xr_media_layer_init.h"
#include "third_party/blink/renderer/modules/xr/xr_media_quad_layer_init.h"
#include "third_party/blink/renderer/modules/xr/xr_quad_layer_init.h"
#include "third_party/blink/renderer/modules/xr/xr_session.h"
#include "third_party/blink/renderer/modules/xr/xr_webgl_binding.h"
#include "third_party/blink/renderer/platform/bindings/exception_state.h"

namespace blink {

namespace {
constexpr char kSessionEnded[] =
    "Cannot create an XRMediaBinding for an "
    "XRSession which has already ended.";
constexpr char kImmersive[] =
    "Cannot create an XRMediaBinding with a "
    "non-immersive session.";
constexpr char kVideoDimensions[] =
    "One or more of the video dimensions are zero,"
    "is the video ready? ";
constexpr char kWrongLayout[] = "Invalid layout passed to layer.";
}  // namespace

XRMediaBinding* XRMediaBinding::Create(XRSession* session,
                                       ExceptionState& exception_state) {
  if (session->ended()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kSessionEnded);
    return nullptr;
  }

  if (!session->immersive()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kImmersive);
    return nullptr;
  }

  return MakeGarbageCollected<XRMediaBinding>(session);
}

XRMediaBinding::XRMediaBinding(XRSession* session) : session_(session) {}

namespace {

template <typename Layer, typename Initializer>
using LayerCreatorT = Layer* (XRWebGLBinding::*)(ScriptState*,
                                                 Initializer*,
                                                 ExceptionState&,
                                                 HTMLVideoElement*);
template <typename Layer,
          typename Initializer,
          LayerCreatorT<Layer, Initializer> LayerCreator>
Layer* createVideoLayer(XRSession* session,
                        ScriptState* script_state,
                        HTMLVideoElement* video,
                        XRMediaLayerInit* init,
                        Initializer& layer_init,
                        ExceptionState& exception_state) {
  if (!video->videoWidth() || !video->videoHeight()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kVideoDimensions);
    return nullptr;
  }

  if (init->layout() == XRCompositionLayer::kLayoutDefault ||
      init->layout() == XRCompositionLayer::kLayoutStereo) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kWrongLayout);
  }

  auto* binding = session->getOrCreateMediaLayerManager(exception_state);

  if (!binding) {
    return nullptr;
  }

  // init->invertStereo ignored on stereoscopic displays
  layer_init.setSpace(init->space());
  layer_init.setViewPixelWidth(
      init->layout() == XRCompositionLayer::kLayoutStereoLeftRight
          ? video->videoWidth() / 2
          : video->videoWidth());
  layer_init.setViewPixelHeight(
      init->layout() == XRCompositionLayer::kLayoutStereoTopBottom
          ? video->videoHeight() / 2
          : video->videoHeight());
  layer_init.setLayout(init->layout());
  layer_init.setIsStatic(true);
  layer_init.setTextureType("texture");

  return (binding->*LayerCreator)(script_state, &layer_init,
                                  exception_state, video);
}

float calcAspectRatio(const XRMediaLayerInit* init,
                      const HTMLVideoElement* video) {
  DCHECK(init);
  DCHECK(video);
  const uint32_t videoW = init->layout() ==
                            XRCompositionLayer::kLayoutStereoLeftRight
                        ? video->videoWidth() / 2
                        : video->videoWidth();
  const uint32_t videoH = init->layout() ==
                            XRCompositionLayer::kLayoutStereoTopBottom
                        ? video->videoHeight() / 2
                        : video->videoHeight();
  return float(videoW) / videoH;
}

}  // namespace

XRQuadLayer* XRMediaBinding::createQuadLayer(
    ScriptState* script_state,
    HTMLVideoElement* video,
    XRMediaQuadLayerInit* init,
    ExceptionState& exception_state) {
  DCHECK(init);
  XRQuadLayerInit layer_init;
  if (init->hasTransform()) {
    layer_init.setTransform(init->transform());
  }
  if (!init->hasWidth() || !init->hasHeight()) {
    const float aspectRatio = calcAspectRatio(init, video);
    if (!init->hasWidth() && init->hasHeight()) {
      layer_init.setWidth(init->height() * aspectRatio);
    }
    else if (init->hasWidth() && !init->hasHeight()) {
      layer_init.setHeight(init->width() / aspectRatio);
    } else {
      layer_init.setWidth(1.0f);
      layer_init.setHeight(1.0f / aspectRatio);
    }
  } else {
    layer_init.setWidth(init->width());
    layer_init.setHeight(init->height());
  }
  return createVideoLayer<XRQuadLayer, XRQuadLayerInit,
                          &XRWebGLBinding::createQuadLayer>(
      session_, script_state, video, init, layer_init, exception_state);
}

XRCylinderLayer* XRMediaBinding::createCylinderLayer(
    ScriptState* script_state,
    HTMLVideoElement* video,
    XRMediaCylinderLayerInit* init,
    ExceptionState& exception_state) {
  DCHECK(init);
  XRCylinderLayerInit layer_init;
  if (init->hasTransform()) {
    layer_init.setTransform(init->transform());
  }
  layer_init.setRadius(init->radius());
  layer_init.setCentralAngle(init->centralAngle());
  if (init->hasAspectRatio()) {
    layer_init.setAspectRatio(init->aspectRatio());
  } else {
    layer_init.setAspectRatio(calcAspectRatio(init, video));
  }
  return createVideoLayer<XRCylinderLayer, XRCylinderLayerInit,
                          &XRWebGLBinding::createCylinderLayer>(
      session_, script_state, video, init, layer_init, exception_state);
}

XREquirectLayer* XRMediaBinding::createEquirectLayer(
    ScriptState* script_state,
    HTMLVideoElement* video,
    XRMediaEquirectLayerInit* init,
    ExceptionState& exception_state) {
  DCHECK(init);
  XREquirectLayerInit layer_init;
  if (init->hasTransform()) {
    layer_init.setTransform(init->transform());
  }
  layer_init.setCentralHorizontalAngle(init->centralHorizontalAngle());
  layer_init.setUpperVerticalAngle(init->upperVerticalAngle());
  layer_init.setLowerVerticalAngle(init->lowerVerticalAngle());
  return createVideoLayer<XREquirectLayer, XREquirectLayerInit,
                          &XRWebGLBinding::createEquirectLayer>(
      session_, script_state, video, init, layer_init, exception_state);
}

void XRMediaBinding::Trace(Visitor* visitor) const {
  visitor->Trace(session_);
  ScriptWrappable::Trace(visitor);
}

}  // namespace blink
