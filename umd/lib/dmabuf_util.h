#ifndef DMABUF_UTIL_H
#define DMABUF_UTIL_H

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/dma-heap.h>
#include <unistd.h>

enum accel_shbuf_dir {
    ACCEL_SHBUF_DIR_IN = 0,
    ACCEL_SHBUF_DIR_OUT = 0
};

struct accel_shbuf_desc {
    int dmabuf_fd;
    int accel_dev_fd_in;
    int accel_dev_fd_out;
    void* buff_mmap_ptr;
    size_t buff_size;
    int *accel_buf_attach(int, enum accel_shbuf_dir);
    int *accel_buf_detach(int, enum accel_shbuf_dir);
};

int dmabuf_open();
void dmabuf_close(int fd);
int dmabuf_alloc(int fd, size_t size);
void* dmabuf_mmap(int fd, size_t size);
void dmabuf_unmap(void* ptr, size_t size);
void dmabuf_free(int fd);
int dmabuf_sync_start(int fd);
int dmabuf_sync_end(int fd);

#endif // DMABUF_UTIL_H
