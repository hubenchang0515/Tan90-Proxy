#include "common.h" 

void allocer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    buf->base = (char*) malloc(suggested_size);
    if(buf->base == NULL)
    {
        log_printf(LOG_ERROR, "Malloc %s bytes error.", suggested_size);
        buf->len = 0;
    }
    else
    {
        buf->len = suggested_size;
    }
}

void free_self(uv_handle_t *handle)
{
    free(handle);
}

void free_with_data(uv_handle_t *handle)
{
    free(handle->data);
    free(handle);
}

