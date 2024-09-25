#ifndef LB_READ_BUFFER_H
#define LB_READ_BUFFER_H

#ifdef __cplusplus
#include <cstring>
#include <cstdint>
extern "C" {
#else
#include <string.h>
#include <stdint.h>
#endif

#ifndef LB_READ_BUFFER_NO_SAFETY
#define LB_READ_BUFFER_SAFETY
#endif

// Error codes for initializing a LB_ReadBuffer.
typedef enum LB_ReadBufferInitError {
    // No error.
    LB_READ_BUFFER_INIT_NONE = 0x0,
    // The data is NULL.
    LB_READ_BUFFER_INIT_DATA_NULL = 0x1,
    // The buffer is NULL.
    LB_READ_BUFFER_INIT_NO_BUFFER = 0x2,
    // The length is zero.
    LB_READ_BUFFER_INIT_LENGTH_ZERO = 0x4,
} LB_ReadBufferInitError;

// Error codes for `lbReadBuffer` methods.
typedef enum LB_ReadBufferReadError {
    // No error.
    LB_READ_BUFFER_READ_NONE = 0x0,
    // The buffer is at the end.
    LB_READ_BUFFER_READ_END = 0x1,
    // The buffer is NULL.
    LB_READ_BUFFER_READ_BUFFER_NULL = 0x2,
    // The data is NULL.
    LB_READ_BUFFER_READ_DATA_NULL = 0x4,

    // The value is invalid.
    LB_READ_BUFFER_READ_INVALID_VALUE = 0x8,
} LB_ReadBufferReadError;

typedef struct LB_ReadBuffer {
    const void *data;
    size_t length;
    size_t position;
} LB_ReadBuffer;

/**
 * Initialize a LB_ReadBuffer and returns an error code, if any.                                <br>
 * Will not do error checking if LB_READ_BUFFER_NO_SAFETY is defined.
 *
 * @param buffer A pointer to the LB_ReadBuffer to be initialized.                              <br>
 * If NULL and LB_READ_BUFFER_SAFETY, will return LB_READ_BUFFER_INIT_NO_BUFFER.                <br>
 * @param data A pointer to the data to be written to.                                          <br>
 * If NULL and LB_READ_BUFFER_SAFETY, will return LB_READ_BUFFER_INIT_DATA_NULL.                <br>
 * @param length The length of the data to be written.                                          <br>
 * If 0 and LB_READ_BUFFER_SAFETY, will return LB_READ_BUFFER_INIT_LENGTH_ZERO.                 <br>
 * @return LB_ReadBufferInitError An error code indicating the result of the initialization.
 */
inline LB_ReadBufferInitError lbReadBufferInit(LB_ReadBuffer *buffer, const size_t length, const void *data) {
#ifdef LB_READ_BUFFER_SAFETY
    LB_ReadBufferInitError e = LB_READ_BUFFER_INIT_NONE;
    if (buffer == NULL) {
        e |= LB_READ_BUFFER_INIT_NO_BUFFER;
    }

    if (data == NULL) {
        e |= LB_READ_BUFFER_INIT_DATA_NULL;
    }

    if (length == 0) {
        e |= LB_READ_BUFFER_INIT_LENGTH_ZERO;
    }

    if (e) {
        return e;
    }
#endif

    buffer->data = data;
    buffer->length = length;
    buffer->position = 0;
    return LB_READ_BUFFER_INIT_NONE;
}

#ifdef LB_READ_BUFFER_SAFETY
inline LB_ReadBufferReadError lbReadBufferReadSafety(const LB_ReadBuffer *buffer, const size_t length, const void *out_value) {
    LB_ReadBufferReadError e = LB_READ_BUFFER_READ_NONE;
    if (buffer == NULL) {
        e |= LB_READ_BUFFER_READ_BUFFER_NULL;
    } else {
        if (buffer->position + length > buffer->length) {
            e |= LB_READ_BUFFER_READ_END;
        }

        if (buffer->data == NULL) {
            e |= LB_READ_BUFFER_READ_DATA_NULL;
        }
    }

    if (out_value == NULL) {
        e |= LB_READ_BUFFER_READ_INVALID_VALUE;
    }

    return e;
}

#else
#define lbReadBufferReadSafety(buffer, length, value) LB_READ_BUFFER_READ_NONE
#endif

inline LB_ReadBufferReadError lbReadBufferReadUnsafe(LB_ReadBuffer *buffer, const size_t length, void *out_value) {
    memcpy(out_value, (uint8_t *) buffer->data + buffer->position, length);
    buffer->position += length;
    return LB_READ_BUFFER_READ_NONE;
}

inline LB_ReadBufferReadError lbReadBufferRead(LB_ReadBuffer *buffer, const size_t length, void *out_value) {
#ifdef LB_READ_BUFFER_SAFETY
    const LB_ReadBufferReadError e = lbReadBufferReadSafety(buffer, length, out_value);
    if (e) {
        return e;
    }
#endif

    return lbReadBufferReadUnsafe(buffer, length, out_value);
}

inline LB_ReadBufferReadError lbReadBufferReadReversedUnsafe(LB_ReadBuffer *buffer, const size_t length,
                                                                 void *out_value) {
    for (size_t i = 0; i < length; i++) {
        *((uint8_t *) out_value + length - 1 - i) = *((uint8_t *) buffer->data + buffer->position + i);
    }

    buffer->position += length;
    return LB_READ_BUFFER_READ_NONE;
}

inline LB_ReadBufferReadError lbReadBufferReadReversed(LB_ReadBuffer *buffer, const size_t length, void *out_value) {
#ifdef LB_READ_BUFFER_SAFETY
    const LB_ReadBufferReadError e = lbReadBufferReadSafety(buffer, length, out_value);
    if (e) {
        return e;
    }
#endif

    return lbReadBufferReadReversedUnsafe(buffer, length, out_value);
}

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define lbReadBufferLE lbReadBufferRead
#define lbReadBufferBE lbReadBufferReadReversed
#define lbReadBufferUnsafeLE lbReadBufferReadUnsafe
#define lbReadBufferUnsafeBE lbReadBufferReadReversedUnsafe
#else
#define lbReadBufferLE lbReadBufferReadReversed
#define lbReadBufferBE lbReadBufferRead
#define lbReadBufferUnsafeLE lbReadBufferReadReversedUnsafe
#define lbReadBufferUnsafeBE lbReadBufferReadUnsafe
#endif

inline uint8_t lbReadBufferU8(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    uint8_t result;
    const LB_ReadBufferReadError error = lbReadBufferRead(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint8_t lbReadBufferU8LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    uint8_t result;
    const LB_ReadBufferReadError error = lbReadBufferLE(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint8_t lbReadBufferU8BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    uint8_t result;
    const LB_ReadBufferReadError error = lbReadBufferBE(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint16_t lbReadBufferU16(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    uint16_t result;
    const LB_ReadBufferReadError error = lbReadBufferRead(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint16_t lbReadBufferU16LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    uint16_t result;
    const LB_ReadBufferReadError error = lbReadBufferLE(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint16_t lbReadBufferU16BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    uint16_t result;
    const LB_ReadBufferReadError error = lbReadBufferBE(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint32_t lbReadBufferU32(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    uint32_t result;
    const LB_ReadBufferReadError error = lbReadBufferRead(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint32_t lbReadBufferU32LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    uint32_t result;
    const LB_ReadBufferReadError error = lbReadBufferLE(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint32_t lbReadBufferU32BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    uint32_t result;
    const LB_ReadBufferReadError error = lbReadBufferBE(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}


inline uint64_t lbReadBufferU64(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    uint64_t result;
    const LB_ReadBufferReadError error = lbReadBufferRead(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint64_t lbReadBufferU64LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    uint64_t result;
    const LB_ReadBufferReadError error = lbReadBufferLE(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint64_t lbReadBufferU64BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    uint64_t result;
    const LB_ReadBufferReadError error = lbReadBufferBE(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}


inline int8_t lbReadBufferI8(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    int8_t result;
    const LB_ReadBufferReadError error = lbReadBufferRead(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int8_t lbReadBufferI8LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    int8_t result;
    const LB_ReadBufferReadError error = lbReadBufferLE(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int8_t lbReadBufferI8BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    int8_t result;
    const LB_ReadBufferReadError error = lbReadBufferBE(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int16_t lbReadBufferI16(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    int16_t result;
    const LB_ReadBufferReadError error = lbReadBufferRead(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int16_t lbReadBufferI16LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    int16_t result;
    const LB_ReadBufferReadError error = lbReadBufferLE(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int16_t lbReadBufferI16BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    int16_t result;
    const LB_ReadBufferReadError error = lbReadBufferBE(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int32_t lbReadBufferI32(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    int32_t result;
    const LB_ReadBufferReadError error = lbReadBufferRead(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int32_t lbReadBufferI32LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    int32_t result;
    const LB_ReadBufferReadError error = lbReadBufferLE(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int32_t lbReadBufferI32BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    int32_t result;
    const LB_ReadBufferReadError error = lbReadBufferBE(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}


inline int64_t lbReadBufferI64(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    int64_t result;
    const LB_ReadBufferReadError error = lbReadBufferRead(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int64_t lbReadBufferI64LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    int64_t result;
    const LB_ReadBufferReadError error = lbReadBufferLE(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int64_t lbReadBufferI64BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    int64_t result;
    const LB_ReadBufferReadError error = lbReadBufferBE(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}


inline float lbReadBufferF32(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    float result;
    const LB_ReadBufferReadError error = lbReadBufferRead(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline float lbReadBufferF32LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    float result;
    const LB_ReadBufferReadError error = lbReadBufferLE(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline float lbReadBufferF32BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    float result;
    const LB_ReadBufferReadError error = lbReadBufferBE(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}


inline double lbReadBufferF64(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    double result;
    const LB_ReadBufferReadError error = lbReadBufferRead(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline double lbReadBufferF64LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    double result;
    const LB_ReadBufferReadError error = lbReadBufferLE(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline double lbReadBufferF64BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    double result;
    const LB_ReadBufferReadError error = lbReadBufferBE(buffer, sizeof(result), &result);
    #ifdef LB_READ_BUFFER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

/*
 * Normalized Integer Functions
 */

inline float lbReadBufferNU8(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    return (float) lbReadBufferU8(buffer, out_error) / (float) UINT8_MAX;
}

inline float lbReadBufferNU8LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    return lbReadBufferNU8(buffer, out_error);
}

inline float lbReadBufferNU8BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    return lbReadBufferNU8(buffer, out_error);
}


inline float lbReadBufferNU16(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    return (float) lbReadBufferU16(buffer, out_error) / (float) UINT16_MAX;
}

inline float lbReadBufferNU16LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    return (float) lbReadBufferU16LE(buffer, out_error) / (float) UINT16_MAX;
}

inline float lbReadBufferNU16BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    return (float) lbReadBufferU16BE(buffer, out_error) / (float) UINT16_MAX;
}

/*  Normalized integers utilizing more than 23 bits can carry more precision than a float.  */
inline double lbReadBufferNU32(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    return (double) lbReadBufferU32(buffer, out_error) / (double) UINT32_MAX;
}

inline double lbReadBufferNU32LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    return (double) lbReadBufferU32LE(buffer, out_error) / (double) UINT32_MAX;
}

inline double lbReadBufferNU32BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    return (double) lbReadBufferU32BE(buffer, out_error) / (double) UINT32_MAX;
}


inline double lbReadBufferNU64(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    return (double) lbReadBufferU64(buffer, out_error) / (double) UINT64_MAX;
}

inline double lbReadBufferNU64LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    return (double) lbReadBufferU64LE(buffer, out_error) / (double) UINT64_MAX;
}

inline double lbReadBufferNU64BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    return (double) lbReadBufferU64BE(buffer, out_error) / (double) UINT64_MAX;
}


inline float lbReadBufferNI8(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    return (float) lbReadBufferI8(buffer, out_error) / (float) INT8_MAX;
}

inline float lbReadBufferNI8LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    return lbReadBufferNI8(buffer, out_error);
}

inline float lbReadBufferNI8BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    return lbReadBufferNI8(buffer, out_error);
}


inline float lbReadBufferNI16(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    return (float) lbReadBufferI16(buffer, out_error) / (float) INT16_MAX;
}

inline float lbReadBufferNI16LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    return (float) lbReadBufferI16LE(buffer, out_error) / (float) INT16_MAX;
}

inline float lbReadBufferNI16BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    return (float) lbReadBufferI16BE(buffer, out_error) / (float) INT16_MAX;
}


inline double lbReadBufferNI32(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    return (double) lbReadBufferI32(buffer, out_error) / (double) INT32_MAX;
}

inline double lbReadBufferNI32LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    return (double) lbReadBufferI32LE(buffer, out_error) / (double) INT32_MAX;
}

inline double lbReadBufferNI32BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    return (double) lbReadBufferI32BE(buffer, out_error) / (double) INT32_MAX;
}


inline double lbReadBufferNI64(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    return (double) lbReadBufferI64(buffer, out_error) / (double) INT64_MAX;
}

inline double lbReadBufferNI64LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    return (double) lbReadBufferI64LE(buffer, out_error) / (double) INT64_MAX;
}

inline double lbReadBufferNI64BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error) {
    return (double) lbReadBufferI64BE(buffer, out_error) / (double) INT64_MAX;
}
#ifdef __cplusplus
}
#endif
#endif

#ifdef LB_READ_BUFFER_IMPLEMENTATION
#ifdef __cplusplus
extern "C" {
#endif
LB_ReadBufferInitError lbReadBufferInit(LB_ReadBuffer *buffer, size_t length, const void *data);
#ifdef LB_READ_BUFFER_SAFETY
LB_ReadBufferReadError lbReadBufferReadSafety(const LB_ReadBuffer *buffer, size_t length, const void* out_value);
#endif
LB_ReadBufferReadError lbReadBufferReadUnsafe(LB_ReadBuffer *buffer, size_t length, void* out_value);
LB_ReadBufferReadError lbReadBufferRead(LB_ReadBuffer *buffer, size_t length, void* out_value);
LB_ReadBufferReadError lbReadBufferReadReversedUnsafe(LB_ReadBuffer *buffer, size_t length, void* out_value);
LB_ReadBufferReadError lbReadBufferReadReversed(LB_ReadBuffer *buffer, size_t length, void* out_value);
uint8_t lbReadBufferU8(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
uint8_t lbReadBufferU8LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
uint8_t lbReadBufferU8BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
uint16_t lbReadBufferU16(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
uint16_t lbReadBufferU16LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
uint16_t lbReadBufferU16BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
uint32_t lbReadBufferU32(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
uint32_t lbReadBufferU32LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
uint32_t lbReadBufferU32BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
uint64_t lbReadBufferU64(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
uint64_t lbReadBufferU64LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
uint64_t lbReadBufferU64BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
int8_t lbReadBufferI8(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
int8_t lbReadBufferI8LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
int8_t lbReadBufferI8BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
int16_t lbReadBufferI16(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
int16_t lbReadBufferI16LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
int16_t lbReadBufferI16BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
int32_t lbReadBufferI32(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
int32_t lbReadBufferI32LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
int32_t lbReadBufferI32BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
int64_t lbReadBufferI64(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
int64_t lbReadBufferI64LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
int64_t lbReadBufferI64BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
float lbReadBufferF32(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
float lbReadBufferF32LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
float lbReadBufferF32BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
double lbReadBufferF64(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
double lbReadBufferF64LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
double lbReadBufferF64BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
float lbReadBufferNU8(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
float lbReadBufferNU8LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
float lbReadBufferNU8BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
float lbReadBufferNU16(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
float lbReadBufferNU16LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
float lbReadBufferNU16BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
double lbReadBufferNU32(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
double lbReadBufferNU32LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
double lbReadBufferNU32BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
double lbReadBufferNU64(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
double lbReadBufferNU64LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
double lbReadBufferNU64BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
float lbReadBufferNI8(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
float lbReadBufferNI8LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
float lbReadBufferNI8BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
float lbReadBufferNI16(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
float lbReadBufferNI16LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
float lbReadBufferNI16BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
double lbReadBufferNI32(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
double lbReadBufferNI32LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
double lbReadBufferNI32BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
double lbReadBufferNI64(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
double lbReadBufferNI64LE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
double lbReadBufferNI64BE(LB_ReadBuffer *buffer, LB_ReadBufferReadError* out_error);
#endif

#ifdef __cplusplus
}
#endif

