#include "./init.h"

namespace cq {
    int32_t _ac = 0;
    void (*_init_impl)() = nullptr;
} // namespace cq
