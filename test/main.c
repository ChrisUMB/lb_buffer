#include <stdbool.h>
#include <stdio.h>

#define LB_BUFFER_IMPLEMENTATION
#include "lb_buffer.h"

#define LB_PAGED_ARENA_IMPLEMENTATION
#include "lb_paged_arena.h"

#include <stdlib.h>
#include <math.h>

int main(int argc, char *argv[]) {
    // LB_Reader reader;
    // // FILE* file = fopen("test.bin", "rb");
    // // lbReadInitFile(&reader, file);
    // float value = 3.14159265359f;
    // lbReadInitBuffer(&reader, sizeof(float), &value);
    //
    // float f32 = lbReadF32(&reader, NULL);
    // printf("F32: %f\n", f32);

    LB_Writer writer;
    FILE* file = fopen("test.bin", "wb");
    lbWriterInitFile(&writer, file);

    // const float value = 3.14159265359f;
    // lbWriteF32(&writer, value);
    // lbWriterSeek(&writer, 16);
    // lbWriteF32(&writer, value*2);
    for(int i = 0; i < 32; i++) {
        lbWriteI32(&writer, i << 1);
    }

    fclose(file);
    LB_Reader reader;
    file = fopen("test.bin", "rb");
    lbReaderInitFile(&reader, file);

    // float received = lbReadF32(&reader, NULL);
    // printf("Received: %f\n", received);
    // printf("Equal: %s\n", value == received ? "true" : "false");
    //
    // lbReaderSeek(&reader, 16);
    // received = lbReadF32(&reader, NULL);
    // printf("Received: %f\n", received);
    // printf("Equal: %s\n", value*2 == received ? "true" : "false");

    for(int i = 0; i < 32; i++) {
        const int32_t received = lbReadI32(&reader, NULL);
        printf("Received: %d\n", received);
        printf("Equal: %s\n", (i << 1) == received ? "true" : "false");
    }

    fclose(file);
}

int mahin(int argc, char *argv[]) {
    LB_PagedArena *arena = lbPagedArenaNew(64);
    if (!arena) {
        printf("Failed to create paged arena\n");
        return 1;
    }

    for(int i = 0; i < 10; i++) {
        void* data = lbPagedArenaAlloc(arena, 32);
        if (!data) {
            printf("(%d) Failed to allocate memory\n", i);
            return 1;
        }

        printf("(%d) Allocated memory at %p\n", i, data);
    }

    return 0;
}
//
//
// int maign(int argc, char *argv[]) {
//     char data[1024] = {0};
//
//     LB_WriteBuffer buffer;
//     const int error = lbWriteBufferInit(&buffer, 1024, data);
//
//     if (error) {
//         printf("Failed to initialize write buffer: %d\n", error);
//         return 1;
//     }
//
//     // for (int i = 0; i < 1024; i++) {
//     //     const int e = lbWriteBufferU8(&buffer, i % 256);
//     //     if (e) {
//     //         printf("Failed to write U8: %d (%d)\n", e, i);
//     //     }
//     // }
//
//     if(lbWriteBufferF32(&buffer, 3.14159265359f)) {
//         printf("Failed to write F32\n");
//     }
//     if(lbWriteBufferNU8(&buffer, 0.5f)) {
//         printf("Failed to write NU8\n");
//     }
//     if(lbWriteBufferNI8(&buffer, -0.5f)) {
//         printf("Failed to write NI8\n");
//     }
//     if(lbWriteBufferNI16(&buffer, 0.14159265359f)) {
//         printf("Failed to write NI8\n");
//     }
//     if(lbWriteBufferI32BE(&buffer, 0x12345678)) {
//         printf("Failed to write I32BE\n");
//     }
//
//     LB_ReadBuffer reader;
//     lbReadBufferInit(&reader, 1024, buffer.data);
//     if(lbReadBufferF32(&reader, NULL) != 3.14159265359f) {
//         printf("Failed to read F32\n");
//     }
//     if(fabsf(lbReadBufferNU8(&reader, NULL) - 0.5f) > 0.01f) {
//         printf("Failed to read NU8\n");
//     }
//     if(fabsf(lbReadBufferNI8(&reader, NULL) - (-0.5f)) > 0.01f) {
//         printf("Failed to read NI8\n");
//     }
//     if(fabsf(lbReadBufferNI16(&reader, NULL) - 0.14159265359f) > 0.01f) {
//         printf("Failed to read NI16\n");
//     }
//     if(lbReadBufferI32BE(&reader, NULL) != 0x12345678) {
//         printf("Failed to read I32BE\n");
//     }
//
//     FILE *file;
//     fopen_s(&file, "test.bin", "wb");
//
//     if (!file) {
//         printf("Failed to open test.bin for writing: %d\n", error);
//         return 1;
//     }
//
//     LB_WriteFile writeFile;
//     lbWriteFileInit(&writeFile, file);
//
//     if(lbWriteFileF32(&writeFile, 3.14159265359f)) {
//         printf("Failed to write F32\n");
//     }
//     if(lbWriteFileNU8(&writeFile, 0.5f)) {
//         printf("Failed to write NU8\n");
//     }
//     if(lbWriteFileNI8(&writeFile, -0.5f)) {
//         printf("Failed to write NI8\n");
//     }
//     if(lbWriteFileNI16(&writeFile, 0.14159265359f)) {
//         printf("Failed to write NI8\n");
//     }
//     if(lbWriteFileI32BE(&writeFile, 0x12345678)) {
//         printf("Failed to write I32BE\n");
//     }
//
//     fclose(file);
//
//     fopen_s(&file, "test.bin", "rb");
//     LB_ReadFile readFile;
//     lbReadFileInit(&readFile, file);
//
//     if(lbReadFileF32(&readFile, NULL) != 3.14159265359f) {
//         printf("Failed to read F32\n");
//     }
//     if(fabsf(lbReadFileNU8(&readFile, NULL) - 0.5f) > 0.01f) {
//         printf("Failed to read NU8\n");
//     }
//     if(fabsf(lbReadFileNI8(&readFile, NULL) - (-0.5f)) > 0.01f) {
//         printf("Failed to read NI8\n");
//     }
//     if(fabsf(lbReadFileNI16(&readFile, NULL) - 0.14159265359f) > 0.01f) {
//         printf("Failed to read NI16\n");
//     }
//     if(lbReadFileI32BE(&readFile, NULL) != 0x12345678) {
//         printf("Failed to read I32BE\n");
//     }
//
//     fclose(file);
//
//     return 0;
// }
