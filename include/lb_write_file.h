#ifndef LB_WRITE_FILE_H
#define LB_WRITE_FILE_H

#ifdef __cplusplus
#include <cstring>
#include <cstdint>
extern "C" {
#else
#include <string.h>
#include <stdint.h>
#endif

#ifndef LB_WRITE_FILE_NO_SAFETY
#define LB_WRITE_FILE_SAFETY
#endif

// Error codes for initializing a LB_WriteFile.
typedef enum LB_WriteFileInitError {
    // No error.
    LB_WRITE_FILE_INIT_NONE = 0x0,
    // The data is NULL.
    LB_WRITE_FILE_INIT_DATA_NULL = 0x1,
    // The file is NULL.
    LB_WRITE_FILE_INIT_NO_FILE = 0x2,
} LB_WriteFileInitError;

// Error codes for `lbWriteFile` methods.
typedef enum LB_WriteFileWriteError {
    // No error.
    LB_WRITE_FILE_WRITE_NONE = 0x0,
    // The file is full.
    LB_WRITE_FILE_WRITE_FULL = 0x1,
    // The file is NULL.
    LB_WRITE_FILE_WRITE_FILE_NULL = 0x2,
    // The data is NULL.
    LB_WRITE_FILE_WRITE_DATA_NULL = 0x4,

    // The value is invalid.
    LB_WRITE_FILE_WRITE_INVALID_VALUE = 0x8,
} LB_WriteFileWriteError;

typedef struct LB_WriteFile {
    FILE *data;
} LB_WriteFile;

/**
 * Initialize a LB_WriteFile and returns an error code, if any.                           <br>
 * Will not do error checking if LB_WRITE_FILE_NO_SAFETY is defined.
 *
 * @param file A pointer to the LB_WriteFile to be initialized.                         <br>
 * If NULL and LB_WRITE_FILE_SAFETY, will return LB_WRITE_FILE_INIT_NO_FILE.                <br>
 * @param data A pointer to the data to be written to.                                      <br>
 * If NULL and LB_WRITE_FILE_SAFETY, will return LB_WRITE_FILE_INIT_DATA_NULL.                <br>
 * @param length The length of the data to be written.                                      <br>
 * If 0 and LB_WRITE_FILE_SAFETY, will return LB_WRITE_FILE_INIT_LENGTH_ZERO.                 <br>
 * @return LB_WriteFileInitError An error code indicating the result of the initialization.
 */
inline LB_WriteFileInitError lbWriteFileInit(LB_WriteFile *file, FILE *data) {
#ifdef LB_WRITE_FILE_SAFETY
    LB_WriteFileInitError e = LB_WRITE_FILE_INIT_NONE;
    if (file == NULL) {
        e |= LB_WRITE_FILE_INIT_NO_FILE;
    }

    if (data == NULL) {
        e |= LB_WRITE_FILE_INIT_DATA_NULL;
    }

    if (e) {
        return e;
    }
#endif

    file->data = data;
    return LB_WRITE_FILE_INIT_NONE;
}

#ifdef LB_WRITE_FILE_SAFETY
inline LB_WriteFileWriteError lbWriteFileWriteSafety(const LB_WriteFile *file, const size_t length, const void *value) {
    LB_WriteFileWriteError e = LB_WRITE_FILE_WRITE_NONE;
    if (file == NULL) {
        e |= LB_WRITE_FILE_WRITE_FILE_NULL;
    } else {
        if (file->data == NULL) {
            e |= LB_WRITE_FILE_WRITE_DATA_NULL;
        }
    }

    if (value == NULL) {
        e |= LB_WRITE_FILE_WRITE_INVALID_VALUE;
    }

    return e;
}

#else
#define lbWriteFileWriteSafety(file, length, value) LB_WRITE_FILE_WRITE_NONE
#endif

inline LB_WriteFileWriteError lbWriteFileWriteUnsafe(LB_WriteFile *file, const size_t length, const void *value) {
    // memcpy(((uint8_t *) file->data) + file->position, value, length);
    // file->position += length;
    if (fwrite(value, length, 1, file->data) != 1) {
        return LB_WRITE_FILE_WRITE_FULL; // TODO: What?
    }
    return LB_WRITE_FILE_WRITE_NONE;
}

inline LB_WriteFileWriteError lbWriteFileWrite(LB_WriteFile *file, const size_t length, const void *value) {
#ifdef LB_WRITE_FILE_SAFETY
    const LB_WriteFileWriteError e = lbWriteFileWriteSafety(file, length, value);
    if (e) {
        return e;
    }
#endif

    return lbWriteFileWriteUnsafe(file, length, value);
}

inline LB_WriteFileWriteError lbWriteFileWriteReversedUnsafe(LB_WriteFile *file, const size_t length,
                                                                 const void *value) {
    // TODO: Do NOT write one byte at a time!
    for (size_t i = 0; i < length; i++) {
        if (fwrite(((uint8_t *) value) + length - 1 - i, 1, 1, file->data) != 1) {
            return LB_WRITE_FILE_WRITE_FULL; // TODO: What?
        }
    }

    return LB_WRITE_FILE_WRITE_NONE;
}

inline LB_WriteFileWriteError lbWriteFileWriteReversed(LB_WriteFile *file, const size_t length, const void *value) {
#ifdef LB_WRITE_FILE_SAFETY
    const LB_WriteFileWriteError e = lbWriteFileWriteSafety(file, length, value);
    if (e) {
        return e;
    }
#endif

    return lbWriteFileWriteReversedUnsafe(file, length, value);
}

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define lbWriteFileLE lbWriteFileWrite
#define lbWriteFileBE lbWriteFileWriteReversed
#define lbWriteFileUnsafeLE lbWriteFileWriteUnsafe
#define lbWriteFileUnsafeBE lbWriteFileWriteReversedUnsafe
#else
#define lbWriteFileLE lbWriteFileWriteReversed
#define lbWriteFileBE lbWriteFileWrite
#define lbWriteFileUnsafeLE lbWriteFileWriteReversedUnsafe
#define lbWriteFileUnsafeBE lbWriteFileWriteUnsafe
#endif

inline LB_WriteFileWriteError lbWriteFileU8(LB_WriteFile *file, const uint8_t value) {
    return lbWriteFileWrite(file, sizeof(value), &value);
}

inline LB_WriteFileWriteError lbWriteFileU8LE(LB_WriteFile *file, const uint8_t value) {
    return lbWriteFileLE(file, sizeof(value), &value);
}

inline LB_WriteFileWriteError lbWriteFileU8BE(LB_WriteFile *file, const uint8_t value) {
    return lbWriteFileBE(file, sizeof(value), &value);
}

inline LB_WriteFileWriteError lbWriteFileU16(LB_WriteFile *file, const uint16_t value) {
    return lbWriteFileWrite(file, sizeof(value), &value);
}

inline LB_WriteFileWriteError lbWriteFileU16LE(LB_WriteFile *file, const uint16_t value) {
    return lbWriteFileLE(file, sizeof(value), &value);
}

inline LB_WriteFileWriteError lbWriteFileU16BE(LB_WriteFile *file, const uint16_t value) {
    return lbWriteFileBE(file, sizeof(value), &value);
}

inline LB_WriteFileWriteError lbWriteFileU32(LB_WriteFile *file, const uint32_t value) {
    return lbWriteFileWrite(file, sizeof(value), &value);
}

inline LB_WriteFileWriteError lbWriteFileU32LE(LB_WriteFile *file, const uint32_t value) {
    return lbWriteFileLE(file, sizeof(value), &value);
}

inline LB_WriteFileWriteError lbWriteFileU32BE(LB_WriteFile *file, const uint32_t value) {
    return lbWriteFileBE(file, sizeof(value), &value);
}


inline LB_WriteFileWriteError lbWriteFileU64(LB_WriteFile *file, const uint64_t value) {
    return lbWriteFileWrite(file, sizeof(value), &value);
}

inline LB_WriteFileWriteError lbWriteFileU64LE(LB_WriteFile *file, const uint64_t value) {
    return lbWriteFileLE(file, sizeof(value), &value);
}

inline LB_WriteFileWriteError lbWriteFileU64BE(LB_WriteFile *file, const uint64_t value) {
    return lbWriteFileBE(file, sizeof(value), &value);
}


inline LB_WriteFileWriteError lbWriteFileI8(LB_WriteFile *file, const int8_t value) {
    return lbWriteFileWrite(file, sizeof(value), &value);
}

inline LB_WriteFileWriteError lbWriteFileI8LE(LB_WriteFile *file, const int8_t value) {
    return lbWriteFileLE(file, sizeof(value), &value);
}

inline LB_WriteFileWriteError lbWriteFileI8BE(LB_WriteFile *file, const int8_t value) {
    return lbWriteFileBE(file, sizeof(value), &value);
}

inline LB_WriteFileWriteError lbWriteFileI16(LB_WriteFile *file, const int16_t value) {
    return lbWriteFileWrite(file, sizeof(value), &value);
}

inline LB_WriteFileWriteError lbWriteFileI16LE(LB_WriteFile *file, const int16_t value) {
    return lbWriteFileLE(file, sizeof(value), &value);
}

inline LB_WriteFileWriteError lbWriteFileI16BE(LB_WriteFile *file, const int16_t value) {
    return lbWriteFileBE(file, sizeof(value), &value);
}

inline LB_WriteFileWriteError lbWriteFileI32(LB_WriteFile *file, const int32_t value) {
    return lbWriteFileWrite(file, sizeof(value), &value);
}

inline LB_WriteFileWriteError lbWriteFileI32LE(LB_WriteFile *file, const int32_t value) {
    return lbWriteFileLE(file, sizeof(value), &value);
}

inline LB_WriteFileWriteError lbWriteFileI32BE(LB_WriteFile *file, const int32_t value) {
    return lbWriteFileBE(file, sizeof(value), &value);
}


inline LB_WriteFileWriteError lbWriteFileI64(LB_WriteFile *file, const int64_t value) {
    return lbWriteFileWrite(file, sizeof(value), &value);
}

inline LB_WriteFileWriteError lbWriteFileI64LE(LB_WriteFile *file, const int64_t value) {
    return lbWriteFileLE(file, sizeof(value), &value);
}

inline LB_WriteFileWriteError lbWriteFileI64BE(LB_WriteFile *file, const int64_t value) {
    return lbWriteFileBE(file, sizeof(value), &value);
}


inline LB_WriteFileWriteError lbWriteFileF32(LB_WriteFile *file, const float value) {
    return lbWriteFileWrite(file, sizeof(value), &value);
}

inline LB_WriteFileWriteError lbWriteFileF32LE(LB_WriteFile *file, const float value) {
    return lbWriteFileLE(file, sizeof(value), &value);
}

inline LB_WriteFileWriteError lbWriteFileF32BE(LB_WriteFile *file, const float value) {
    return lbWriteFileBE(file, sizeof(value), &value);
}


inline LB_WriteFileWriteError lbWriteFileF64(LB_WriteFile *file, const double value) {
    return lbWriteFileWrite(file, sizeof(value), &value);
}

inline LB_WriteFileWriteError lbWriteFileF64LE(LB_WriteFile *file, const double value) {
    return lbWriteFileLE(file, sizeof(value), &value);
}

inline LB_WriteFileWriteError lbWriteFileF64BE(LB_WriteFile *file, const double value) {
    return lbWriteFileBE(file, sizeof(value), &value);
}

/*
 * Normalized Integer Functions
 */

// safety stuff
#ifdef LB_WRITE_FILE_SAFETY
#define LB_WRITE_FILE_NORMALIZED_UNSIGNED_SAFETY \
    LB_WriteFileWriteError e = lbWriteFileWriteSafety(file, 1, &normalized); \
    if (value < 0.0f || value > 1.0f) { \
        e |= LB_WRITE_FILE_WRITE_INVALID_VALUE; \
    } \
    if (e) { \
        return e; \
    }

#define LB_WRITE_FILE_NORMALIZED_SIGNED_SAFETY \
LB_WriteFileWriteError e = lbWriteFileWriteSafety(file, 1, &normalized); \
if (value < -1.0f || value > 1.0f) { \
e |= LB_WRITE_FILE_WRITE_INVALID_VALUE; \
} \
if (e) { \
return e; \
}
#else
#define LB_WRITE_FILE_NORMALIZED_SIGNED_SAFETY
#define LB_WRITE_FILE_NORMALIZED_UNSIGNED_SAFETY
#endif

inline LB_WriteFileWriteError lbWriteFileNU8(LB_WriteFile *file, const float value) {
    const uint8_t normalized = (uint8_t) (value * ((float) UINT8_MAX) + 0.5f);
    LB_WRITE_FILE_NORMALIZED_UNSIGNED_SAFETY
    return lbWriteFileWriteUnsafe(file, sizeof(normalized), &normalized);
}

inline LB_WriteFileWriteError lbWriteFileNU8LE(LB_WriteFile *file, const float value) {
    return lbWriteFileNU8(file, value);
}

inline LB_WriteFileWriteError lbWriteFileNU8BE(LB_WriteFile *file, const float value) {
    return lbWriteFileNU8(file, value);
}


inline LB_WriteFileWriteError lbWriteFileNU16(LB_WriteFile *file, const float value) {
    const uint16_t normalized = (uint16_t) (value * ((float) UINT16_MAX) + 0.5f);
    LB_WRITE_FILE_NORMALIZED_UNSIGNED_SAFETY
    return lbWriteFileWriteUnsafe(file, sizeof(normalized), &normalized);
}

inline LB_WriteFileWriteError lbWriteFileNU16LE(LB_WriteFile *file, const float value) {
    return lbWriteFileNU16(file, value);
}

inline LB_WriteFileWriteError lbWriteFileNU16BE(LB_WriteFile *file, const float value) {
    return lbWriteFileNU16(file, value);
}

/*  Normalized integers utilizing more than 23 bits can carry more precision than a float.  */
inline LB_WriteFileWriteError lbWriteFileNU32(LB_WriteFile *file, const double value) {
    const uint32_t normalized = (uint32_t) (value * ((double) UINT32_MAX) + 0.5);
    LB_WRITE_FILE_NORMALIZED_UNSIGNED_SAFETY
    return lbWriteFileWriteUnsafe(file, sizeof(normalized), &normalized);
}

inline LB_WriteFileWriteError lbWriteFileNU32LE(LB_WriteFile *file, const double value) {
    return lbWriteFileNU32(file, value);
}

inline LB_WriteFileWriteError lbWriteFileNU32BE(LB_WriteFile *file, const double value) {
    return lbWriteFileNU32(file, value);
}


inline LB_WriteFileWriteError lbWriteFileNU64(LB_WriteFile *file, const double value) {
    const uint64_t normalized = (uint64_t) (value * ((double) UINT64_MAX) + 0.5);
    LB_WRITE_FILE_NORMALIZED_UNSIGNED_SAFETY
    return lbWriteFileWriteUnsafe(file, sizeof(normalized), &normalized);
}

inline LB_WriteFileWriteError lbWriteFileNU64LE(LB_WriteFile *file, const double value) {
    return lbWriteFileNU64(file, value);
}

inline LB_WriteFileWriteError lbWriteFileNU64BE(LB_WriteFile *file, const double value) {
    return lbWriteFileNU64(file, value);
}


inline LB_WriteFileWriteError lbWriteFileNI8(LB_WriteFile *file, const float value) {
    const int8_t normalized = (int8_t) (value * ((float) INT8_MAX) + 0.5f);
    LB_WRITE_FILE_NORMALIZED_SIGNED_SAFETY
    return lbWriteFileWriteUnsafe(file, sizeof(normalized), &normalized);
}

inline LB_WriteFileWriteError lbWriteFileNI8LE(LB_WriteFile *file, const float value) {
    return lbWriteFileNI8(file, value);
}

inline LB_WriteFileWriteError lbWriteFileNI8BE(LB_WriteFile *file, const float value) {
    return lbWriteFileNI8(file, value);
}


inline LB_WriteFileWriteError lbWriteFileNI16(LB_WriteFile *file, const float value) {
    const int16_t normalized = (int16_t) (value * ((float) INT16_MAX) + 0.5f);
    LB_WRITE_FILE_NORMALIZED_SIGNED_SAFETY
    return lbWriteFileWriteUnsafe(file, sizeof(normalized), &normalized);
}

inline LB_WriteFileWriteError lbWriteFileNI16LE(LB_WriteFile *file, const float value) {
    return lbWriteFileNI16(file, value);
}

inline LB_WriteFileWriteError lbWriteFileNI16BE(LB_WriteFile *file, const float value) {
    return lbWriteFileNI16(file, value);
}


inline LB_WriteFileWriteError lbWriteFileNI32(LB_WriteFile *file, const double value) {
    const int32_t normalized = (int32_t) (value * ((double) INT32_MAX) + 0.5);
    LB_WRITE_FILE_NORMALIZED_SIGNED_SAFETY
    return lbWriteFileWriteUnsafe(file, sizeof(normalized), &normalized);
}

inline LB_WriteFileWriteError lbWriteFileNI32LE(LB_WriteFile *file, const double value) {
    return lbWriteFileNI32(file, value);
}

inline LB_WriteFileWriteError lbWriteFileNI32BE(LB_WriteFile *file, const double value) {
    return lbWriteFileNI32(file, value);
}


inline LB_WriteFileWriteError lbWriteFileNI64(LB_WriteFile *file, const double value) {
    const int64_t normalized = (int64_t) (value * ((double) INT64_MAX) + 0.5);
    LB_WRITE_FILE_NORMALIZED_SIGNED_SAFETY
    return lbWriteFileWriteUnsafe(file, sizeof(normalized), &normalized);
}

inline LB_WriteFileWriteError lbWriteFileNI64LE(LB_WriteFile *file, const double value) {
    return lbWriteFileNI64(file, value);
}

inline LB_WriteFileWriteError lbWriteFileNI64BE(LB_WriteFile *file, const double value) {
    return lbWriteFileNI64(file, value);
}
#ifdef __cplusplus
}
#endif
#endif

#ifdef LB_WRITE_FILE_IMPLEMENTATION
#ifdef __cplusplus
extern "C" {
#endif
LB_WriteFileInitError lbWriteFileInit(LB_WriteFile *file, FILE *data);
#ifdef LB_WRITE_FILE_SAFETY
LB_WriteFileWriteError lbWriteFileWriteSafety(const LB_WriteFile *file, size_t length, const void *value);
#endif
LB_WriteFileWriteError lbWriteFileWriteUnsafe(LB_WriteFile *file, size_t length, const void *value);
LB_WriteFileWriteError lbWriteFileWrite(LB_WriteFile *file, size_t length, const void *value);
LB_WriteFileWriteError lbWriteFileWriteReversedUnsafe(LB_WriteFile *file, size_t length, const void *value);
LB_WriteFileWriteError lbWriteFileWriteReversed(LB_WriteFile *file, size_t length, const void *value);
LB_WriteFileWriteError lbWriteFileU8(LB_WriteFile *file, uint8_t value);
LB_WriteFileWriteError lbWriteFileU8LE(LB_WriteFile *file, uint8_t value);
LB_WriteFileWriteError lbWriteFileU8BE(LB_WriteFile *file, uint8_t value);
LB_WriteFileWriteError lbWriteFileU16(LB_WriteFile *file, uint16_t value);
LB_WriteFileWriteError lbWriteFileU16LE(LB_WriteFile *file, uint16_t value);
LB_WriteFileWriteError lbWriteFileU16BE(LB_WriteFile *file, uint16_t value);
LB_WriteFileWriteError lbWriteFileU32(LB_WriteFile *file, uint32_t value);
LB_WriteFileWriteError lbWriteFileU32LE(LB_WriteFile *file, uint32_t value);
LB_WriteFileWriteError lbWriteFileU32BE(LB_WriteFile *file, uint32_t value);
LB_WriteFileWriteError lbWriteFileU64(LB_WriteFile *file, uint64_t value);
LB_WriteFileWriteError lbWriteFileU64LE(LB_WriteFile *file, uint64_t value);
LB_WriteFileWriteError lbWriteFileU64BE(LB_WriteFile *file, uint64_t value);
LB_WriteFileWriteError lbWriteFileI8(LB_WriteFile *file, int8_t value);
LB_WriteFileWriteError lbWriteFileI8LE(LB_WriteFile *file, int8_t value);
LB_WriteFileWriteError lbWriteFileI8BE(LB_WriteFile *file, int8_t value);
LB_WriteFileWriteError lbWriteFileI16(LB_WriteFile *file, int16_t value);
LB_WriteFileWriteError lbWriteFileI16LE(LB_WriteFile *file, int16_t value);
LB_WriteFileWriteError lbWriteFileI16BE(LB_WriteFile *file, int16_t value);
LB_WriteFileWriteError lbWriteFileI32(LB_WriteFile *file, int32_t value);
LB_WriteFileWriteError lbWriteFileI32LE(LB_WriteFile *file, int32_t value);
LB_WriteFileWriteError lbWriteFileI32BE(LB_WriteFile *file, int32_t value);
LB_WriteFileWriteError lbWriteFileI64(LB_WriteFile *file, int64_t value);
LB_WriteFileWriteError lbWriteFileI64LE(LB_WriteFile *file, int64_t value);
LB_WriteFileWriteError lbWriteFileI64BE(LB_WriteFile *file, int64_t value);
LB_WriteFileWriteError lbWriteFileF32(LB_WriteFile *file, float value);
LB_WriteFileWriteError lbWriteFileF32LE(LB_WriteFile *file, float value);
LB_WriteFileWriteError lbWriteFileF32BE(LB_WriteFile *file, float value);
LB_WriteFileWriteError lbWriteFileF64(LB_WriteFile *file, double value);
LB_WriteFileWriteError lbWriteFileF64LE(LB_WriteFile *file, double value);
LB_WriteFileWriteError lbWriteFileF64BE(LB_WriteFile *file, double value);
LB_WriteFileWriteError lbWriteFileNU8(LB_WriteFile *file, float value);
LB_WriteFileWriteError lbWriteFileNU8LE(LB_WriteFile *file, float value);
LB_WriteFileWriteError lbWriteFileNU8BE(LB_WriteFile *file, float value);
LB_WriteFileWriteError lbWriteFileNU16(LB_WriteFile *file, float value);
LB_WriteFileWriteError lbWriteFileNU16LE(LB_WriteFile *file, float value);
LB_WriteFileWriteError lbWriteFileNU16BE(LB_WriteFile *file, float value);
LB_WriteFileWriteError lbWriteFileNU32(LB_WriteFile *file, double value);
LB_WriteFileWriteError lbWriteFileNU32LE(LB_WriteFile *file, double value);
LB_WriteFileWriteError lbWriteFileNU32BE(LB_WriteFile *file, double value);
LB_WriteFileWriteError lbWriteFileNU64(LB_WriteFile *file, double value);
LB_WriteFileWriteError lbWriteFileNU64LE(LB_WriteFile *file, double value);
LB_WriteFileWriteError lbWriteFileNU64BE(LB_WriteFile *file, double value);
LB_WriteFileWriteError lbWriteFileNI8(LB_WriteFile *file, float value);
LB_WriteFileWriteError lbWriteFileNI8LE(LB_WriteFile *file, float value);
LB_WriteFileWriteError lbWriteFileNI8BE(LB_WriteFile *file, float value);
LB_WriteFileWriteError lbWriteFileNI16(LB_WriteFile *file, float value);
LB_WriteFileWriteError lbWriteFileNI16LE(LB_WriteFile *file, float value);
LB_WriteFileWriteError lbWriteFileNI16BE(LB_WriteFile *file, float value);
LB_WriteFileWriteError lbWriteFileNI32(LB_WriteFile *file, double value);
LB_WriteFileWriteError lbWriteFileNI32LE(LB_WriteFile *file, double value);
LB_WriteFileWriteError lbWriteFileNI32BE(LB_WriteFile *file, double value);
LB_WriteFileWriteError lbWriteFileNI64(LB_WriteFile *file, double value);
LB_WriteFileWriteError lbWriteFileNI64LE(LB_WriteFile *file, double value);
LB_WriteFileWriteError lbWriteFileNI64BE(LB_WriteFile *file, double value);
#endif

#ifdef __cplusplus
}
#endif

