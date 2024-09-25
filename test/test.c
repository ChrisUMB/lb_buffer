#include "lb_write_buffer.h"

#define A_B 1

#define SUFFIX _B

#define X(v) (v)
#define TEST X(A##SUFFIX)
#if TEST == 1
#error "HEY"
#endif

int main(int argc, char **argv) {

}
