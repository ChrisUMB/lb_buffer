#ifndef LB_BUFFER_H
#define LB_BUFFER_H

#ifdef LB_BUFFER_IMPLEMENTATION
#define LB_WRITER_IMPLEMENTATION
#define LB_READER_IMPLEMENTATION
#endif

#ifdef LB_BUFFER_NO_SAFETY
#define LB_WRITER_NO_SAFETY
#define LB_READER_NO_SAFETY
#endif

#include "lb_writer.h"
#include "lb_reader.h"

#endif //LB_BUFFER_H
