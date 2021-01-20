// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

#ifndef THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_LAYER_EVENT_H_
#define THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_LAYER_EVENT_H_

#include "third_party/blink/renderer/bindings/modules/v8/v8_xr_layer_event_init.h"
#include "third_party/blink/renderer/modules/event_modules.h"
#include "third_party/blink/renderer/modules/xr/xr_layer.h"

namespace blink {

class XRLayerEvent final : public Event {
  DEFINE_WRAPPERTYPEINFO();

 public:
  static XRLayerEvent* Create() {
    return MakeGarbageCollected<XRLayerEvent>();
  }
  static XRLayerEvent* Create(const AtomicString& type, XRLayer* layer) {
    return MakeGarbageCollected<XRLayerEvent>(type, layer);
  }

  static XRLayerEvent* Create(const AtomicString& type,
                                const XRLayerEventInit* initializer) {
    return MakeGarbageCollected<XRLayerEvent>(type, initializer);
  }

  XRLayerEvent();
  XRLayerEvent(const AtomicString& type, XRLayer*);
  XRLayerEvent(const AtomicString& type,
                 XRLayer*,
                 Event::Bubbles,
                 Event::Cancelable,
                 Event::ComposedMode);
  XRLayerEvent(const AtomicString& type, const XRLayerEventInit*);
  ~XRLayerEvent() override;

  XRLayer* layer() const { return layer_.Get(); }

  const AtomicString& InterfaceName() const override;

  void Trace(Visitor*) const override;

 private:
  Member<XRLayer> layer_;
};

}  // namespace blink

#endif  // THIRD_PARTY_BLINK_RENDERER_MODULES_XR_XR_LAYER_EVENT_H_
