// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

#include "third_party/blink/renderer/modules/xr/xr_layer_event.h"

namespace blink {

XRLayerEvent::XRLayerEvent() = default;

XRLayerEvent::XRLayerEvent(const AtomicString& type, XRLayer* layer)
    : Event(type, Bubbles::kNo, Cancelable::kYes), layer_(layer) {}

XRLayerEvent::XRLayerEvent(const AtomicString& type,
                               XRLayer* layer,
                               Event::Bubbles bubbles,
                               Event::Cancelable cancelable,
                               Event::ComposedMode composed)
    : Event(type, bubbles, cancelable, composed), layer_(layer) {}

XRLayerEvent::XRLayerEvent(const AtomicString& type,
                               const XRLayerEventInit* initializer)
    : Event(type, initializer) {
  if (initializer->hasLayer())
    layer_ = initializer->layer();
}

XRLayerEvent::~XRLayerEvent() = default;

const AtomicString& XRLayerEvent::InterfaceName() const {
  return event_interface_names::kXRLayerEvent;
}

void XRLayerEvent::Trace(Visitor* visitor) const {
  visitor->Trace(layer_);
  Event::Trace(visitor);
}

}  // namespace blink
