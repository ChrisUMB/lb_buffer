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

inline const char* lbReaderInitErrorName(const LB_ReaderInitError error) {
    switch (error) {
        case LB_READER_INIT_NONE:
            return "LB_READER_INIT_NONE";
        case LB_READER_INIT_DATA_NULL:
            return "LB_READER_INIT_DATA_NULL";
        case LB_READER_INIT_FILE_INVALID:
            return "LB_READER_INIT_FILE_INVALID";
        case LB_READER_INIT_NO_READER:
            return "LB_READER_INIT_NO_READER";
        case LB_READER_INIT_LENGTH_ZERO:
            return "LB_READER_INIT_LENGTH_ZERO";
        default:
            return "UNKNOWN";
    }
}

inline const char* lbReaderInitErrorMessage(const LB_ReaderInitError error) {
    switch (error) {
        case LB_READER_INIT_NONE:
            return "No error.";
        case LB_READER_INIT_DATA_NULL:
            return "The data is NULL.";
        case LB_READER_INIT_FILE_INVALID:
            return "The file could not be opened.";
        case LB_READER_INIT_NO_READER:
            return "The reader is NULL.";
        case LB_READER_INIT_LENGTH_ZERO:
            return "The length is zero.";
        default:
            return "An unknown error occurred.";
    }
}

// Error codes for `lbRead` methods.
typedef enum LB_ReaderError {
    // No error.
    LB_READER_ERROR_NONE = 0x0,
    // The reader is at the end.
    LB_READER_ERROR_END = 0x1,
    // The reader is NULL.
    LB_READER_ERROR_READER_NULL = 0x2,
    // The data is NULL.
    LB_READER_ERROR_DATA_NULL = 0x4,

    // The value is invalid.
    LB_READER_ERROR_INVALID_VALUE = 0x8,
} LB_ReaderError;

inline const char* lbReaderErrorName(const LB_ReaderError error) {
    switch (error) {
        case LB_READER_ERROR_NONE:
            return "LB_READER_ERROR_NONE";
        case LB_READER_ERROR_END:
            return "LB_READER_ERROR_END";
        case LB_READER_ERROR_READER_NULL:
            return "LB_READER_ERROR_READER_NULL";
        case LB_READER_ERROR_DATA_NULL:
            return "LB_READER_ERROR_DATA_NULL";
        case LB_READER_ERROR_INVALID_VALUE:
            return "LB_READER_ERROR_INVALID_VALUE";
        default:
            return "UNKNOWN";
    }
}

inline const char* lbReaderErrorMessage(const LB_ReaderError error) {
    switch (error) {
        case LB_READER_ERROR_NONE:
            return "No error.";
        case LB_READER_ERROR_END:
            return "The reader is at the end.";
        case LB_READER_ERROR_READER_NULL:
            return "The reader is NULL.";
        case LB_READER_ERROR_DATA_NULL:
            return "The data is NULL.";
        case LB_READER_ERROR_INVALID_VALUE:
            return "The value is invalid.";
        default:
            return "An unknown error occurred.";
    }
}

// typedef struct LB_Reader {
//     LB_ReaderMode mode;
//     const void *data;
//     size_t length;
//     size_t position;
// } LB_Reader;

typedef struct LB_ReaderBuffer {
    const void *data;
    size_t length;
    size_t position;
} LB_ReaderBuffer;

typedef struct LB_Reader {
    struct {
        LB_ReaderMode mode;

        union {
            LB_ReaderBuffer buffer;
            FILE *file;
        };
    } _;
} LB_Reader;

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
inline LB_ReaderInitError lbReaderInitBuffer(LB_Reader *reader, const void *data, const size_t length) {
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

    // ReSharper disable once CppDFANullDereference // It's not null, or if it is and safety is off, just crash.
    *reader = (LB_Reader){
        ._ = {
            .mode = LB_READER_MODE_BUFFER,
            .buffer = {
                .data = data,
                .length = length,
                .position = 0,
            }
        }
    };
    return LB_READER_INIT_NONE;
}


inline LB_ReaderInitError lbReaderInitFile(LB_Reader *reader, FILE *file) {
    if (file == NULL) {
        return LB_READER_INIT_DATA_NULL;
    }

    *reader = (LB_Reader){
        ._ = {
            .mode = LB_READER_MODE_FILE,
            .file = file
        }
    };
    return LB_READER_INIT_NONE;
}

#ifdef LB_READER_SAFETY
inline LB_ReaderError lbReaderCheckSafety(const LB_Reader *reader, const void *out_value, const size_t length) {
    LB_ReaderError e = LB_READER_ERROR_NONE;
    if (reader == NULL) {
        e |= LB_READER_ERROR_READER_NULL;
    } else if (reader->_.mode == LB_READER_MODE_BUFFER) {
        const LB_ReaderBuffer *buffer = &reader->_.buffer;
        if (buffer->position + length > buffer->length) {
            e |= LB_READER_ERROR_END;
        }

        if (buffer->data == NULL) {
            e |= LB_READER_ERROR_DATA_NULL;
        }
    } else if (reader->_.file == NULL) {
        e |= LB_READER_ERROR_DATA_NULL;
    }

    if (out_value == NULL) {
        e |= LB_READER_ERROR_INVALID_VALUE;
    }

    return e;
}

#else
#define lbReadSafety(reader, length, value) LB_READER_READ_NONE
#endif

inline LB_ReaderMode lbReaderGetMode(const LB_Reader *reader) {
    return reader->_.mode;
}

inline LB_ReaderError lbReaderSeek(LB_Reader *reader, const size_t position) {
    if (reader->_.mode == LB_READER_MODE_BUFFER) {
        LB_ReaderBuffer *buffer = &reader->_.buffer;
        if (position >= buffer->length) {
            return LB_READER_ERROR_END;
        }

        buffer->position = position;
        return LB_READER_ERROR_NONE;
    }

    FILE *file = reader->_.file;
    if (fseek(file, position, SEEK_SET) != 0) {
        return LB_READER_ERROR_END;
    }

    return LB_READER_ERROR_NONE;
}

#define lbReaderTell lbReaderPosition

inline size_t lbReaderPosition(const LB_Reader *reader) {
    if (reader->_.mode == LB_READER_MODE_BUFFER) {
        return reader->_.buffer.position;
    }

    FILE *file = reader->_.file;
#ifdef _LARGEFILE64_SOURCE
#ifdef _MSC_VER
    return _ftelli64(file);
#else
    return ftello64(file);
#endif
#else
    return ftell(file);
#endif
}

inline size_t lbReaderLength(const LB_Reader *reader) {
    if (reader->_.mode == LB_READER_MODE_BUFFER) {
        return reader->_.buffer.length;
    }

    FILE *file = reader->_.file;
#ifdef _LARGEFILE64_SOURCE
#ifdef _MSC_VER
    const long long position = _ftelli64(file);
    _fseeki64(file, 0, SEEK_END);
    const long long end = _ftelli64(file);
    _fseeki64(file, position, SEEK_SET);
    return end;
#else
    const off64_t position = ftello64(file);
    fseeko64(file, 0, SEEK_END);
    const off64_t end = ftello64(file);
    fseeko64(file, position, SEEK_SET);
    return end;
#endif
#else
    const long position = ftell(file);
    fseek(file, 0, SEEK_END);
    const long end = ftell(file);
    fseek(file, position, SEEK_SET);
    return end;
#endif
}

inline size_t lbReaderRemaining(const LB_Reader *reader) {
    return lbReaderLength(reader) - lbReaderPosition(reader);
}

inline LB_ReaderError lbReadUnsafe(LB_Reader *reader, void *out_value, const size_t length) {
    if (reader->_.mode == LB_READER_MODE_BUFFER) {
        LB_ReaderBuffer *buffer = &reader->_.buffer;
        memcpy(out_value, (uint8_t *) buffer->data + buffer->position, length);
        buffer->position += length;
        return LB_READER_ERROR_NONE;
    }

    FILE *file = reader->_.file;
    if (fread(out_value, length, 1, file) != 1) {
        return LB_READER_ERROR_END;
    }

    return LB_READER_ERROR_NONE;
}


inline LB_ReaderError lbRead(LB_Reader *reader, void *out_value, const size_t length) {
#ifdef LB_READER_SAFETY
    const LB_ReaderError e = lbReaderCheckSafety(reader, out_value, length);
    if (e) {
        return e;
    }
#endif

    return lbReadUnsafe(reader, out_value, length);
}

inline LB_ReaderError lbReadReversedUnsafe(LB_Reader *reader, void *out_value, const size_t length) {
    if (reader->_.mode == LB_READER_MODE_BUFFER) {
        LB_ReaderBuffer *buffer = &reader->_.buffer;
        for (size_t i = 0; i < length; i++) {
            *((uint8_t *) out_value + length - 1 - i) = *((uint8_t *) buffer->data + buffer->position + i);
        }

        buffer->position += length;
        return LB_READER_ERROR_NONE;
    }

    if (fread(out_value, length, 1, reader->_.file) != 1) {
        return LB_READER_ERROR_END;
    }

    // Reverse the bytes.
    for (size_t i = 0; i < length / 2; i++) {
        const uint8_t temp = *((uint8_t *) out_value + i);
        *((uint8_t *) out_value + i) = *((uint8_t *) out_value + length - 1 - i);
        *((uint8_t *) out_value + length - 1 - i) = temp;
    }

    return LB_READER_ERROR_NONE;
}

inline LB_ReaderError lbReadReversed(LB_Reader *reader, void *out_value, const size_t length) {
#ifdef LB_READER_SAFETY
    const LB_ReaderError e = lbReaderCheckSafety(reader, out_value, length);
    if (e) {
        return e;
    }
#endif

    return lbReadReversedUnsafe(reader, out_value, length);
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

inline uint8_t lbReadU8(LB_Reader *reader, LB_ReaderError *out_error) {
    uint8_t result;
    const LB_ReaderError error = lbRead(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}

inline uint8_t lbReadU8LE(LB_Reader *reader, LB_ReaderError *out_error) {
    uint8_t result;
    const LB_ReaderError error = lbReadLE(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}

inline uint8_t lbReadU8BE(LB_Reader *reader, LB_ReaderError *out_error) {
    uint8_t result;
    const LB_ReaderError error = lbReadBE(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}

inline uint16_t lbReadU16(LB_Reader *reader, LB_ReaderError *out_error) {
    uint16_t result;
    const LB_ReaderError error = lbRead(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}

inline uint16_t lbReadU16LE(LB_Reader *reader, LB_ReaderError *out_error) {
    uint16_t result;
    const LB_ReaderError error = lbReadLE(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}

inline uint16_t lbReadU16BE(LB_Reader *reader, LB_ReaderError *out_error) {
    uint16_t result;
    const LB_ReaderError error = lbReadBE(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}

inline uint32_t lbReadU32(LB_Reader *reader, LB_ReaderError *out_error) {
    uint32_t result;
    const LB_ReaderError error = lbRead(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}

inline uint32_t lbReadU32LE(LB_Reader *reader, LB_ReaderError *out_error) {
    uint32_t result;
    const LB_ReaderError error = lbReadLE(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}

inline uint32_t lbReadU32BE(LB_Reader *reader, LB_ReaderError *out_error) {
    uint32_t result;
    const LB_ReaderError error = lbReadBE(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}


inline uint64_t lbReadU64(LB_Reader *reader, LB_ReaderError *out_error) {
    uint64_t result;
    const LB_ReaderError error = lbRead(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}

inline uint64_t lbReadU64LE(LB_Reader *reader, LB_ReaderError *out_error) {
    uint64_t result;
    const LB_ReaderError error = lbReadLE(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}

inline uint64_t lbReadU64BE(LB_Reader *reader, LB_ReaderError *out_error) {
    uint64_t result;
    const LB_ReaderError error = lbReadBE(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}


inline int8_t lbReadI8(LB_Reader *reader, LB_ReaderError *out_error) {
    int8_t result;
    const LB_ReaderError error = lbRead(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}

inline int8_t lbReadI8LE(LB_Reader *reader, LB_ReaderError *out_error) {
    int8_t result;
    const LB_ReaderError error = lbReadLE(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}

inline int8_t lbReadI8BE(LB_Reader *reader, LB_ReaderError *out_error) {
    int8_t result;
    const LB_ReaderError error = lbReadBE(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}

inline int16_t lbReadI16(LB_Reader *reader, LB_ReaderError *out_error) {
    int16_t result;
    const LB_ReaderError error = lbRead(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}

inline int16_t lbReadI16LE(LB_Reader *reader, LB_ReaderError *out_error) {
    int16_t result;
    const LB_ReaderError error = lbReadLE(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}

inline int16_t lbReadI16BE(LB_Reader *reader, LB_ReaderError *out_error) {
    int16_t result;
    const LB_ReaderError error = lbReadBE(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}

inline int32_t lbReadI32(LB_Reader *reader, LB_ReaderError *out_error) {
    int32_t result;
    const LB_ReaderError error = lbRead(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}

inline int32_t lbReadI32LE(LB_Reader *reader, LB_ReaderError *out_error) {
    int32_t result;
    const LB_ReaderError error = lbReadLE(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}

inline int32_t lbReadI32BE(LB_Reader *reader, LB_ReaderError *out_error) {
    int32_t result;
    const LB_ReaderError error = lbReadBE(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}


inline int64_t lbReadI64(LB_Reader *reader, LB_ReaderError *out_error) {
    int64_t result;
    const LB_ReaderError error = lbRead(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}

inline int64_t lbReadI64LE(LB_Reader *reader, LB_ReaderError *out_error) {
    int64_t result;
    const LB_ReaderError error = lbReadLE(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}

inline int64_t lbReadI64BE(LB_Reader *reader, LB_ReaderError *out_error) {
    int64_t result;
    const LB_ReaderError error = lbReadBE(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}


inline float lbReadF32(LB_Reader *reader, LB_ReaderError *out_error) {
    float result;
    const LB_ReaderError error = lbRead(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}

inline float lbReadF32LE(LB_Reader *reader, LB_ReaderError *out_error) {
    float result;
    const LB_ReaderError error = lbReadLE(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}

inline float lbReadF32BE(LB_Reader *reader, LB_ReaderError *out_error) {
    float result;
    const LB_ReaderError error = lbReadBE(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}


inline double lbReadF64(LB_Reader *reader, LB_ReaderError *out_error) {
    double result;
    const LB_ReaderError error = lbRead(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}

inline double lbReadF64LE(LB_Reader *reader, LB_ReaderError *out_error) {
    double result;
    const LB_ReaderError error = lbReadLE(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}

inline double lbReadF64BE(LB_Reader *reader, LB_ReaderError *out_error) {
    double result;
    const LB_ReaderError error = lbReadBE(reader, &result, sizeof(result));
#ifdef LB_READER_SAFETY
    if (out_error != NULL) {
        *out_error = error;
    }
#endif
    return result;
}

/*
 * Normalized Integer Functions
 */

inline float lbReadNU8(LB_Reader *reader, LB_ReaderError *out_error) {
    return (float) lbReadU8(reader, out_error) / (float) UINT8_MAX;
}

inline float lbReadNU8LE(LB_Reader *reader, LB_ReaderError *out_error) {
    return lbReadNU8(reader, out_error);
}

inline float lbReadNU8BE(LB_Reader *reader, LB_ReaderError *out_error) {
    return lbReadNU8(reader, out_error);
}


inline float lbReadNU16(LB_Reader *reader, LB_ReaderError *out_error) {
    return (float) lbReadU16(reader, out_error) / (float) UINT16_MAX;
}

inline float lbReadNU16LE(LB_Reader *reader, LB_ReaderError *out_error) {
    return (float) lbReadU16LE(reader, out_error) / (float) UINT16_MAX;
}

inline float lbReadNU16BE(LB_Reader *reader, LB_ReaderError *out_error) {
    return (float) lbReadU16BE(reader, out_error) / (float) UINT16_MAX;
}

/*  Normalized integers utilizing more than 23 bits can carry more precision than a float.  */
inline double lbReadNU32(LB_Reader *reader, LB_ReaderError *out_error) {
    return (double) lbReadU32(reader, out_error) / (double) UINT32_MAX;
}

inline double lbReadNU32LE(LB_Reader *reader, LB_ReaderError *out_error) {
    return (double) lbReadU32LE(reader, out_error) / (double) UINT32_MAX;
}

inline double lbReadNU32BE(LB_Reader *reader, LB_ReaderError *out_error) {
    return (double) lbReadU32BE(reader, out_error) / (double) UINT32_MAX;
}


inline double lbReadNU64(LB_Reader *reader, LB_ReaderError *out_error) {
    return (double) lbReadU64(reader, out_error) / (double) UINT64_MAX;
}

inline double lbReadNU64LE(LB_Reader *reader, LB_ReaderError *out_error) {
    return (double) lbReadU64LE(reader, out_error) / (double) UINT64_MAX;
}

inline double lbReadNU64BE(LB_Reader *reader, LB_ReaderError *out_error) {
    return (double) lbReadU64BE(reader, out_error) / (double) UINT64_MAX;
}


inline float lbReadNI8(LB_Reader *reader, LB_ReaderError *out_error) {
    return (float) lbReadI8(reader, out_error) / (float) INT8_MAX;
}

inline float lbReadNI8LE(LB_Reader *reader, LB_ReaderError *out_error) {
    return lbReadNI8(reader, out_error);
}

inline float lbReadNI8BE(LB_Reader *reader, LB_ReaderError *out_error) {
    return lbReadNI8(reader, out_error);
}


inline float lbReadNI16(LB_Reader *reader, LB_ReaderError *out_error) {
    return (float) lbReadI16(reader, out_error) / (float) INT16_MAX;
}

inline float lbReadNI16LE(LB_Reader *reader, LB_ReaderError *out_error) {
    return (float) lbReadI16LE(reader, out_error) / (float) INT16_MAX;
}

inline float lbReadNI16BE(LB_Reader *reader, LB_ReaderError *out_error) {
    return (float) lbReadI16BE(reader, out_error) / (float) INT16_MAX;
}


inline double lbReadNI32(LB_Reader *reader, LB_ReaderError *out_error) {
    return (double) lbReadI32(reader, out_error) / (double) INT32_MAX;
}

inline double lbReadNI32LE(LB_Reader *reader, LB_ReaderError *out_error) {
    return (double) lbReadI32LE(reader, out_error) / (double) INT32_MAX;
}

inline double lbReadNI32BE(LB_Reader *reader, LB_ReaderError *out_error) {
    return (double) lbReadI32BE(reader, out_error) / (double) INT32_MAX;
}


inline double lbReadNI64(LB_Reader *reader, LB_ReaderError *out_error) {
    return (double) lbReadI64(reader, out_error) / (double) INT64_MAX;
}

inline double lbReadNI64LE(LB_Reader *reader, LB_ReaderError *out_error) {
    return (double) lbReadI64LE(reader, out_error) / (double) INT64_MAX;
}

inline double lbReadNI64BE(LB_Reader *reader, LB_ReaderError *out_error) {
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

const char* lbReaderInitErrorName(const LB_ReaderInitError error);
const char* lbReaderInitErrorMessage(const LB_ReaderInitError error);
const char* lbReaderErrorName(const LB_ReaderError error);
const char* lbReaderErrorMessage(const LB_ReaderError error);

LB_ReaderInitError lbReaderInitBuffer(LB_Reader *reader, const void *data, size_t length);

LB_ReaderInitError lbReaderInitFile(LB_Reader *reader, FILE *file);
#ifdef LB_READER_SAFETY
LB_ReaderError lbReaderCheckSafety(const LB_Reader *reader, const void *out_value, size_t length);
#endif
LB_ReaderMode lbReaderGetMode(const LB_Reader *reader);

LB_ReaderError lbReaderSeek(LB_Reader *reader, size_t position);

size_t lbReaderTell(const LB_Reader *reader);

LB_ReaderError lbReadUnsafe(LB_Reader *reader, void *out_value, size_t length);

LB_ReaderError lbRead(LB_Reader *reader, void *out_value, size_t length);

LB_ReaderError lbReadReversedUnsafe(LB_Reader *reader, void *out_value, size_t length);

LB_ReaderError lbReadReversed(LB_Reader *reader, void *out_value, size_t length);

size_t lbReaderLength(const LB_Reader *reader);

size_t lbReaderRemaining(const LB_Reader *reader);

uint8_t lbReadU8(LB_Reader *reader, LB_ReaderError *out_error);

uint8_t lbReadU8LE(LB_Reader *reader, LB_ReaderError *out_error);

uint8_t lbReadU8BE(LB_Reader *reader, LB_ReaderError *out_error);

uint16_t lbReadU16(LB_Reader *reader, LB_ReaderError *out_error);

uint16_t lbReadU16LE(LB_Reader *reader, LB_ReaderError *out_error);

uint16_t lbReadU16BE(LB_Reader *reader, LB_ReaderError *out_error);

uint32_t lbReadU32(LB_Reader *reader, LB_ReaderError *out_error);

uint32_t lbReadU32LE(LB_Reader *reader, LB_ReaderError *out_error);

uint32_t lbReadU32BE(LB_Reader *reader, LB_ReaderError *out_error);

uint64_t lbReadU64(LB_Reader *reader, LB_ReaderError *out_error);

uint64_t lbReadU64LE(LB_Reader *reader, LB_ReaderError *out_error);

uint64_t lbReadU64BE(LB_Reader *reader, LB_ReaderError *out_error);

int8_t lbReadI8(LB_Reader *reader, LB_ReaderError *out_error);

int8_t lbReadI8LE(LB_Reader *reader, LB_ReaderError *out_error);

int8_t lbReadI8BE(LB_Reader *reader, LB_ReaderError *out_error);

int16_t lbReadI16(LB_Reader *reader, LB_ReaderError *out_error);

int16_t lbReadI16LE(LB_Reader *reader, LB_ReaderError *out_error);

int16_t lbReadI16BE(LB_Reader *reader, LB_ReaderError *out_error);

int32_t lbReadI32(LB_Reader *reader, LB_ReaderError *out_error);

int32_t lbReadI32LE(LB_Reader *reader, LB_ReaderError *out_error);

int32_t lbReadI32BE(LB_Reader *reader, LB_ReaderError *out_error);

int64_t lbReadI64(LB_Reader *reader, LB_ReaderError *out_error);

int64_t lbReadI64LE(LB_Reader *reader, LB_ReaderError *out_error);

int64_t lbReadI64BE(LB_Reader *reader, LB_ReaderError *out_error);

float lbReadF32(LB_Reader *reader, LB_ReaderError *out_error);

float lbReadF32LE(LB_Reader *reader, LB_ReaderError *out_error);

float lbReadF32BE(LB_Reader *reader, LB_ReaderError *out_error);

double lbReadF64(LB_Reader *reader, LB_ReaderError *out_error);

double lbReadF64LE(LB_Reader *reader, LB_ReaderError *out_error);

double lbReadF64BE(LB_Reader *reader, LB_ReaderError *out_error);

float lbReadNU8(LB_Reader *reader, LB_ReaderError *out_error);

float lbReadNU8LE(LB_Reader *reader, LB_ReaderError *out_error);

float lbReadNU8BE(LB_Reader *reader, LB_ReaderError *out_error);

float lbReadNU16(LB_Reader *reader, LB_ReaderError *out_error);

float lbReadNU16LE(LB_Reader *reader, LB_ReaderError *out_error);

float lbReadNU16BE(LB_Reader *reader, LB_ReaderError *out_error);

double lbReadNU32(LB_Reader *reader, LB_ReaderError *out_error);

double lbReadNU32LE(LB_Reader *reader, LB_ReaderError *out_error);

double lbReadNU32BE(LB_Reader *reader, LB_ReaderError *out_error);

double lbReadNU64(LB_Reader *reader, LB_ReaderError *out_error);

double lbReadNU64LE(LB_Reader *reader, LB_ReaderError *out_error);

double lbReadNU64BE(LB_Reader *reader, LB_ReaderError *out_error);

float lbReadNI8(LB_Reader *reader, LB_ReaderError *out_error);

float lbReadNI8LE(LB_Reader *reader, LB_ReaderError *out_error);

float lbReadNI8BE(LB_Reader *reader, LB_ReaderError *out_error);

float lbReadNI16(LB_Reader *reader, LB_ReaderError *out_error);

float lbReadNI16LE(LB_Reader *reader, LB_ReaderError *out_error);

float lbReadNI16BE(LB_Reader *reader, LB_ReaderError *out_error);

double lbReadNI32(LB_Reader *reader, LB_ReaderError *out_error);

double lbReadNI32LE(LB_Reader *reader, LB_ReaderError *out_error);

double lbReadNI32BE(LB_Reader *reader, LB_ReaderError *out_error);

double lbReadNI64(LB_Reader *reader, LB_ReaderError *out_error);

double lbReadNI64LE(LB_Reader *reader, LB_ReaderError *out_error);

double lbReadNI64BE(LB_Reader *reader, LB_ReaderError *out_error);
#endif

#ifdef __cplusplus
}
#endif
