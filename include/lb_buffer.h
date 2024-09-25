#ifndef LB_BUFFER_H
#define LB_BUFFER_H

#include <string.h>

#include "stdint.h"

#ifndef LB_BUFFER_NO_SAFETY
#define LB_BUFFER_SAFETY
#endif

// Error codes for initializing a LB_WriteBuffer.
typedef enum LB_WriteBufferInitError {
    // No error.
    LB_WRITE_BUFFER_INIT_NONE = 0x0,
    // The data is NULL.
    LB_WRITE_BUFFER_INIT_DATA_NULL = 0x1,
    // The buffer is NULL.
    LB_WRITE_BUFFER_INIT_NO_BUFFER = 0x2,
    // The length is zero.
    LB_WRITE_BUFFER_INIT_LENGTH_ZERO = 0x4,
} LB_WriteBufferInitError;

// Error codes for `lbWriteBuffer` methods.
typedef enum LB_WriteBufferWriteError {
    // No error.
    LB_WRITE_BUFFER_WRITE_NONE = 0x0,
    // The buffer is full.
    LB_WRITE_BUFFER_WRITE_FULL = 0x1,
    // The buffer is NULL.
    LB_WRITE_BUFFER_WRITE_BUFFER_NULL = 0x2,
    // The data is NULL.
    LB_WRITE_BUFFER_WRITE_DATA_NULL = 0x4,

    // The value is invalid.
    LB_WRITE_BUFFER_WRITE_INVALID_VALUE = 0x8,
} LB_WriteBufferWriteError;

typedef struct LB_WriteBuffer {
    void *data;
    size_t length;
    size_t position;
} LB_WriteBuffer;

typedef struct LB_ReadBuffer {
    const void *data;
    size_t length;
    size_t position_bits;
} LB_ReadBuffer;

/**
 * Initialize a LB_WriteBuffer and returns an error code, if any.                           <br>
 * Will not do error checking if LB_BUFFER_NO_SAFETY is defined.
 *
 * @param buffer A pointer to the LB_WriteBuffer to be initialized.                         <br>
 * If NULL and LB_BUFFER_SAFETY, will return LB_WRITE_BUFFER_INIT_NO_BUFFER.                <br>
 * @param data A pointer to the data to be written to.                                      <br>
 * If NULL and LB_BUFFER_SAFETY, will return LB_WRITE_BUFFER_INIT_DATA_NULL.                <br>
 * @param length The length of the data to be written.                                      <br>
 * If 0 and LB_BUFFER_SAFETY, will return LB_WRITE_BUFFER_INIT_LENGTH_ZERO.                 <br>
 * @return LB_WriteBufferInitError An error code indicating the result of the initialization.
 */
inline LB_WriteBufferInitError lbWriteBufferInit(LB_WriteBuffer *buffer, void *data, const size_t length) {
#ifdef LB_BUFFER_SAFETY
    LB_WriteBufferInitError e = LB_WRITE_BUFFER_INIT_NONE;
    if (buffer == NULL) {
        e |= LB_WRITE_BUFFER_INIT_NO_BUFFER;
    }

    if (data == NULL) {
        e |= LB_WRITE_BUFFER_INIT_DATA_NULL;
    }

    if (length == 0) {
        e |= LB_WRITE_BUFFER_INIT_LENGTH_ZERO;
    }

    if (e) {
        return e;
    }
#endif

    buffer->data = data;
    buffer->length = length;
    buffer->position = 0;
    return LB_WRITE_BUFFER_INIT_NONE;
}

#ifdef LB_BUFFER_SAFETY
inline LB_WriteBufferWriteError lbWriteBufferWriteSafety(LB_WriteBuffer *buffer, size_t length, const void *value) {
    LB_WriteBufferWriteError e = LB_WRITE_BUFFER_WRITE_NONE;
    if (buffer == NULL) {
        e |= LB_WRITE_BUFFER_WRITE_BUFFER_NULL;
    } else {
        if (buffer->position + length > buffer->length) {
            e |= LB_WRITE_BUFFER_WRITE_FULL;
        }

        if (buffer->data == NULL) {
            e |= LB_WRITE_BUFFER_WRITE_DATA_NULL;
        }
    }

    if (value == NULL) {
        e |= LB_WRITE_BUFFER_WRITE_INVALID_VALUE;
    }

    return e;
}

#else
#define lbWriteBufferWriteSafety(buffer, length, value) LB_WRITE_BUFFER_WRITE_NONE
#endif

inline LB_WriteBufferWriteError lbWriteBufferWriteUnsafe(LB_WriteBuffer *buffer, size_t length, const void *value) {
    memcpy(((uint8_t *) buffer->data) + buffer->position, value, length);
    buffer->position += length;
    return LB_WRITE_BUFFER_WRITE_NONE;
}

inline LB_WriteBufferWriteError lbWriteBufferWrite(LB_WriteBuffer *buffer, size_t length, const void *value) {
#ifdef LB_BUFFER_SAFETY
    const LB_WriteBufferWriteError e = lbWriteBufferWriteSafety(buffer, length, value);
    if (e) {
        return e;
    }
#endif

    return lbWriteBufferWriteUnsafe(buffer, length, value);
}

inline LB_WriteBufferWriteError lbWriteBufferWriteReversedUnsafe(LB_WriteBuffer *buffer, size_t length,
                                                                 const void *value) {
    for (size_t i = 0; i < length; i++) {
        *((uint8_t *) buffer->data + buffer->position + i) = *((uint8_t *) value + length - 1 - i);
    }

    buffer->position += length;
    return LB_WRITE_BUFFER_WRITE_NONE;
}

inline LB_WriteBufferWriteError lbWriteBufferWriteReversed(LB_WriteBuffer *buffer, size_t length, const void *value) {
#ifdef LB_BUFFER_SAFETY
    const LB_WriteBufferWriteError e = lbWriteBufferWriteSafety(buffer, length, value);
    if (e) {
        return e;
    }
#endif

    return lbWriteBufferWriteReversedUnsafe(buffer, length, value);
}

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define lbWriteBufferLE lbWriteBufferWrite
#define lbWriteBufferBE lbWriteBufferWriteReversed
#define lbWriteBufferUnsafeLE lbWriteBufferWriteUnsafe
#define lbWriteBufferUnsafeBE lbWriteBufferWriteReversedUnsafe
#else
#define lbWriteBufferLE lbWriteBufferWriteReversed
#define lbWriteBufferBE lbWriteBufferWrite
#define lbWriteBufferUnsafeLE lbWriteBufferWriteReversedUnsafe
#define lbWriteBufferUnsafeBE lbWriteBufferWriteUnsafe
#endif

inline LB_WriteBufferWriteError lbWriteBufferU8(LB_WriteBuffer *buffer, uint8_t value) {
    return lbWriteBufferWrite(buffer, 1, &value);
}

inline LB_WriteBufferWriteError lbWriteBufferU8LE(LB_WriteBuffer *buffer, uint8_t value) {
    return lbWriteBufferLE(buffer, 1, &value);
}

inline LB_WriteBufferWriteError lbWriteBufferU8BE(LB_WriteBuffer *buffer, uint8_t value) {
    return lbWriteBufferBE(buffer, 1, &value);
}

inline LB_WriteBufferWriteError lbWriteBufferU16(LB_WriteBuffer *buffer, uint16_t value) {
    return lbWriteBufferWrite(buffer, 2, &value);
}

inline LB_WriteBufferWriteError lbWriteBufferU16LE(LB_WriteBuffer *buffer, uint16_t value) {
    return lbWriteBufferLE(buffer, 2, &value);
}

inline LB_WriteBufferWriteError lbWriteBufferU16BE(LB_WriteBuffer *buffer, uint16_t value) {
    return lbWriteBufferBE(buffer, 2, &value);
}

inline LB_WriteBufferWriteError lbWriteBufferU32(LB_WriteBuffer *buffer, uint32_t value) {
    return lbWriteBufferWrite(buffer, 4, &value);
}

inline LB_WriteBufferWriteError lbWriteBufferU32LE(LB_WriteBuffer *buffer, uint32_t value) {
    return lbWriteBufferLE(buffer, 4, &value);
}

inline LB_WriteBufferWriteError lbWriteBufferU32BE(LB_WriteBuffer *buffer, uint32_t value) {
    return lbWriteBufferBE(buffer, 4, &value);
}


inline LB_WriteBufferWriteError lbWriteBufferU64(LB_WriteBuffer *buffer, uint64_t value) {
    return lbWriteBufferWrite(buffer, 8, &value);
}

inline LB_WriteBufferWriteError lbWriteBufferU64LE(LB_WriteBuffer *buffer, uint64_t value) {
    return lbWriteBufferLE(buffer, 8, &value);
}

inline LB_WriteBufferWriteError lbWriteBufferU64BE(LB_WriteBuffer *buffer, uint64_t value) {
    return lbWriteBufferBE(buffer, 8, &value);
}


inline LB_WriteBufferWriteError lbWriteBufferI8(LB_WriteBuffer *buffer, int8_t value) {
    return lbWriteBufferWrite(buffer, 1, &value);
}

inline LB_WriteBufferWriteError lbWriteBufferI8LE(LB_WriteBuffer *buffer, int8_t value) {
    return lbWriteBufferLE(buffer, 1, &value);
}

inline LB_WriteBufferWriteError lbWriteBufferI8BE(LB_WriteBuffer *buffer, int8_t value) {
    return lbWriteBufferBE(buffer, 1, &value);
}

inline LB_WriteBufferWriteError lbWriteBufferI16(LB_WriteBuffer *buffer, int16_t value) {
    return lbWriteBufferWrite(buffer, 2, &value);
}

inline LB_WriteBufferWriteError lbWriteBufferI16LE(LB_WriteBuffer *buffer, int16_t value) {
    return lbWriteBufferLE(buffer, 2, &value);
}

inline LB_WriteBufferWriteError lbWriteBufferI16BE(LB_WriteBuffer *buffer, int16_t value) {
    return lbWriteBufferBE(buffer, 2, &value);
}

inline LB_WriteBufferWriteError lbWriteBufferI32(LB_WriteBuffer *buffer, int32_t value) {
    return lbWriteBufferWrite(buffer, 4, &value);
}

inline LB_WriteBufferWriteError lbWriteBufferI32LE(LB_WriteBuffer *buffer, int32_t value) {
    return lbWriteBufferLE(buffer, 4, &value);
}

inline LB_WriteBufferWriteError lbWriteBufferI32BE(LB_WriteBuffer *buffer, int32_t value) {
    return lbWriteBufferBE(buffer, 4, &value);
}


inline LB_WriteBufferWriteError lbWriteBufferI64(LB_WriteBuffer *buffer, int64_t value) {
    return lbWriteBufferWrite(buffer, 8, &value);
}

inline LB_WriteBufferWriteError lbWriteBufferI64LE(LB_WriteBuffer *buffer, int64_t value) {
    return lbWriteBufferLE(buffer, 8, &value);
}

inline LB_WriteBufferWriteError lbWriteBufferI64BE(LB_WriteBuffer *buffer, int64_t value) {
    return lbWriteBufferBE(buffer, 8, &value);
}


inline LB_WriteBufferWriteError lbWriteBufferF32(LB_WriteBuffer *buffer, float value) {
    return lbWriteBufferWrite(buffer, 4, &value);
}

inline LB_WriteBufferWriteError lbWriteBufferF32LE(LB_WriteBuffer *buffer, float value) {
    return lbWriteBufferLE(buffer, 4, &value);
}

inline LB_WriteBufferWriteError lbWriteBufferF32BE(LB_WriteBuffer *buffer, float value) {
    return lbWriteBufferBE(buffer, 4, &value);
}


inline LB_WriteBufferWriteError lbWriteBufferF64(LB_WriteBuffer *buffer, double value) {
    return lbWriteBufferWrite(buffer, 8, &value);
}

inline LB_WriteBufferWriteError lbWriteBufferF64LE(LB_WriteBuffer *buffer, double value) {
    return lbWriteBufferLE(buffer, 8, &value);
}

inline LB_WriteBufferWriteError lbWriteBufferF64BE(LB_WriteBuffer *buffer, double value) {
    return lbWriteBufferBE(buffer, 8, &value);
}

/*
 * Normalized Integer Functions
 */

// safety stuff
#ifdef LB_BUFFER_SAFETY
#define LB_WRITE_BUFFER_NORMALIZED_UNSIGNED_SAFETY \
    LB_WriteBufferWriteError e = lbWriteBufferWriteSafety(buffer, 1, &normalized); \
    if (value < 0.0f || value > 1.0f) { \
        e |= LB_WRITE_BUFFER_WRITE_INVALID_VALUE; \
    } \
    if (e) { \
        return e; \
    }

#define LB_WRITE_BUFFER_NORMALIZED_SIGNED_SAFETY \
LB_WriteBufferWriteError e = lbWriteBufferWriteSafety(buffer, 1, &normalized); \
if (value < -1.0f || value > 1.0f) { \
e |= LB_WRITE_BUFFER_WRITE_INVALID_VALUE; \
} \
if (e) { \
return e; \
}
#else
#define LB_WRITE_BUFFER_NORMALIZED_SIGNED_SAFETY
#define LB_WRITE_BUFFER_NORMALIZED_UNSIGNED_SAFETY
#endif

inline LB_WriteBufferWriteError lbWriteBufferNU8(LB_WriteBuffer *buffer, float value) {
    const uint8_t normalized = (uint8_t) (value * ((float) UINT8_MAX) + 0.5f);
    LB_WRITE_BUFFER_NORMALIZED_UNSIGNED_SAFETY
    return lbWriteBufferWriteUnsafe(buffer, 1, &normalized);
}

inline LB_WriteBufferWriteError lbWriteBufferNU8LE(LB_WriteBuffer *buffer, float value) {
    return lbWriteBufferNU8(buffer, value);
}

inline LB_WriteBufferWriteError lbWriteBufferNU8BE(LB_WriteBuffer *buffer, float value) {
    return lbWriteBufferNU8(buffer, value);
}


inline LB_WriteBufferWriteError lbWriteBufferNU16(LB_WriteBuffer *buffer, float value) {
    const uint16_t normalized = (uint16_t) (value * ((float) UINT16_MAX) + 0.5f);
    LB_WRITE_BUFFER_NORMALIZED_UNSIGNED_SAFETY
    return lbWriteBufferWriteUnsafe(buffer, 2, &normalized);
}

inline LB_WriteBufferWriteError lbWriteBufferNU16LE(LB_WriteBuffer *buffer, float value) {
    return lbWriteBufferNU16(buffer, value);
}

inline LB_WriteBufferWriteError lbWriteBufferNU16BE(LB_WriteBuffer *buffer, float value) {
    return lbWriteBufferNU16(buffer, value);
}

/*  Normalized integers utilizing more than 23 bits can carry more precision than a float.  */
inline LB_WriteBufferWriteError lbWriteBufferNU32(LB_WriteBuffer *buffer, double value) {
    const uint32_t normalized = (uint32_t) (value * ((double) UINT32_MAX) + 0.5);
    LB_WRITE_BUFFER_NORMALIZED_UNSIGNED_SAFETY
    return lbWriteBufferWriteUnsafe(buffer, 4, &normalized);
}

inline LB_WriteBufferWriteError lbWriteBufferNU32LE(LB_WriteBuffer *buffer, double value) {
    return lbWriteBufferNU32(buffer, value);
}

inline LB_WriteBufferWriteError lbWriteBufferNU32BE(LB_WriteBuffer *buffer, double value) {
    return lbWriteBufferNU32(buffer, value);
}


inline LB_WriteBufferWriteError lbWriteBufferNU64(LB_WriteBuffer *buffer, double value) {
    const uint64_t normalized = (uint64_t) (value * ((double) UINT64_MAX) + 0.5);
    LB_WRITE_BUFFER_NORMALIZED_UNSIGNED_SAFETY
    return lbWriteBufferWriteUnsafe(buffer, 8, &normalized);
}

inline LB_WriteBufferWriteError lbWriteBufferNU64LE(LB_WriteBuffer *buffer, double value) {
    return lbWriteBufferNU64(buffer, value);
}

inline LB_WriteBufferWriteError lbWriteBufferNU64BE(LB_WriteBuffer *buffer, double value) {
    return lbWriteBufferNU64(buffer, value);
}


inline LB_WriteBufferWriteError lbWriteBufferNI8(LB_WriteBuffer *buffer, float value) {
    const int8_t normalized = (int8_t) (value * ((float) INT8_MAX / 2) + 0.5f);
    LB_WRITE_BUFFER_NORMALIZED_SIGNED_SAFETY
    return lbWriteBufferWriteUnsafe(buffer, 1, &normalized);
}

inline LB_WriteBufferWriteError lbWriteBufferNI8LE(LB_WriteBuffer *buffer, float value) {
    return lbWriteBufferNI8(buffer, value);
}

inline LB_WriteBufferWriteError lbWriteBufferNI8BE(LB_WriteBuffer *buffer, float value) {
    return lbWriteBufferNI8(buffer, value);
}


inline LB_WriteBufferWriteError lbWriteBufferNI16(LB_WriteBuffer *buffer, float value) {
    const int16_t normalized = (int16_t) (value * ((float) INT16_MAX / 2) + 0.5f);
    LB_WRITE_BUFFER_NORMALIZED_SIGNED_SAFETY
    return lbWriteBufferWriteUnsafe(buffer, 2, &normalized);
}

inline LB_WriteBufferWriteError lbWriteBufferNI16LE(LB_WriteBuffer *buffer, float value) {
    return lbWriteBufferNI16(buffer, value);
}

inline LB_WriteBufferWriteError lbWriteBufferNI16BE(LB_WriteBuffer *buffer, float value) {
    return lbWriteBufferNI16(buffer, value);
}


inline LB_WriteBufferWriteError lbWriteBufferNI32(LB_WriteBuffer *buffer, double value) {
    const int32_t normalized = (int32_t) (value * ((double) INT32_MAX / 2) + 0.5);
    LB_WRITE_BUFFER_NORMALIZED_SIGNED_SAFETY
    return lbWriteBufferWriteUnsafe(buffer, 4, &normalized);
}

inline LB_WriteBufferWriteError lbWriteBufferNI32LE(LB_WriteBuffer *buffer, double value) {
    return lbWriteBufferNI32(buffer, value);
}

inline LB_WriteBufferWriteError lbWriteBufferNI32BE(LB_WriteBuffer *buffer, double value) {
    return lbWriteBufferNI32(buffer, value);
}


inline LB_WriteBufferWriteError lbWriteBufferNI64(LB_WriteBuffer *buffer, double value) {
    const int64_t normalized = (int64_t) (value * ((double) INT64_MAX / 2) + 0.5);
    LB_WRITE_BUFFER_NORMALIZED_SIGNED_SAFETY
    return lbWriteBufferWriteUnsafe(buffer, 8, &normalized);
}

inline LB_WriteBufferWriteError lbWriteBufferNI64LE(LB_WriteBuffer *buffer, double value) {
    return lbWriteBufferNI64(buffer, value);
}

inline LB_WriteBufferWriteError lbWriteBufferNI64BE(LB_WriteBuffer *buffer, double value) {
    return lbWriteBufferNI64(buffer, value);
}

#ifdef LB_BUFFER_IMPLEMENTATION
LB_WriteBufferInitError lbWriteBufferInit(LB_WriteBuffer *buffer, void *data, size_t length);
#ifdef LB_BUFFER_SAFETY
LB_WriteBufferWriteError lbWriteBufferWriteSafety(LB_WriteBuffer *buffer, size_t length, const void *value);
#endif
LB_WriteBufferWriteError lbWriteBufferWriteUnsafe(LB_WriteBuffer *buffer, size_t length, const void *value);
LB_WriteBufferWriteError lbWriteBufferWrite(LB_WriteBuffer *buffer, size_t length, const void *value);
LB_WriteBufferWriteError lbWriteBufferWriteReversedUnsafe(LB_WriteBuffer *buffer, size_t length, const void *value);
LB_WriteBufferWriteError lbWriteBufferWriteReversed(LB_WriteBuffer *buffer, size_t length, const void *value);
LB_WriteBufferWriteError lbWriteBufferU8(LB_WriteBuffer *buffer, uint8_t value);
LB_WriteBufferWriteError lbWriteBufferU8LE(LB_WriteBuffer *buffer, uint8_t value);
LB_WriteBufferWriteError lbWriteBufferU8BE(LB_WriteBuffer *buffer, uint8_t value);
LB_WriteBufferWriteError lbWriteBufferU16(LB_WriteBuffer *buffer, uint16_t value);
LB_WriteBufferWriteError lbWriteBufferU16LE(LB_WriteBuffer *buffer, uint16_t value);
LB_WriteBufferWriteError lbWriteBufferU16BE(LB_WriteBuffer *buffer, uint16_t value);
LB_WriteBufferWriteError lbWriteBufferU32(LB_WriteBuffer *buffer, uint32_t value);
LB_WriteBufferWriteError lbWriteBufferU32LE(LB_WriteBuffer *buffer, uint32_t value);
LB_WriteBufferWriteError lbWriteBufferU32BE(LB_WriteBuffer *buffer, uint32_t value);
LB_WriteBufferWriteError lbWriteBufferU64(LB_WriteBuffer *buffer, uint64_t value);
LB_WriteBufferWriteError lbWriteBufferU64LE(LB_WriteBuffer *buffer, uint64_t value);
LB_WriteBufferWriteError lbWriteBufferU64BE(LB_WriteBuffer *buffer, uint64_t value);
LB_WriteBufferWriteError lbWriteBufferI8(LB_WriteBuffer *buffer, int8_t value);
LB_WriteBufferWriteError lbWriteBufferI8LE(LB_WriteBuffer *buffer, int8_t value);
LB_WriteBufferWriteError lbWriteBufferI8BE(LB_WriteBuffer *buffer, int8_t value);
LB_WriteBufferWriteError lbWriteBufferI16(LB_WriteBuffer *buffer, int16_t value);
LB_WriteBufferWriteError lbWriteBufferI16LE(LB_WriteBuffer *buffer, int16_t value);
LB_WriteBufferWriteError lbWriteBufferI16BE(LB_WriteBuffer *buffer, int16_t value);
LB_WriteBufferWriteError lbWriteBufferI32(LB_WriteBuffer *buffer, int32_t value);
LB_WriteBufferWriteError lbWriteBufferI32LE(LB_WriteBuffer *buffer, int32_t value);
LB_WriteBufferWriteError lbWriteBufferI32BE(LB_WriteBuffer *buffer, int32_t value);
LB_WriteBufferWriteError lbWriteBufferI64(LB_WriteBuffer *buffer, int64_t value);
LB_WriteBufferWriteError lbWriteBufferI64LE(LB_WriteBuffer *buffer, int64_t value);
LB_WriteBufferWriteError lbWriteBufferI64BE(LB_WriteBuffer *buffer, int64_t value);
LB_WriteBufferWriteError lbWriteBufferF32(LB_WriteBuffer *buffer, float value);
LB_WriteBufferWriteError lbWriteBufferF32LE(LB_WriteBuffer *buffer, float value);
LB_WriteBufferWriteError lbWriteBufferF32BE(LB_WriteBuffer *buffer, float value);
LB_WriteBufferWriteError lbWriteBufferF64(LB_WriteBuffer *buffer, double value);
LB_WriteBufferWriteError lbWriteBufferF64LE(LB_WriteBuffer *buffer, double value);
LB_WriteBufferWriteError lbWriteBufferF64BE(LB_WriteBuffer *buffer, double value);
LB_WriteBufferWriteError lbWriteBufferNU8(LB_WriteBuffer *buffer, float value);
LB_WriteBufferWriteError lbWriteBufferNU8LE(LB_WriteBuffer *buffer, float value);
LB_WriteBufferWriteError lbWriteBufferNU8BE(LB_WriteBuffer *buffer, float value);
LB_WriteBufferWriteError lbWriteBufferNU16(LB_WriteBuffer *buffer, float value);
LB_WriteBufferWriteError lbWriteBufferNU16LE(LB_WriteBuffer *buffer, float value);
LB_WriteBufferWriteError lbWriteBufferNU16BE(LB_WriteBuffer *buffer, float value);
LB_WriteBufferWriteError lbWriteBufferNU32(LB_WriteBuffer *buffer, double value);
LB_WriteBufferWriteError lbWriteBufferNU32LE(LB_WriteBuffer *buffer, double value);
LB_WriteBufferWriteError lbWriteBufferNU32BE(LB_WriteBuffer *buffer, double value);
LB_WriteBufferWriteError lbWriteBufferNU64(LB_WriteBuffer *buffer, double value);
LB_WriteBufferWriteError lbWriteBufferNU64LE(LB_WriteBuffer *buffer, double value);
LB_WriteBufferWriteError lbWriteBufferNU64BE(LB_WriteBuffer *buffer, double value);
LB_WriteBufferWriteError lbWriteBufferNI8(LB_WriteBuffer *buffer, float value);
LB_WriteBufferWriteError lbWriteBufferNI8LE(LB_WriteBuffer *buffer, float value);
LB_WriteBufferWriteError lbWriteBufferNI8BE(LB_WriteBuffer *buffer, float value);
LB_WriteBufferWriteError lbWriteBufferNI16(LB_WriteBuffer *buffer, float value);
LB_WriteBufferWriteError lbWriteBufferNI16LE(LB_WriteBuffer *buffer, float value);
LB_WriteBufferWriteError lbWriteBufferNI16BE(LB_WriteBuffer *buffer, float value);
LB_WriteBufferWriteError lbWriteBufferNI32(LB_WriteBuffer *buffer, double value);
LB_WriteBufferWriteError lbWriteBufferNI32LE(LB_WriteBuffer *buffer, double value);
LB_WriteBufferWriteError lbWriteBufferNI32BE(LB_WriteBuffer *buffer, double value);
LB_WriteBufferWriteError lbWriteBufferNI64(LB_WriteBuffer *buffer, double value);
LB_WriteBufferWriteError lbWriteBufferNI64LE(LB_WriteBuffer *buffer, double value);
LB_WriteBufferWriteError lbWriteBufferNI64BE(LB_WriteBuffer *buffer, double value);
#endif

#endif
