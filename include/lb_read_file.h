#ifndef LB_READ_FILE_H
#define LB_READ_FILE_H

#ifdef __cplusplus
#include <cstring>
#include <cstdint>
extern "C" {
#else
#include <string.h>
#include <stdint.h>
#endif

#ifndef LB_READ_FILE_NO_SAFETY
#define LB_READ_FILE_SAFETY
#endif

// Error codes for initializing a LB_ReadFile.
typedef enum LB_ReadFileInitError {
    // No error.
    LB_READ_FILE_INIT_NONE = 0x0,
    // The data is NULL.
    LB_READ_FILE_INIT_DATA_NULL = 0x1,
    // The file is NULL.
    LB_READ_FILE_INIT_NO_FILE = 0x2,
} LB_ReadFileInitError;

// Error codes for `lbReadFile` methods.
typedef enum LB_ReadFileReadError {
    // No error.
    LB_READ_FILE_READ_NONE = 0x0,
    // The file is at the end.
    LB_READ_FILE_READ_END = 0x1,
    // The file is NULL.
    LB_READ_FILE_READ_FILE_NULL = 0x2,
    // The data is NULL.
    LB_READ_FILE_READ_DATA_NULL = 0x4,

    // The value is invalid.
    LB_READ_FILE_READ_INVALID_VALUE = 0x8,
} LB_ReadFileReadError;

typedef struct LB_ReadFile {
    FILE *data;
} LB_ReadFile;

/**
 * Initialize a LB_ReadFile and returns an error code, if any.                                <br>
 * Will not do error checking if LB_READ_FILE_NO_SAFETY is defined.
 *
 * @param file A pointer to the LB_ReadFile to be initialized.                              <br>
 * If NULL and LB_READ_FILE_SAFETY, will return LB_READ_FILE_INIT_NO_FILE.                <br>
 * @param data A pointer to the data to be written to.                                          <br>
 * If NULL and LB_READ_FILE_SAFETY, will return LB_READ_FILE_INIT_DATA_NULL.                <br>
 * @return LB_ReadFileInitError An error code indicating the result of the initialization.
 */
inline LB_ReadFileInitError lbReadFileInit(LB_ReadFile *file, FILE *data) {
#ifdef LB_READ_FILE_SAFETY
    LB_ReadFileInitError e = LB_READ_FILE_INIT_NONE;
    if (file == NULL) {
        e |= LB_READ_FILE_INIT_NO_FILE;
    }

    if (data == NULL) {
        e |= LB_READ_FILE_INIT_DATA_NULL;
    }

    if (e) {
        return e;
    }
#endif

    file->data = data;
    return LB_READ_FILE_INIT_NONE;
}

#ifdef LB_READ_FILE_SAFETY
inline LB_ReadFileReadError lbReadFileReadSafety(const LB_ReadFile *file, const size_t length, const void *out_value) {
    LB_ReadFileReadError e = LB_READ_FILE_READ_NONE;
    if (file == NULL) {
        e |= LB_READ_FILE_READ_FILE_NULL;
    } else {
        // if (file->position + length > file->length) {
        //     e |= LB_READ_FILE_READ_END;
        // }

        if (file->data == NULL) {
            e |= LB_READ_FILE_READ_DATA_NULL;
        }
    }

    if (out_value == NULL) {
        e |= LB_READ_FILE_READ_INVALID_VALUE;
    }

    return e;
}

#else
#define lbReadFileReadSafety(file, length, value) LB_READ_FILE_READ_NONE
#endif

inline LB_ReadFileReadError lbReadFileReadUnsafe(LB_ReadFile *file, const size_t length, void *out_value) {
    // memcpy(out_value, (uint8_t *) file->data + file->position, length);
    // file->position += length;
    if(fread(out_value, length, 1, file->data) != 1) {
        return LB_READ_FILE_READ_END;
    }
    return LB_READ_FILE_READ_NONE;
}

inline LB_ReadFileReadError lbReadFileRead(LB_ReadFile *file, const size_t length, void *out_value) {
#ifdef LB_READ_FILE_SAFETY
    const LB_ReadFileReadError e = lbReadFileReadSafety(file, length, out_value);
    if (e) {
        return e;
    }
#endif

    return lbReadFileReadUnsafe(file, length, out_value);
}

inline LB_ReadFileReadError lbReadFileReadReversedUnsafe(LB_ReadFile *file, const size_t length,
                                                                 void *out_value) {
    // for (size_t i = 0; i < length; i++) {
    //     *((uint8_t *) out_value + length - 1 - i) = *((uint8_t *) file->data + file->position + i);
    // }
    //
    // file->position += length;
    if(fread(out_value, length, 1, file->data) != 1) {
        return LB_READ_FILE_READ_END;
    }

    // Reverse the bytes.
    for (size_t i = 0; i < length / 2; i++) {
        const uint8_t temp = *((uint8_t *) out_value + i);
        *((uint8_t *) out_value + i) = *((uint8_t *) out_value + length - 1 - i);
        *((uint8_t *) out_value + length - 1 - i) = temp;
    }

    return LB_READ_FILE_READ_NONE;
}

inline LB_ReadFileReadError lbReadFileReadReversed(LB_ReadFile *file, const size_t length, void *out_value) {
#ifdef LB_READ_FILE_SAFETY
    const LB_ReadFileReadError e = lbReadFileReadSafety(file, length, out_value);
    if (e) {
        return e;
    }
#endif

    return lbReadFileReadReversedUnsafe(file, length, out_value);
}

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define lbReadFileLE lbReadFileRead
#define lbReadFileBE lbReadFileReadReversed
#define lbReadFileUnsafeLE lbReadFileReadUnsafe
#define lbReadFileUnsafeBE lbReadFileReadReversedUnsafe
#else
#define lbReadFileLE lbReadFileReadReversed
#define lbReadFileBE lbReadFileRead
#define lbReadFileUnsafeLE lbReadFileReadReversedUnsafe
#define lbReadFileUnsafeBE lbReadFileReadUnsafe
#endif

inline uint8_t lbReadFileU8(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    uint8_t result;
    const LB_ReadFileReadError error = lbReadFileRead(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint8_t lbReadFileU8LE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    uint8_t result;
    const LB_ReadFileReadError error = lbReadFileLE(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint8_t lbReadFileU8BE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    uint8_t result;
    const LB_ReadFileReadError error = lbReadFileBE(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint16_t lbReadFileU16(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    uint16_t result;
    const LB_ReadFileReadError error = lbReadFileRead(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint16_t lbReadFileU16LE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    uint16_t result;
    const LB_ReadFileReadError error = lbReadFileLE(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint16_t lbReadFileU16BE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    uint16_t result;
    const LB_ReadFileReadError error = lbReadFileBE(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint32_t lbReadFileU32(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    uint32_t result;
    const LB_ReadFileReadError error = lbReadFileRead(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint32_t lbReadFileU32LE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    uint32_t result;
    const LB_ReadFileReadError error = lbReadFileLE(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint32_t lbReadFileU32BE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    uint32_t result;
    const LB_ReadFileReadError error = lbReadFileBE(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}


inline uint64_t lbReadFileU64(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    uint64_t result;
    const LB_ReadFileReadError error = lbReadFileRead(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint64_t lbReadFileU64LE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    uint64_t result;
    const LB_ReadFileReadError error = lbReadFileLE(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline uint64_t lbReadFileU64BE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    uint64_t result;
    const LB_ReadFileReadError error = lbReadFileBE(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}


inline int8_t lbReadFileI8(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    int8_t result;
    const LB_ReadFileReadError error = lbReadFileRead(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int8_t lbReadFileI8LE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    int8_t result;
    const LB_ReadFileReadError error = lbReadFileLE(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int8_t lbReadFileI8BE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    int8_t result;
    const LB_ReadFileReadError error = lbReadFileBE(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int16_t lbReadFileI16(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    int16_t result;
    const LB_ReadFileReadError error = lbReadFileRead(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int16_t lbReadFileI16LE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    int16_t result;
    const LB_ReadFileReadError error = lbReadFileLE(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int16_t lbReadFileI16BE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    int16_t result;
    const LB_ReadFileReadError error = lbReadFileBE(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int32_t lbReadFileI32(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    int32_t result;
    const LB_ReadFileReadError error = lbReadFileRead(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int32_t lbReadFileI32LE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    int32_t result;
    const LB_ReadFileReadError error = lbReadFileLE(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int32_t lbReadFileI32BE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    int32_t result;
    const LB_ReadFileReadError error = lbReadFileBE(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}


inline int64_t lbReadFileI64(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    int64_t result;
    const LB_ReadFileReadError error = lbReadFileRead(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int64_t lbReadFileI64LE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    int64_t result;
    const LB_ReadFileReadError error = lbReadFileLE(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline int64_t lbReadFileI64BE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    int64_t result;
    const LB_ReadFileReadError error = lbReadFileBE(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}


inline float lbReadFileF32(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    float result;
    const LB_ReadFileReadError error = lbReadFileRead(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline float lbReadFileF32LE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    float result;
    const LB_ReadFileReadError error = lbReadFileLE(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline float lbReadFileF32BE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    float result;
    const LB_ReadFileReadError error = lbReadFileBE(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}


inline double lbReadFileF64(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    double result;
    const LB_ReadFileReadError error = lbReadFileRead(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline double lbReadFileF64LE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    double result;
    const LB_ReadFileReadError error = lbReadFileLE(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

inline double lbReadFileF64BE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    double result;
    const LB_ReadFileReadError error = lbReadFileBE(file, sizeof(result), &result);
    #ifdef LB_READ_FILE_SAFETY
    if(out_error != NULL) {
        *out_error = error;
    }
    #endif
    return result;
}

/*
 * Normalized Integer Functions
 */

inline float lbReadFileNU8(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    return (float) lbReadFileU8(file, out_error) / (float) UINT8_MAX;
}

inline float lbReadFileNU8LE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    return lbReadFileNU8(file, out_error);
}

inline float lbReadFileNU8BE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    return lbReadFileNU8(file, out_error);
}


inline float lbReadFileNU16(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    return (float) lbReadFileU16(file, out_error) / (float) UINT16_MAX;
}

inline float lbReadFileNU16LE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    return (float) lbReadFileU16LE(file, out_error) / (float) UINT16_MAX;
}

inline float lbReadFileNU16BE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    return (float) lbReadFileU16BE(file, out_error) / (float) UINT16_MAX;
}

/*  Normalized integers utilizing more than 23 bits can carry more precision than a float.  */
inline double lbReadFileNU32(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    return (double) lbReadFileU32(file, out_error) / (double) UINT32_MAX;
}

inline double lbReadFileNU32LE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    return (double) lbReadFileU32LE(file, out_error) / (double) UINT32_MAX;
}

inline double lbReadFileNU32BE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    return (double) lbReadFileU32BE(file, out_error) / (double) UINT32_MAX;
}


inline double lbReadFileNU64(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    return (double) lbReadFileU64(file, out_error) / (double) UINT64_MAX;
}

inline double lbReadFileNU64LE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    return (double) lbReadFileU64LE(file, out_error) / (double) UINT64_MAX;
}

inline double lbReadFileNU64BE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    return (double) lbReadFileU64BE(file, out_error) / (double) UINT64_MAX;
}


inline float lbReadFileNI8(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    return (float) lbReadFileI8(file, out_error) / (float) INT8_MAX;
}

inline float lbReadFileNI8LE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    return lbReadFileNI8(file, out_error);
}

inline float lbReadFileNI8BE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    return lbReadFileNI8(file, out_error);
}


inline float lbReadFileNI16(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    return (float) lbReadFileI16(file, out_error) / (float) INT16_MAX;
}

inline float lbReadFileNI16LE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    return (float) lbReadFileI16LE(file, out_error) / (float) INT16_MAX;
}

inline float lbReadFileNI16BE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    return (float) lbReadFileI16BE(file, out_error) / (float) INT16_MAX;
}


inline double lbReadFileNI32(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    return (double) lbReadFileI32(file, out_error) / (double) INT32_MAX;
}

inline double lbReadFileNI32LE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    return (double) lbReadFileI32LE(file, out_error) / (double) INT32_MAX;
}

inline double lbReadFileNI32BE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    return (double) lbReadFileI32BE(file, out_error) / (double) INT32_MAX;
}


inline double lbReadFileNI64(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    return (double) lbReadFileI64(file, out_error) / (double) INT64_MAX;
}

inline double lbReadFileNI64LE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    return (double) lbReadFileI64LE(file, out_error) / (double) INT64_MAX;
}

inline double lbReadFileNI64BE(LB_ReadFile *file, LB_ReadFileReadError* out_error) {
    return (double) lbReadFileI64BE(file, out_error) / (double) INT64_MAX;
}
#ifdef __cplusplus
}
#endif
#endif

#ifdef LB_READ_FILE_IMPLEMENTATION
#ifdef __cplusplus
extern "C" {
#endif
LB_ReadFileInitError lbReadFileInit(LB_ReadFile *file, FILE *data);
#ifdef LB_READ_FILE_SAFETY
LB_ReadFileReadError lbReadFileReadSafety(const LB_ReadFile *file, size_t length, const void* out_value);
#endif
LB_ReadFileReadError lbReadFileReadUnsafe(LB_ReadFile *file, size_t length, void* out_value);
LB_ReadFileReadError lbReadFileRead(LB_ReadFile *file, size_t length, void* out_value);
LB_ReadFileReadError lbReadFileReadReversedUnsafe(LB_ReadFile *file, size_t length, void* out_value);
LB_ReadFileReadError lbReadFileReadReversed(LB_ReadFile *file, size_t length, void* out_value);
uint8_t lbReadFileU8(LB_ReadFile *file, LB_ReadFileReadError* out_error);
uint8_t lbReadFileU8LE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
uint8_t lbReadFileU8BE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
uint16_t lbReadFileU16(LB_ReadFile *file, LB_ReadFileReadError* out_error);
uint16_t lbReadFileU16LE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
uint16_t lbReadFileU16BE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
uint32_t lbReadFileU32(LB_ReadFile *file, LB_ReadFileReadError* out_error);
uint32_t lbReadFileU32LE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
uint32_t lbReadFileU32BE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
uint64_t lbReadFileU64(LB_ReadFile *file, LB_ReadFileReadError* out_error);
uint64_t lbReadFileU64LE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
uint64_t lbReadFileU64BE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
int8_t lbReadFileI8(LB_ReadFile *file, LB_ReadFileReadError* out_error);
int8_t lbReadFileI8LE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
int8_t lbReadFileI8BE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
int16_t lbReadFileI16(LB_ReadFile *file, LB_ReadFileReadError* out_error);
int16_t lbReadFileI16LE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
int16_t lbReadFileI16BE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
int32_t lbReadFileI32(LB_ReadFile *file, LB_ReadFileReadError* out_error);
int32_t lbReadFileI32LE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
int32_t lbReadFileI32BE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
int64_t lbReadFileI64(LB_ReadFile *file, LB_ReadFileReadError* out_error);
int64_t lbReadFileI64LE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
int64_t lbReadFileI64BE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
float lbReadFileF32(LB_ReadFile *file, LB_ReadFileReadError* out_error);
float lbReadFileF32LE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
float lbReadFileF32BE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
double lbReadFileF64(LB_ReadFile *file, LB_ReadFileReadError* out_error);
double lbReadFileF64LE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
double lbReadFileF64BE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
float lbReadFileNU8(LB_ReadFile *file, LB_ReadFileReadError* out_error);
float lbReadFileNU8LE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
float lbReadFileNU8BE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
float lbReadFileNU16(LB_ReadFile *file, LB_ReadFileReadError* out_error);
float lbReadFileNU16LE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
float lbReadFileNU16BE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
double lbReadFileNU32(LB_ReadFile *file, LB_ReadFileReadError* out_error);
double lbReadFileNU32LE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
double lbReadFileNU32BE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
double lbReadFileNU64(LB_ReadFile *file, LB_ReadFileReadError* out_error);
double lbReadFileNU64LE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
double lbReadFileNU64BE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
float lbReadFileNI8(LB_ReadFile *file, LB_ReadFileReadError* out_error);
float lbReadFileNI8LE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
float lbReadFileNI8BE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
float lbReadFileNI16(LB_ReadFile *file, LB_ReadFileReadError* out_error);
float lbReadFileNI16LE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
float lbReadFileNI16BE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
double lbReadFileNI32(LB_ReadFile *file, LB_ReadFileReadError* out_error);
double lbReadFileNI32LE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
double lbReadFileNI32BE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
double lbReadFileNI64(LB_ReadFile *file, LB_ReadFileReadError* out_error);
double lbReadFileNI64LE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
double lbReadFileNI64BE(LB_ReadFile *file, LB_ReadFileReadError* out_error);
#endif

#ifdef __cplusplus
}
#endif

