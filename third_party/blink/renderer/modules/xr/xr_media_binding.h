// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_MEDIA_BINDING_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_MEDIA_BINDING_H_

#include "third_party/blink/renderer/modules/xr/xr_media_binding.h"
#include "third_party/blink/renderer/platform/bindings/script_wrappable.h"

namespace blink {
class ExceptionState;
class HTMLVideoElement;
class ScriptState;
class XRCylinderLayer;
class XREquirectLayer;
class XRMediaCylinderLayerInit;
class XRMediaEquirectLayerInit;
class XRMediaQuadLayerInit;
class XRQuadLayer;
class XRSession;

class XRMediaBinding : public ScriptWrappable {
  DEFINE_WRAPPERTYPEINFO();

 public:
  XRMediaBinding(XRSession*);
  ~XRMediaBinding() override = default;

  static XRMediaBinding* Create(XRSession*, ExceptionState&);

  XRQuadLayer* createQuadLayer(ScriptState* script_state,
                               HTMLVideoElement* video,
                               XRMediaQuadLayerInit* init,
                               ExceptionState& exception_state);
  XRCylinderLayer* createCylinderLayer(ScriptState* script_state,
                                       HTMLVideoElement* video,
                                       XRMediaCylinderLayerInit* init,
                                       ExceptionState& exception_state);
  XREquirectLayer* createEquirectLayer(ScriptState* script_state,
                                       HTMLVideoElement* video,
                                       XRMediaEquirectLayerInit* init,
                                       ExceptionState& exception_state);

  void Trace(Visitor*) const override;

 protected:
  const Member<XRSession> session_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_MEDIA_BINDING_H_
