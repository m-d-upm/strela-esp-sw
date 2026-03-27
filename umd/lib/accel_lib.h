#ifndef ACCEL_LIB_H
#define ACCEL_LIB_H

#include "dmabuf_util.h"

int accel_lib_alloc(struct accel_shbuf_desc* desc, size_t size);
void accel_lib_dealloc(struct accel_shbuf_desc* desc);
int accel_lib_attach_buf_to_dev(struct accel_shbuf_desc* desc, int accel_fd, enum accel_shbuf_dir direction);
int accel_lib_detach_buf_from_dev(struct accel_shbuf_desc* desc, enum accel_shbuf_dir direction);

#endif // ACCEL_LIB_H
