#ifndef LB_BITS_H
#define LB_BITS_H

#include <stdint.h>

static inline uint64_t lbWithBits(const uint64_t target, const int position, const int count, const uint64_t value) {
    const uint64_t mask = (1 << count) - 1;
    return (target & ~(mask << position)) | ((value & mask) << position);
}

static inline uint32_t lbWithBits32(const uint32_t target, const int position, const int count, const uint32_t value) {
    const int mask = (1 << count) - 1;
    return (target & ~(mask << position)) | ((value & mask) << position);
}

static inline uint64_t lbGetBits(const uint64_t source, const int position, const uint64_t count) {
    return (source >> position) & ((1 << count) - 1);
}

static inline uint32_t lbGetBits32(const uint32_t source, const int position, const uint32_t count) {
    const int mask = (1 << count) - 1;
    return (source >> position) & mask;
}

#endif //LB_BITS_H