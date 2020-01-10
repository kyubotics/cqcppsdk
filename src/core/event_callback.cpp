#include "./event_callback.h"

namespace cq {
#define DEF_EVENT(EventName, EventType) std::vector<std::function<void(EventType)>> _##EventName##_callbacks;

#include "./events.inc"

#undef DEF_EVENT
} // namespace cq
