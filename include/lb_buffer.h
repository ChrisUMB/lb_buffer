#ifndef LB_BUFFER_H
#define LB_BUFFER_H

#ifdef LB_BUFFER_IMPLEMENTATION
#define LB_WRITE_BUFFER_IMPLEMENTATION
#define LB_READ_BUFFER_IMPLEMENTATION
#endif

#ifdef LB_BUFFER_NO_SAFETY
#define LB_WRITE_BUFFER_NO_SAFETY
#define LB_READ_BUFFER_NO_SAFETY
#endif

#include "lb_write_buffer.h"
#include "lb_read_buffer.h"

#endif //LB_BUFFER_H
