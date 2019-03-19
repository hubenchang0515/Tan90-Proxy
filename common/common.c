#include "common.h" 

void allocer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    (void)handle;
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

const char* filename(const char* path, char* output, size_t size)
{
    const char* p = path + strlen(path) - 1;
	for(; p!= path && *p != '/' && *p != '\\'; p--);
	
	p = p == path ? path : p + 1;
	strncpy(output, p, size-1);
	output[size-1] = 0;

	return output;
}