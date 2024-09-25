#ifndef LB_FILE_H
#define LB_FILE_H

#ifdef LB_FILE_IMPLEMENTATION
#define LB_WRITE_FILE_IMPLEMENTATION
#define LB_READ_FILE_IMPLEMENTATION
#endif

#ifdef LB_FILE_NO_SAFETY
#define LB_WRITE_FILE_NO_SAFETY
#define LB_READ_FILE_NO_SAFETY
#endif

#include "lb_write_file.h"
#include "lb_read_file.h"

#endif //LB_FILE_H
