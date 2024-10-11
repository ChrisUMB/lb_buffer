#ifndef LB_WRITER_H
#define LB_WRITER_H

#ifdef __cplusplus
#include <cstring>
#include <cstdint>
extern "C" {
#else
#include <string.h>
#include <stdint.h>
#endif

#ifndef LB_WRITER_NO_SAFETY
#define LB_WRITER_SAFETY
#endif

typedef enum LB_WriterMode {
    LB_WRITER_MODE_BUFFER = 0,
    LB_WRITER_MODE_FILE = 1,
} LB_WriterMode;

// Error codes for initializing a LB_Writer.
typedef enum LB_WriterInitError {
    // No error.
    LB_WRITER_INIT_NONE = 0x0,
    // The data is NULL.
    LB_WRITER_INIT_DATA_NULL = 0x1,
    // The writer is NULL.
    LB_WRITER_INIT_NO_WRITER = 0x2,
    // The file is invalid.
    LB_WRITER_INIT_INVALID_FILE = 0x4,
    // The length is zero.
    LB_WRITER_INIT_LENGTH_ZERO = 0x8,
} LB_WriterInitError;

// Error codes for `lbWriter` methods.
typedef enum LB_WriterError {
    // No error.
    LB_WRITER_ERROR_NONE = 0x0,
    // The writer is full.
    LB_WRITER_ERROR_FULL = 0x1,
    // The writer is NULL.
    LB_WRITER_ERROR_WRITER_NULL = 0x2,
    // The data is NULL.
    LB_WRITER_ERROR_DATA_NULL = 0x4,

    // The value is invalid.
    LB_WRITER_ERROR_INVALID_VALUE = 0x8,
} LB_WriterError;

typedef struct LB_WriterBuffer {
    const void *data;
    size_t length;
    size_t position;
} LB_WriterBuffer;

typedef struct LB_Writer {
    struct {
        LB_WriterMode mode;

        union {
            LB_WriterBuffer buffer;
            FILE *file;
        };
    } _;
} LB_Writer;

/**
 * Initialize a LB_Writer and returns an error code, if any.                           <br>
 * Will not do error checking if LB_WRITER_NO_SAFETY is defined.
 *
 * @param writer A pointer to the LB_Writer to be initialized.                         <br>
 * If NULL and LB_WRITER_SAFETY, will return LB_WRITER_INIT_NO_WRITER.                <br>
 * @param data A pointer to the data to be written to.                                      <br>
 * If NULL and LB_WRITER_SAFETY, will return LB_WRITER_INIT_DATA_NULL.                <br>
 * @param length The length of the data to be written.                                      <br>
 * If 0 and LB_WRITER_SAFETY, will return LB_WRITER_INIT_LENGTH_ZERO.                 <br>
 * @return LB_WriterInitError An error code indicating the result of the initialization.
 */
inline LB_WriterInitError lbWriterInitBuffer(LB_Writer *writer, void *data, const size_t length) {
#ifdef LB_WRITER_SAFETY
    LB_WriterInitError e = LB_WRITER_INIT_NONE;
    if (writer == NULL) {
        e |= LB_WRITER_INIT_NO_WRITER;
    }

    if (data == NULL) {
        e |= LB_WRITER_INIT_DATA_NULL;
    }

    if (length == 0) {
        e |= LB_WRITER_INIT_LENGTH_ZERO;
    }

    if (e) {
        return e;
    }
#endif

    // ReSharper disable once CppDFANullDereference // Not null, or if safety is disabled then just crash anyway.
    *writer = (LB_Writer) {
        ._ = {
            .mode = LB_WRITER_MODE_BUFFER,
            .buffer = {
                .data = data,
                .length = length,
                .position = 0,
            },
        },
    };
    return LB_WRITER_INIT_NONE;
}

/**
 * Initialize a LB_Writer and returns an error code, if any.                           <br>
 * Will not do error checking if LB_WRITER_NO_SAFETY is defined.
 *
 * @param writer A pointer to the LB_Writer to be initialized.                         <br>
 * If NULL and LB_WRITER_SAFETY, will return LB_WRITER_INIT_NO_WRITER.                <br>
 * @param file A pointer to the LB_Writer to be initialized.                         <br>
 * If NULL and LB_WRITER_SAFETY, will return LB_WRITER_INIT_INVALID_FILE.                <br>
 * @return LB_WriterInitError An error code indicating the result of the initialization.
 */
inline LB_WriterInitError lbWriterInitFile(LB_Writer *writer, FILE *file) {
#ifdef LB_WRITER_SAFETY
    LB_WriterInitError e = LB_WRITER_INIT_NONE;
    if (file == NULL) {
        e |= LB_WRITER_INIT_INVALID_FILE;
    }

    if (e) {
        return e;
    }
#endif

    *writer = (LB_Writer) {
        ._ = {
            .mode = LB_WRITER_MODE_FILE,
            .file = file,
        },
    };
    return LB_WRITER_INIT_NONE;
}

#ifdef LB_WRITER_SAFETY
inline LB_WriterError lbWriterCheckSafety(const LB_Writer *writer, const void *value, const size_t length) {
    LB_WriterError e = LB_WRITER_ERROR_NONE;
    if (writer == NULL) {
        e |= LB_WRITER_ERROR_WRITER_NULL;
    } else if(writer->_.mode == LB_WRITER_MODE_BUFFER) {
        const LB_WriterBuffer *buffer = &writer->_.buffer;
        if (buffer->position + length > buffer->length) {
            e |= LB_WRITER_ERROR_FULL;
        }

        if (buffer->data == NULL) {
            e |= LB_WRITER_ERROR_DATA_NULL;
        }
    } else if(writer->_.mode == LB_WRITER_MODE_FILE) {
        if(writer->_.file == NULL) {
            e |= LB_WRITER_ERROR_DATA_NULL;
        }
    }

    if (value == NULL) {
        e |= LB_WRITER_ERROR_INVALID_VALUE;
    }

    return e;
}

#else
#define lbWriteSafety(writer, length, value) LB_WRITER_WRITE_NONE
#endif

inline LB_WriterError lbWriterSeek(LB_Writer *writer, const size_t position) {
    if (writer->_.mode == LB_WRITER_MODE_BUFFER) {
        LB_WriterBuffer *buffer = &writer->_.buffer;
        if (position >= buffer->length) {
            return LB_WRITER_ERROR_FULL;
        }

        buffer->position = position;
        return LB_WRITER_ERROR_NONE;
    }

    FILE *file = writer->_.file;
    if (fseek(file, position, SEEK_SET) != 0) {
        return LB_WRITER_ERROR_FULL;
    }

    return LB_WRITER_ERROR_NONE;
}

#define lbWriterTell lbWriterPosition

inline size_t lbWriterPosition(const LB_Writer *writer) {
    if (writer->_.mode == LB_WRITER_MODE_BUFFER) {
        return writer->_.buffer.position;
    }

    FILE *file = writer->_.file;
#ifdef _LARGEFILE64_SOURCE
    return ftello64(file);
#else
    return ftell(file);
#endif
}

inline size_t lbWriterLength(const LB_Writer *writer) {
    if (writer->_.mode == LB_WRITER_MODE_BUFFER) {
        return writer->_.buffer.length;
    }

    FILE *file = writer->_.file;
#ifdef _LARGEFILE64_SOURCE
    const off64_t position = ftello64(file);
    fseeko64(file, 0, SEEK_END);
    const off64_t end = ftello64(file);
    fseeko64(file, position, SEEK_SET);
    return end;
#else
    const long position = ftell(file);
    fseek(file, 0, SEEK_END);
    const long end = ftell(file);
    fseek(file, position, SEEK_SET);
    return end;
#endif
}

inline size_t lbWriterRemaining(const LB_Writer *writer) {
    return lbWriterLength(writer) - lbWriterPosition(writer);
}

inline LB_WriterError lbWriteUnsafe(LB_Writer *writer, const void *value, const size_t length) {
    if(writer->_.mode == LB_WRITER_MODE_BUFFER) {
        LB_WriterBuffer *buffer = &writer->_.buffer;
        memcpy(((uint8_t *) buffer->data) + buffer->position, value, length);
        buffer->position += length;
        return LB_WRITER_ERROR_NONE;
    }

    if (fwrite(value, length, 1, writer->_.file) != 1) {
        return LB_WRITER_ERROR_FULL; // TODO: What?
    }
    return LB_WRITER_ERROR_NONE;
}

inline LB_WriterError lbWrite(LB_Writer *writer, const void *value, const size_t length) {
#ifdef LB_WRITER_SAFETY
    const LB_WriterError e = lbWriterCheckSafety(writer, value, length);
    if (e) {
        return e;
    }
#endif

    return lbWriteUnsafe(writer, value, length);
}

inline LB_WriterError lbWriteReversedUnsafe(LB_Writer *writer, const void *value, const size_t length) {
    if(writer->_.mode == LB_WRITER_MODE_BUFFER) {
        LB_WriterBuffer *buffer = &writer->_.buffer;
        for (size_t i = 0; i < length; i++) {
            *((uint8_t *) buffer->data + buffer->position + i) = *((uint8_t *) value + length - 1 - i);
        }

        buffer->position += length;
        return LB_WRITER_ERROR_NONE;
    }

    // TODO: Do NOT write one byte at a time!
    for (size_t i = 0; i < length; i++) {
        if (fwrite(((uint8_t *) value) + length - 1 - i, 1, 1, writer->_.file) != 1) {
            return LB_WRITER_ERROR_FULL; // TODO: What?
        }
    }

    return LB_WRITER_ERROR_NONE;
}

inline LB_WriterError lbWriteReversed(LB_Writer *writer, const void *value, const size_t length) {
#ifdef LB_WRITER_SAFETY
    const LB_WriterError e = lbWriterCheckSafety(writer, value, length);
    if (e) {
        return e;
    }
#endif

    return lbWriteReversedUnsafe(writer, value, length);
}

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define lbWriteLE lbWrite
#define lbWriteBE lbWriteReversed
#define lbWriteUnsafeLE lbWriteUnsafe
#define lbWriteUnsafeBE lbWriteReversedUnsafe
#else
#define lbWriterLE lbWriteReversed
#define lbWriterBE lbWrite
#define lbWriterUnsafeLE lbWriteReversedUnsafe
#define lbWriterUnsafeBE lbWriteUnsafe
#endif

inline LB_WriterError lbWriteU8(LB_Writer *writer, const uint8_t value) {
    return lbWrite(writer, &value, sizeof(value));
}

inline LB_WriterError lbWriteU8LE(LB_Writer *writer, const uint8_t value) {
    return lbWriteLE(writer, &value, sizeof(value));
}

inline LB_WriterError lbWriteU8BE(LB_Writer *writer, const uint8_t value) {
    return lbWriteBE(writer, &value, sizeof(value));
}

inline LB_WriterError lbWriteU16(LB_Writer *writer, const uint16_t value) {
    return lbWrite(writer, &value, sizeof(value));
}

inline LB_WriterError lbWriteU16LE(LB_Writer *writer, const uint16_t value) {
    return lbWriteLE(writer, &value, sizeof(value));
}

inline LB_WriterError lbWriteU16BE(LB_Writer *writer, const uint16_t value) {
    return lbWriteBE(writer, &value, sizeof(value));
}

inline LB_WriterError lbWriteU32(LB_Writer *writer, const uint32_t value) {
    return lbWrite(writer, &value, sizeof(value));
}

inline LB_WriterError lbWriteU32LE(LB_Writer *writer, const uint32_t value) {
    return lbWriteLE(writer, &value, sizeof(value));
}

inline LB_WriterError lbWriteU32BE(LB_Writer *writer, const uint32_t value) {
    return lbWriteBE(writer, &value, sizeof(value));
}


inline LB_WriterError lbWriteU64(LB_Writer *writer, const uint64_t value) {
    return lbWrite(writer, &value, sizeof(value));
}

inline LB_WriterError lbWriteU64LE(LB_Writer *writer, const uint64_t value) {
    return lbWriteLE(writer, &value, sizeof(value));
}

inline LB_WriterError lbWriteU64BE(LB_Writer *writer, const uint64_t value) {
    return lbWriteBE(writer, &value, sizeof(value));
}


inline LB_WriterError lbWriteI8(LB_Writer *writer, const int8_t value) {
    return lbWrite(writer, &value, sizeof(value));
}

inline LB_WriterError lbWriteI8LE(LB_Writer *writer, const int8_t value) {
    return lbWriteLE(writer, &value, sizeof(value));
}

inline LB_WriterError lbWriteI8BE(LB_Writer *writer, const int8_t value) {
    return lbWriteBE(writer, &value, sizeof(value));
}

inline LB_WriterError lbWriteI16(LB_Writer *writer, const int16_t value) {
    return lbWrite(writer, &value, sizeof(value));
}

inline LB_WriterError lbWriteI16LE(LB_Writer *writer, const int16_t value) {
    return lbWriteLE(writer, &value, sizeof(value));
}

inline LB_WriterError lbWriteI16BE(LB_Writer *writer, const int16_t value) {
    return lbWriteBE(writer, &value, sizeof(value));
}

inline LB_WriterError lbWriteI32(LB_Writer *writer, const int32_t value) {
    return lbWrite(writer, &value, sizeof(value));
}

inline LB_WriterError lbWriteI32LE(LB_Writer *writer, const int32_t value) {
    return lbWriteLE(writer, &value, sizeof(value));
}

inline LB_WriterError lbWriteI32BE(LB_Writer *writer, const int32_t value) {
    return lbWriteBE(writer, &value, sizeof(value));
}


inline LB_WriterError lbWriteI64(LB_Writer *writer, const int64_t value) {
    return lbWrite(writer, &value, sizeof(value));
}

inline LB_WriterError lbWriteI64LE(LB_Writer *writer, const int64_t value) {
    return lbWriteLE(writer, &value, sizeof(value));
}

inline LB_WriterError lbWriteI64BE(LB_Writer *writer, const int64_t value) {
    return lbWriteBE(writer, &value, sizeof(value));
}


inline LB_WriterError lbWriteF32(LB_Writer *writer, const float value) {
    return lbWrite(writer, &value, sizeof(value));
}

inline LB_WriterError lbWriteF32LE(LB_Writer *writer, const float value) {
    return lbWriteLE(writer, &value, sizeof(value));
}

inline LB_WriterError lbWriteF32BE(LB_Writer *writer, const float value) {
    return lbWriteBE(writer, &value, sizeof(value));
}


inline LB_WriterError lbWriteF64(LB_Writer *writer, const double value) {
    return lbWrite(writer, &value, sizeof(value));
}

inline LB_WriterError lbWriteF64LE(LB_Writer *writer, const double value) {
    return lbWriteLE(writer, &value, sizeof(value));
}

inline LB_WriterError lbWriteF64BE(LB_Writer *writer, const double value) {
    return lbWriteBE(writer, &value, sizeof(value));
}

/*
 * Normalized Integer Functions
 */

// safety stuff
#ifdef LB_WRITER_SAFETY
#define LB_WRITER_NORMALIZED_UNSIGNED_SAFETY \
    LB_WriterError e = lbWriterCheckSafety(writer, &normalized, 1); \
    if (value < 0.0f || value > 1.0f) { \
        e |= LB_WRITER_ERROR_INVALID_VALUE; \
    } \
    if (e) { \
        return e; \
    }

#define LB_WRITER_NORMALIZED_SIGNED_SAFETY \
LB_WriterError e = lbWriterCheckSafety(writer, &normalized, 1); \
if (value < -1.0f || value > 1.0f) { \
e |= LB_WRITER_ERROR_INVALID_VALUE; \
} \
if (e) { \
return e; \
}
#else
#define LB_WRITER_NORMALIZED_SIGNED_SAFETY
#define LB_WRITER_NORMALIZED_UNSIGNED_SAFETY
#endif

inline LB_WriterError lbWriteNU8(LB_Writer *writer, const float value) {
    const uint8_t normalized = (uint8_t) (value * ((float) UINT8_MAX) + 0.5f);
    LB_WRITER_NORMALIZED_UNSIGNED_SAFETY
    return lbWriteUnsafe(writer, &normalized, sizeof(normalized));
}

inline LB_WriterError lbWriteNU8LE(LB_Writer *writer, const float value) {
    return lbWriteNU8(writer, value);
}

inline LB_WriterError lbWriteNU8BE(LB_Writer *writer, const float value) {
    return lbWriteNU8(writer, value);
}


inline LB_WriterError lbWriteNU16(LB_Writer *writer, const float value) {
    const uint16_t normalized = (uint16_t) (value * ((float) UINT16_MAX) + 0.5f);
    LB_WRITER_NORMALIZED_UNSIGNED_SAFETY
    return lbWriteUnsafe(writer, &normalized, sizeof(normalized));
}

inline LB_WriterError lbWriteNU16LE(LB_Writer *writer, const float value) {
    return lbWriteNU16(writer, value);
}

inline LB_WriterError lbWriteNU16BE(LB_Writer *writer, const float value) {
    return lbWriteNU16(writer, value);
}

/*  Normalized integers utilizing more than 23 bits can carry more precision than a float.  */
inline LB_WriterError lbWriteNU32(LB_Writer *writer, const double value) {
    const uint32_t normalized = (uint32_t) (value * ((double) UINT32_MAX) + 0.5);
    LB_WRITER_NORMALIZED_UNSIGNED_SAFETY
    return lbWriteUnsafe(writer, &normalized, sizeof(normalized));
}

inline LB_WriterError lbWriteNU32LE(LB_Writer *writer, const double value) {
    return lbWriteNU32(writer, value);
}

inline LB_WriterError lbWriteNU32BE(LB_Writer *writer, const double value) {
    return lbWriteNU32(writer, value);
}


inline LB_WriterError lbWriteNU64(LB_Writer *writer, const double value) {
    const uint64_t normalized = (uint64_t) (value * ((double) UINT64_MAX) + 0.5);
    LB_WRITER_NORMALIZED_UNSIGNED_SAFETY
    return lbWriteUnsafe(writer, &normalized, sizeof(normalized));
}

inline LB_WriterError lbWriteNU64LE(LB_Writer *writer, const double value) {
    return lbWriteNU64(writer, value);
}

inline LB_WriterError lbWriteNU64BE(LB_Writer *writer, const double value) {
    return lbWriteNU64(writer, value);
}


inline LB_WriterError lbWriteNI8(LB_Writer *writer, const float value) {
    const int8_t normalized = (int8_t) (value * ((float) INT8_MAX) + 0.5f);
    LB_WRITER_NORMALIZED_SIGNED_SAFETY
    return lbWriteUnsafe(writer, &normalized, sizeof(normalized));
}

inline LB_WriterError lbWriteNI8LE(LB_Writer *writer, const float value) {
    return lbWriteNI8(writer, value);
}

inline LB_WriterError lbWriteNI8BE(LB_Writer *writer, const float value) {
    return lbWriteNI8(writer, value);
}


inline LB_WriterError lbWriteNI16(LB_Writer *writer, const float value) {
    const int16_t normalized = (int16_t) (value * ((float) INT16_MAX) + 0.5f);
    LB_WRITER_NORMALIZED_SIGNED_SAFETY
    return lbWriteUnsafe(writer, &normalized, sizeof(normalized));
}

inline LB_WriterError lbWriteNI16LE(LB_Writer *writer, const float value) {
    return lbWriteNI16(writer, value);
}

inline LB_WriterError lbWriteNI16BE(LB_Writer *writer, const float value) {
    return lbWriteNI16(writer, value);
}


inline LB_WriterError lbWriteNI32(LB_Writer *writer, const double value) {
    const int32_t normalized = (int32_t) (value * ((double) INT32_MAX) + 0.5);
    LB_WRITER_NORMALIZED_SIGNED_SAFETY
    return lbWriteUnsafe(writer, &normalized, sizeof(normalized));
}

inline LB_WriterError lbWriteNI32LE(LB_Writer *writer, const double value) {
    return lbWriteNI32(writer, value);
}

inline LB_WriterError lbWriteNI32BE(LB_Writer *writer, const double value) {
    return lbWriteNI32(writer, value);
}


inline LB_WriterError lbWriteNI64(LB_Writer *writer, const double value) {
    const int64_t normalized = (int64_t) (value * ((double) INT64_MAX) + 0.5);
    LB_WRITER_NORMALIZED_SIGNED_SAFETY
    return lbWriteUnsafe(writer, &normalized, sizeof(normalized));
}

inline LB_WriterError lbWriteNI64LE(LB_Writer *writer, const double value) {
    return lbWriteNI64(writer, value);
}

inline LB_WriterError lbWriteNI64BE(LB_Writer *writer, const double value) {
    return lbWriteNI64(writer, value);
}
#ifdef __cplusplus
}
#endif
#endif

#ifdef LB_WRITER_IMPLEMENTATION
#ifdef __cplusplus
extern "C" {
#endif
LB_WriterInitError lbWriterInitBuffer(LB_Writer *writer, void *data, size_t length);
LB_WriterInitError lbWriterInitFile(LB_Writer *writer, FILE *file);
#ifdef LB_WRITER_SAFETY
LB_WriterError lbWriterCheckSafety(const LB_Writer *writer, const void *value, size_t length);
#endif

LB_WriterMode lbWriterGetMode(const LB_Writer *writer);

LB_WriterError lbWriterSeek(LB_Writer *writer, size_t position);

size_t lbWriterTell(const LB_Writer *writer);

LB_WriterError lbWriteUnsafe(LB_Writer *writer, const void *value, size_t length);
LB_WriterError lbWrite(LB_Writer *writer, const void *value, size_t length);
LB_WriterError lbWriteReversedUnsafe(LB_Writer *writer, const void *value, size_t length);
LB_WriterError lbWriteReversed(LB_Writer *writer, const void *value, size_t length);
LB_WriterError lbWriteU8(LB_Writer *writer, uint8_t value);
LB_WriterError lbWriteU8LE(LB_Writer *writer, uint8_t value);
LB_WriterError lbWriteU8BE(LB_Writer *writer, uint8_t value);
LB_WriterError lbWriteU16(LB_Writer *writer, uint16_t value);
LB_WriterError lbWriteU16LE(LB_Writer *writer, uint16_t value);
LB_WriterError lbWriteU16BE(LB_Writer *writer, uint16_t value);
LB_WriterError lbWriteU32(LB_Writer *writer, uint32_t value);
LB_WriterError lbWriteU32LE(LB_Writer *writer, uint32_t value);
LB_WriterError lbWriteU32BE(LB_Writer *writer, uint32_t value);
LB_WriterError lbWriteU64(LB_Writer *writer, uint64_t value);
LB_WriterError lbWriteU64LE(LB_Writer *writer, uint64_t value);
LB_WriterError lbWriteU64BE(LB_Writer *writer, uint64_t value);
LB_WriterError lbWriteI8(LB_Writer *writer, int8_t value);
LB_WriterError lbWriteI8LE(LB_Writer *writer, int8_t value);
LB_WriterError lbWriteI8BE(LB_Writer *writer, int8_t value);
LB_WriterError lbWriteI16(LB_Writer *writer, int16_t value);
LB_WriterError lbWriteI16LE(LB_Writer *writer, int16_t value);
LB_WriterError lbWriteI16BE(LB_Writer *writer, int16_t value);
LB_WriterError lbWriteI32(LB_Writer *writer, int32_t value);
LB_WriterError lbWriteI32LE(LB_Writer *writer, int32_t value);
LB_WriterError lbWriteI32BE(LB_Writer *writer, int32_t value);
LB_WriterError lbWriteI64(LB_Writer *writer, int64_t value);
LB_WriterError lbWriteI64LE(LB_Writer *writer, int64_t value);
LB_WriterError lbWriteI64BE(LB_Writer *writer, int64_t value);
LB_WriterError lbWriteF32(LB_Writer *writer, float value);
LB_WriterError lbWriteF32LE(LB_Writer *writer, float value);
LB_WriterError lbWriteF32BE(LB_Writer *writer, float value);
LB_WriterError lbWriteF64(LB_Writer *writer, double value);
LB_WriterError lbWriteF64LE(LB_Writer *writer, double value);
LB_WriterError lbWriteF64BE(LB_Writer *writer, double value);
LB_WriterError lbWriteNU8(LB_Writer *writer, float value);
LB_WriterError lbWriteNU8LE(LB_Writer *writer, float value);
LB_WriterError lbWriteNU8BE(LB_Writer *writer, float value);
LB_WriterError lbWriteNU16(LB_Writer *writer, float value);
LB_WriterError lbWriteNU16LE(LB_Writer *writer, float value);
LB_WriterError lbWriteNU16BE(LB_Writer *writer, float value);
LB_WriterError lbWriteNU32(LB_Writer *writer, double value);
LB_WriterError lbWriteNU32LE(LB_Writer *writer, double value);
LB_WriterError lbWriteNU32BE(LB_Writer *writer, double value);
LB_WriterError lbWriteNU64(LB_Writer *writer, double value);
LB_WriterError lbWriteNU64LE(LB_Writer *writer, double value);
LB_WriterError lbWriteNU64BE(LB_Writer *writer, double value);
LB_WriterError lbWriteNI8(LB_Writer *writer, float value);
LB_WriterError lbWriteNI8LE(LB_Writer *writer, float value);
LB_WriterError lbWriteNI8BE(LB_Writer *writer, float value);
LB_WriterError lbWriteNI16(LB_Writer *writer, float value);
LB_WriterError lbWriteNI16LE(LB_Writer *writer, float value);
LB_WriterError lbWriteNI16BE(LB_Writer *writer, float value);
LB_WriterError lbWriteNI32(LB_Writer *writer, double value);
LB_WriterError lbWriteNI32LE(LB_Writer *writer, double value);
LB_WriterError lbWriteNI32BE(LB_Writer *writer, double value);
LB_WriterError lbWriteNI64(LB_Writer *writer, double value);
LB_WriterError lbWriteNI64LE(LB_Writer *writer, double value);
LB_WriterError lbWriteNI64BE(LB_Writer *writer, double value);
#endif

#ifdef __cplusplus
}
#endif

