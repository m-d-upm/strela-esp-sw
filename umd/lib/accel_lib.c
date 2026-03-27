#include "accel_lib.h"

int accel_lib_alloc(struct accel_shbuf_desc* desc, size_t size)
{
    int dmabuf_heap_fd;

    if(!desc || (size == 0))
    {
        return -1;
    }

    dmabuf_heap_fd = strela_dmabuf_open();

    if(dmabuf_heap_fd < 0)
    {
        return -1;
    }

    desc->dmabuf_fd = dmabuf_alloc(dmabuf_heap_fd, size);

    if(desc->dmabuf_fd < 0)
    {
        goto error_buff_alloc;
    }

    desc->buff_mmap_ptr = strela_dmabuf_mmap(buffer_fd, size);

    if(!desc->buff_mmap_ptr)
    {
        goto error_mmap;
    }

    dmabuf_close(dmabuf_heap_fd);
    desc->buff_size = size;

    return 0;

error_mmap:
    dmabuf_free(desc->dmabuf_fd);
error_buff_alloc:
    dmabuf_close(dmabuf_heap_fd);

    return -1;
}

void accel_lib_dealloc(struct accel_shbuf_desc* desc)
{
    if(!desc)
    {
        return;
    }

    if(!desc->buff_mmap_ptr)
    {
        return;
    }

    dmabuf_unmap(desc->buff_mmap_ptr, desc->buff_size);

    dmabuf_free(desc->dmabuf_fd);

    desc->buff_mmap_ptr = NULL;
    desc->dmabuf_fd = -1;
    desc->buff_size = 0;
}

int accel_lib_attach_buf_to_dev(struct accel_shbuf_desc* desc, int accel_fd, enum accel_shbuf_dir direction)
{
    if(accel_fd < 0)
    {
        return -1;
    }

    if(!desc)
    {
        return -1;
    }

    if(!desc->accel_buf_attach)
    {
        return -1;
    }

    if(desc->accel_buf_attach(accel_fd, direction) < 0)
    {
        return -1;
    }

    if(direction == ACCEL_SHBUF_DIR_IN)
    {
        desc->accel_dev_fd_in = accel_fd;
    }
    else
    {
        desc->accel_dev_fd_out = accel_fd;
    }

    return 0;
}

int accel_lib_detach_buf_from_dev(struct accel_shbuf_desc* desc, enum accel_shbuf_dir direction)
{
    if(!desc)
    {
        return -1;
    }

    if(desc->accel_dev_fd == -1)
    {
        return -1;
    }

    if(!desc->accel_buf_detach)
    {
        return -1;
    }

    if(desc->accel_buf_detach(desc->accel_dev_fd, direction) < 0)
    {
        return -1;
    }

    if(direction == ACCEL_SHBUF_DIR_IN)
    {
        desc->accel_dev_fd_in = -1;
    }
    else
    {
        desc->accel_dev_fd_out = -1;
    }

    return 0;
}
