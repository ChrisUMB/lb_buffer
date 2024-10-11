#ifndef LB_READER_H
#define LB_READER_H

#ifdef __cplusplus
#include <cstring>
#include <cstdio>
#include <cstdint>
#include <cstdlib>
extern "C" {
#else
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#endif

#ifndef LB_READER_NO_SAFETY
#define LB_READER_SAFETY
#endif

typedef enum LB_ReaderMode {
    LB_READER_MODE_BUFFER = 0,
    LB_READER_MODE_FILE = 1,
} LB_ReaderMode;

// Error codes for initializing a LB_Reader.
typedef enum LB_ReaderInitError {
    // No error.
    LB_READER_INIT_NONE = 0x0,
    // The data is NULL.
    LB_READER_INIT_DATA_NULL = 0x1,
    // The file could not be opened.
    LB_READER_INIT_FILE_INVALID = 0x2,
    // The reader is NULL.
    LB_READER_INIT_NO_READER = 0x4,
    // The length is zero.
    LB_READER_INIT_LENGTH_ZERO = 0x8,
} LB_ReaderInitError;

// Error codes for `lbRead` methods.
typedef enum LB_ReaderReadError {
    // No error.
    LB_READER_READ_NONE = 0x0,
    // The reader is at the end.
    LB_READER_READ_END = 0x1,
    // The reader is NULL.
    LB_READER_READER_NULL = 0x2,
    // The data is NULL.
    LB_READER_READ_DATA_NULL = 0x4,

    // The value is invalid.
    LB_READER_READ_INVALID_VALUE = 0x8,
} LB_ReaderReadError;

typedef struct LB_Reader {
    LB_ReaderMode mode;
    const void *data;
    size_t length;
    size_t position;
} LB_Reader;

typedef struct LB_ReaderA {
    const LB_ReaderMode mode;
    union {
        struct {
            const void *data;
            size_t length;
            size_t position;
        } buffer;

        FILE* file;
    } _;
} LB_ReaderA;

/**
 * Initialize a LB_Reader and returns an error code, if any.                                <br>
 * Will not do error checking if LB_READER_NO_SAFETY is defined.
 *
 * @param reader A pointer to the LB_Reader to be initialized.                              <br>
 * If NULL and LB_READER_SAFETY, will return LB_READER_INIT_NO_READER.                <br>
 * @param data A pointer to the data to be written to.                                          <br>
 * If NULL and LB_READER_SAFETY, will return LB_READER_INIT_DATA_NULL.                <br>
 * @param length The length of the data to be written.                                          <br>
 * If 0 and LB_READER_SAFETY, will return LB_READER_INIT_LENGTH_ZERO.                 <br>
 * @return LB_ReaderInitError An error code indicating the result of the initialization.
 */
inline LB_ReaderInitError lbReadInitBuffer(LB_Reader *reader, const size_t length, const void *data) {
#ifdef LB_READER_SAFETY
    LB_ReaderInitError e = LB_READER_INIT_NONE;
    if (reader == NULL) {
        e |= LB_READER_INIT_NO_READER;
    }

    if (data == NULL) {
        e |= LB_READER_INIT_DATA_NULL;
    }

    if (length == 0) {
        e |= LB_READER_INIT_LENGTH_ZERO;
    }

    if (e) {
        return e;
    }
#endif

    reader->mode = LB_READER_MODE_BUFFER;
    reader->data = data;
    reader->length = length;
    reader->position = 0;
    return LB_READER_INIT_NONE;
}


inline LB_ReaderInitError lbReadInitFile(LB_Reader *reader, FILE* file) {
    if (file == NULL) {
        return LB_READER_INIT_DATA_NULL;
    }

    reader->mode = LB_READER_MODE_FILE;
    reader->data = file;
    reader->length = 0;
    reader->position = 0;
    return LB_READER_INIT_NONE;
}

#ifdef LB_READER_SAFETY
inline LB_ReaderReadError lbReadSafety(const LB_Reader *reader, const size_t length, const void *out_value) {
    LB_ReaderReadError e = LB_READER_READ_NONE;
    if (reader == NULL) {
        e |= LB_READER_READER_NULL;
    } else {
        if (reader->mode == LB_READER_MODE_BUFFER && reader->position + length > reader->length) {
            e |= LB_READER_READ_END;
        }

        if (reader->data == NULL) {
            e |= LB_READER_READ_DATA_NULL;
        }
    }

    if (out_value == NULL) {
        e |= LB_READER_READ_INVALID_VALUE;
    }

    return e;
}

#else
#define lbReadSafety(reader, length, value) LB_READER_READ_NONE
#endif

inline LB_ReaderReadError lbReadUnsafe(LB_Reader *reader, const size_t length, void *out_value) {
    if(reader->mode == LB_READER_MODE_BUFFER) {
        memcpy(out_value, (uint8_t *) reader->data + reader->position, length);
        reader->position += length;
        return LB_READER_READ_NONE;
    }

    FILE *file = (FILE *) reader->data;
    if (fread(out_value, length, 1, file) != 1) {
        return LB_READER_READ_END;
    }

    return LB_READER_READ_NONE;
}

inline LB_ReaderReadError lbReaderSeek(LB_Reader *reader, const size_t position) {
    if (reader->mode == LB_READER_MODE_BUFFER) {
        if(position >= reader->length) {
            return LB_READER_READ_END;
        }

        reader->position = position;
        return LB_READER_READ_NONE;
    }

    FILE *file = (FILE *) reader->data;
    if (fseek(file, position, SEEK_SET) != 0) {
        return LB_READER_READ_END;
    }

    return LB_READER_READ_NONE;
}

inline size_t lbReaderTell(const LB_Reader *reader) {
    if (reader->mode == LB_READER_MODE_BUFFER) {
        return reader->position;
    }

    FILE *file = (FILE *) reader->data;
    return ftell(file);
}

inline LB_ReaderReadError lbRead(LB_Reader *reader, const size_t length, void *out_value) {
#ifdef LB_READER_SAFETY
    const LB_ReaderReadError e = lbReadSafety(reader, length, out_value);
    if (e) {
        return e;
    }
#endif

    return lbReadUnsafe(reader, length, out_value);
}

inline LB_ReaderReadError lbReadReversedUnsafe(LB_Reader *reader, const size_t length, void *out_value) {
    if(reader->mode == LB_READER_MODE_BUFFER) {
        for (size_t i = 0; i < length; i++) {
            *((uint8_t *) out_value + length - 1 - i) = *((uint8_t *) reader->data + reader->position + i);
        }

        reader->position += length;
        return LB_READER_READ_NONE;
    }

    if(fread(out_value, length, 1, (FILE*) reader->data) != 1) {
        return LB_READER_READ_END;
    }

    // Reverse the bytes.
    for (size_t i = 0; i < length / 2; i++) {
        const uint8_t temp = *((uint8_t *) out_value + i);
        *((uint8_t *) out_value + i) = *((uint8_t *) out_value + length - 1 - i);
        *((uint8_t *) out_value + length - 1 - i) = temp;
    }

    return LB_READER_READ_NONE;
}

inline LB_ReaderReadError lbReadReversed(LB_Reader *reader, const size_t length, void *out_value) {
#ifdef LB_READER_SAFETY
    const LB_ReaderReadError e = lbReadSafety(reader, length, out_value);
    if (e) {
        return e;
    }
#endif

    return lbReadReversedUnsafe(reader, length, out_value);
}

inline size_t lbReadRemaining(const LB_Reader *reader) {
    return reader->length - reader->position;
}

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define lbReadLE lbRead
#define lbReadBE lbReadReversed
#define lbReadUnsafeLE lbReadUnsafe
#define lbReadUnsafeBE lbReadReversedUnsafe
#else
#define lbReadLE lbReadReversed
#define lbReadBE lbRead
#define lbReadUnsafeLE lbReadReversedUnsafe
#define lbReadUnsafeBE lbReadUnsafe
#endif

inline uint8_t lbReadU8(LB_Reader *reader, LB_ReaderReadError* out_error) {
    uint8_t result;
    const LB_ReaderReadError error = lbRead(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint8_t lbReadU8LE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    uint8_t result;
    const LB_ReaderReadError error = lbReadLE(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint8_t lbReadU8BE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    uint8_t result;
    const LB_ReaderReadError error = lbReadBE(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint16_t lbReadU16(LB_Reader *reader, LB_ReaderReadError* out_error) {
    uint16_t result;
    const LB_ReaderReadError error = lbRead(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint16_t lbReadU16LE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    uint16_t result;
    const LB_ReaderReadError error = lbReadLE(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint16_t lbReadU16BE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    uint16_t result;
    const LB_ReaderReadError error = lbReadBE(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint32_t lbReadU32(LB_Reader *reader, LB_ReaderReadError* out_error) {
    uint32_t result;
    const LB_ReaderReadError error = lbRead(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint32_t lbReadU32LE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    uint32_t result;
    const LB_ReaderReadError error = lbReadLE(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint32_t lbReadU32BE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    uint32_t result;
    const LB_ReaderReadError error = lbReadBE(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}


inline uint64_t lbReadU64(LB_Reader *reader, LB_ReaderReadError* out_error) {
    uint64_t result;
    const LB_ReaderReadError error = lbRead(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint64_t lbReadU64LE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    uint64_t result;
    const LB_ReaderReadError error = lbReadLE(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint64_t lbReadU64BE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    uint64_t result;
    const LB_ReaderReadError error = lbReadBE(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}


inline int8_t lbReadI8(LB_Reader *reader, LB_ReaderReadError* out_error) {
    int8_t result;
    const LB_ReaderReadError error = lbRead(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int8_t lbReadI8LE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    int8_t result;
    const LB_ReaderReadError error = lbReadLE(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int8_t lbReadI8BE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    int8_t result;
    const LB_ReaderReadError error = lbReadBE(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int16_t lbReadI16(LB_Reader *reader, LB_ReaderReadError* out_error) {
    int16_t result;
    const LB_ReaderReadError error = lbRead(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int16_t lbReadI16LE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    int16_t result;
    const LB_ReaderReadError error = lbReadLE(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int16_t lbReadI16BE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    int16_t result;
    const LB_ReaderReadError error = lbReadBE(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int32_t lbReadI32(LB_Reader *reader, LB_ReaderReadError* out_error) {
    int32_t result;
    const LB_ReaderReadError error = lbRead(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int32_t lbReadI32LE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    int32_t result;
    const LB_ReaderReadError error = lbReadLE(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int32_t lbReadI32BE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    int32_t result;
    const LB_ReaderReadError error = lbReadBE(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}


inline int64_t lbReadI64(LB_Reader *reader, LB_ReaderReadError* out_error) {
    int64_t result;
    const LB_ReaderReadError error = lbRead(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int64_t lbReadI64LE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    int64_t result;
    const LB_ReaderReadError error = lbReadLE(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int64_t lbReadI64BE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    int64_t result;
    const LB_ReaderReadError error = lbReadBE(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}


inline float lbReadF32(LB_Reader *reader, LB_ReaderReadError* out_error) {
    float result;
    const LB_ReaderReadError error = lbRead(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline float lbReadF32LE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    float result;
    const LB_ReaderReadError error = lbReadLE(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline float lbReadF32BE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    float result;
    const LB_ReaderReadError error = lbReadBE(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}


inline double lbReadF64(LB_Reader *reader, LB_ReaderReadError* out_error) {
    double result;
    const LB_ReaderReadError error = lbRead(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline double lbReadF64LE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    double result;
    const LB_ReaderReadError error = lbReadLE(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline double lbReadF64BE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    double result;
    const LB_ReaderReadError error = lbReadBE(reader, sizeof(result), &result);
    #ifdef LB_READER_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

/*
 * Normalized Integer Functions
 */

inline float lbReadNU8(LB_Reader *reader, LB_ReaderReadError* out_error) {
    return (float) lbReadU8(reader, out_error) / (float) UINT8_MAX;
}

inline float lbReadNU8LE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    return lbReadNU8(reader, out_error);
}

inline float lbReadNU8BE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    return lbReadNU8(reader, out_error);
}


inline float lbReadNU16(LB_Reader *reader, LB_ReaderReadError* out_error) {
    return (float) lbReadU16(reader, out_error) / (float) UINT16_MAX;
}

inline float lbReadNU16LE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    return (float) lbReadU16LE(reader, out_error) / (float) UINT16_MAX;
}

inline float lbReadNU16BE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    return (float) lbReadU16BE(reader, out_error) / (float) UINT16_MAX;
}

/*  Normalized integers utilizing more than 23 bits can carry more precision than a float.  */
inline double lbReadNU32(LB_Reader *reader, LB_ReaderReadError* out_error) {
    return (double) lbReadU32(reader, out_error) / (double) UINT32_MAX;
}

inline double lbReadNU32LE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    return (double) lbReadU32LE(reader, out_error) / (double) UINT32_MAX;
}

inline double lbReadNU32BE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    return (double) lbReadU32BE(reader, out_error) / (double) UINT32_MAX;
}


inline double lbReadNU64(LB_Reader *reader, LB_ReaderReadError* out_error) {
    return (double) lbReadU64(reader, out_error) / (double) UINT64_MAX;
}

inline double lbReadNU64LE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    return (double) lbReadU64LE(reader, out_error) / (double) UINT64_MAX;
}

inline double lbReadNU64BE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    return (double) lbReadU64BE(reader, out_error) / (double) UINT64_MAX;
}


inline float lbReadNI8(LB_Reader *reader, LB_ReaderReadError* out_error) {
    return (float) lbReadI8(reader, out_error) / (float) INT8_MAX;
}

inline float lbReadNI8LE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    return lbReadNI8(reader, out_error);
}

inline float lbReadNI8BE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    return lbReadNI8(reader, out_error);
}


inline float lbReadNI16(LB_Reader *reader, LB_ReaderReadError* out_error) {
    return (float) lbReadI16(reader, out_error) / (float) INT16_MAX;
}

inline float lbReadNI16LE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    return (float) lbReadI16LE(reader, out_error) / (float) INT16_MAX;
}

inline float lbReadNI16BE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    return (float) lbReadI16BE(reader, out_error) / (float) INT16_MAX;
}


inline double lbReadNI32(LB_Reader *reader, LB_ReaderReadError* out_error) {
    return (double) lbReadI32(reader, out_error) / (double) INT32_MAX;
}

inline double lbReadNI32LE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    return (double) lbReadI32LE(reader, out_error) / (double) INT32_MAX;
}

inline double lbReadNI32BE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    return (double) lbReadI32BE(reader, out_error) / (double) INT32_MAX;
}


inline double lbReadNI64(LB_Reader *reader, LB_ReaderReadError* out_error) {
    return (double) lbReadI64(reader, out_error) / (double) INT64_MAX;
}

inline double lbReadNI64LE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    return (double) lbReadI64LE(reader, out_error) / (double) INT64_MAX;
}

inline double lbReadNI64BE(LB_Reader *reader, LB_ReaderReadError* out_error) {
    return (double) lbReadI64BE(reader, out_error) / (double) INT64_MAX;
}
#ifdef __cplusplus
}
#endif
#endif

#ifdef LB_READER_IMPLEMENTATION
#ifdef __cplusplus
extern "C" {
#endif
LB_ReaderInitError lbReadInitBuffer(LB_Reader *reader, size_t length, const void *data);
LB_ReaderInitError lbReadInitFile(LB_Reader *reader, FILE* file);
#ifdef LB_READER_SAFETY
LB_ReaderReadError lbReadSafety(const LB_Reader *reader, size_t length, const void* out_value);
#endif
LB_ReaderReadError lbReadUnsafe(LB_Reader *reader, size_t length, void* out_value);
LB_ReaderReadError lbRead(LB_Reader *reader, size_t length, void* out_value);
LB_ReaderReadError lbReadReversedUnsafe(LB_Reader *reader, size_t length, void* out_value);
LB_ReaderReadError lbReadReversed(LB_Reader *reader, size_t length, void* out_value);
size_t lbReadRemaining(const LB_Reader *reader);
LB_ReaderReadError lbReaderSeek(LB_Reader *reader, size_t position);
size_t lbReaderTell(const LB_Reader *reader);

uint8_t lbReadU8(LB_Reader *reader, LB_ReaderReadError* out_error);
uint8_t lbReadU8LE(LB_Reader *reader, LB_ReaderReadError* out_error);
uint8_t lbReadU8BE(LB_Reader *reader, LB_ReaderReadError* out_error);
uint16_t lbReadU16(LB_Reader *reader, LB_ReaderReadError* out_error);
uint16_t lbReadU16LE(LB_Reader *reader, LB_ReaderReadError* out_error);
uint16_t lbReadU16BE(LB_Reader *reader, LB_ReaderReadError* out_error);
uint32_t lbReadU32(LB_Reader *reader, LB_ReaderReadError* out_error);
uint32_t lbReadU32LE(LB_Reader *reader, LB_ReaderReadError* out_error);
uint32_t lbReadU32BE(LB_Reader *reader, LB_ReaderReadError* out_error);
uint64_t lbReadU64(LB_Reader *reader, LB_ReaderReadError* out_error);
uint64_t lbReadU64LE(LB_Reader *reader, LB_ReaderReadError* out_error);
uint64_t lbReadU64BE(LB_Reader *reader, LB_ReaderReadError* out_error);
int8_t lbReadI8(LB_Reader *reader, LB_ReaderReadError* out_error);
int8_t lbReadI8LE(LB_Reader *reader, LB_ReaderReadError* out_error);
int8_t lbReadI8BE(LB_Reader *reader, LB_ReaderReadError* out_error);
int16_t lbReadI16(LB_Reader *reader, LB_ReaderReadError* out_error);
int16_t lbReadI16LE(LB_Reader *reader, LB_ReaderReadError* out_error);
int16_t lbReadI16BE(LB_Reader *reader, LB_ReaderReadError* out_error);
int32_t lbReadI32(LB_Reader *reader, LB_ReaderReadError* out_error);
int32_t lbReadI32LE(LB_Reader *reader, LB_ReaderReadError* out_error);
int32_t lbReadI32BE(LB_Reader *reader, LB_ReaderReadError* out_error);
int64_t lbReadI64(LB_Reader *reader, LB_ReaderReadError* out_error);
int64_t lbReadI64LE(LB_Reader *reader, LB_ReaderReadError* out_error);
int64_t lbReadI64BE(LB_Reader *reader, LB_ReaderReadError* out_error);
float lbReadF32(LB_Reader *reader, LB_ReaderReadError* out_error);
float lbReadF32LE(LB_Reader *reader, LB_ReaderReadError* out_error);
float lbReadF32BE(LB_Reader *reader, LB_ReaderReadError* out_error);
double lbReadF64(LB_Reader *reader, LB_ReaderReadError* out_error);
double lbReadF64LE(LB_Reader *reader, LB_ReaderReadError* out_error);
double lbReadF64BE(LB_Reader *reader, LB_ReaderReadError* out_error);
float lbReadNU8(LB_Reader *reader, LB_ReaderReadError* out_error);
float lbReadNU8LE(LB_Reader *reader, LB_ReaderReadError* out_error);
float lbReadNU8BE(LB_Reader *reader, LB_ReaderReadError* out_error);
float lbReadNU16(LB_Reader *reader, LB_ReaderReadError* out_error);
float lbReadNU16LE(LB_Reader *reader, LB_ReaderReadError* out_error);
float lbReadNU16BE(LB_Reader *reader, LB_ReaderReadError* out_error);
double lbReadNU32(LB_Reader *reader, LB_ReaderReadError* out_error);
double lbReadNU32LE(LB_Reader *reader, LB_ReaderReadError* out_error);
double lbReadNU32BE(LB_Reader *reader, LB_ReaderReadError* out_error);
double lbReadNU64(LB_Reader *reader, LB_ReaderReadError* out_error);
double lbReadNU64LE(LB_Reader *reader, LB_ReaderReadError* out_error);
double lbReadNU64BE(LB_Reader *reader, LB_ReaderReadError* out_error);
float lbReadNI8(LB_Reader *reader, LB_ReaderReadError* out_error);
float lbReadNI8LE(LB_Reader *reader, LB_ReaderReadError* out_error);
float lbReadNI8BE(LB_Reader *reader, LB_ReaderReadError* out_error);
float lbReadNI16(LB_Reader *reader, LB_ReaderReadError* out_error);
float lbReadNI16LE(LB_Reader *reader, LB_ReaderReadError* out_error);
float lbReadNI16BE(LB_Reader *reader, LB_ReaderReadError* out_error);
double lbReadNI32(LB_Reader *reader, LB_ReaderReadError* out_error);
double lbReadNI32LE(LB_Reader *reader, LB_ReaderReadError* out_error);
double lbReadNI32BE(LB_Reader *reader, LB_ReaderReadError* out_error);
double lbReadNI64(LB_Reader *reader, LB_ReaderReadError* out_error);
double lbReadNI64LE(LB_Reader *reader, LB_ReaderReadError* out_error);
double lbReadNI64BE(LB_Reader *reader, LB_ReaderReadError* out_error);
#endif

#ifdef __cplusplus
}
#endif

