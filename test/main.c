#include <stdbool.h>
#include <stdio.h>

#define LB_BUFFER_IMPLEMENTATION
// #define LB_BUFFER_NO_SAFETY
#include <stdlib.h>

#include "lb_buffer.h"

int main(int argc, char *argv[]) {
    char data[1024] = {0};

    LB_WriteBuffer buffer;
    const int error = lbWriteBufferInit(&buffer, data, 1024);

    if (error) {
        printf("Failed to initialize write buffer: %d\n", error);
        return 1;
    }

    for (int i = 0; i < 1024; i++) {
        const int e = lbWriteBufferU8(&buffer, i % 256);
        if (e) {
            printf("Failed to write U8: %d (%d)\n", e, i);
        }
    }

    printf("Buffer position: %llu\n", buffer.position);

    FILE *file;
    fopen_s(&file, "test.bin", "wb");

    if (!file) {
        printf("Failed to open test.bin for writing: %d\n", error);
        return 1;
    }

    fwrite(buffer.data, 1024, 1, file);

    fclose(file);

    return 0;
}
