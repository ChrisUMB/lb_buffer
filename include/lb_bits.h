#ifndef LB_BITS_H
#define LB_BITS_H

#include <stdint.h>

static inline uint64_t lbWithBits64(const uint64_t target, const uint32_t position, const uint32_t count, const uint64_t value) {
    const uint64_t mask = (1 << count) - 1;
    return (target & ~(mask << position)) | ((value & mask) << position);
}

static inline uint32_t lbWithBits32(const uint32_t target, const uint32_t position, const uint32_t count, const uint32_t value) {
    const int mask = (1 << count) - 1;
    return (target & ~(mask << position)) | ((value & mask) << position);
}

static inline uint64_t lbGetBits64(const uint64_t source, const uint32_t position, const uint64_t count) {
    return (source >> position) & ((1 << count) - 1);
}

static inline uint32_t lbGetBits32(const uint32_t source, const uint32_t position, const uint32_t count) {
    const int mask = (1 << count) - 1;
    return (source >> position) & mask;
}

#endif //LB_BITS_H