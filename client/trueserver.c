/****************************************************************************************
MIT License

Copyright (c) 2019 Plan C

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/

#include "trueserver.h"
#include "proxyserver.h"

/*********************************************************
 * Function     : Callback function while get new connection
 *                from true server
 * Parameters   : tcp - uv_stream_t*
 *                status - status of connection
 * Return       : void
*********************************************************/
void true_server_proxy_connected(uv_connect_t* req, int status)
{
    if(status < 0)
    {
        log_printf(LOG_ERROR, "Connection error : %s.", uv_strerror(status));
        return;
    }

    data_proxy_t* data_proxy = req->data;
    data_control_t* data_control = data_proxy->data_control;
    struct sockaddr_in addr;
    int len = sizeof(addr);
    uv_tcp_getsockname((uv_tcp_t*)req->handle, (struct sockaddr*)&addr, &len);
    
    data_proxy->partner = tcpmap_get_first_key(data_control->idle_tcp);
    if(data_proxy->partner == NULL)
    {
        log_printf(LOG_INFO, "Connected proxy connection to true server %s:%d by %s:%d but no idle true server connection.",
                    inet_ntoa(data_control->true_server_addr.sin_addr), 
                    htons(data_control->true_server_addr.sin_port),
                    inet_ntoa(addr.sin_addr), htons(addr.sin_port));
    }
    else
    {
        log_printf(LOG_INFO, "Connected proxy connection to true server %s:%d by %s:%d.",
                    inet_ntoa(data_control->true_server_addr.sin_addr), 
                    htons(data_control->true_server_addr.sin_port),
                    inet_ntoa(addr.sin_addr), htons(addr.sin_port));

        // remove from waiting queue
        tcpmap_remove(data_control->idle_tcp, data_proxy->partner);

        // bind partners
        data_proxy_t* partnerdata_proxy = data_proxy->partner->data;
        partnerdata_proxy->partner = (uv_tcp_t*)req->handle;

        // registe read
        uv_read_start((uv_stream_t*)(req->handle), allocer, true_server_proxy_read);
        uv_read_start((uv_stream_t*)(data_proxy->partner), allocer, proxy_server_proxy_read);
    }
    
    free(req);
}


/*********************************************************
 * Function     : Callback function while read data
 *                from true server
 * Parameters   : tcp - uv_stream_t*
 *                status - status of connection
 * Return       : void
*********************************************************/
void true_server_proxy_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
    data_proxy_t* data_proxy = stream->data;
    data_control_t* data_control = data_proxy->data_control;
    if(nread < 0 || nread == UV_EOF)
    {
        if(nread == UV_EOF) // disconnected
        {
            log_printf(LOG_INFO, "Loss proxy connection to true server %s:%d.",
                            inet_ntoa(data_control->proxy_server_addr.sin_addr), 
                            htons(data_control->proxy_server_addr.sin_port));
        }
        else // error
        {
            log_printf(LOG_ERROR, "Exceptionally loss proxy connection to true server %s:%d.",
                            inet_ntoa(data_control->proxy_server_addr.sin_addr), 
                            htons(data_control->proxy_server_addr.sin_port));
        }
        
        uv_close((uv_handle_t*)(stream), free_with_data);
        if(data_proxy->partner != NULL)
        {
            uv_close((uv_handle_t*)(data_proxy->partner), free_with_data);
            tcpmap_remove(data_control->all_tcp, (uv_tcp_t*)(data_proxy->partner));
        }
        free(buf->base);
    }
    else if(data_proxy == NULL)
    {
        log_printf(LOG_ERROR, "Get data but not partner.");
        free(buf->base);
        return;
    }
    else
    {
        uv_write_t* req = malloc(sizeof(uv_write_t));
        if(req == NULL)
        {
            log_printf(LOG_ERROR, "Malloc %d bytes error.", sizeof(uv_write_t));
            return;
        }
        uv_buf_t* new_buf = malloc(sizeof(uv_buf_t));
        if(new_buf == NULL)
        {
            free(req);
            log_printf(LOG_ERROR, "Malloc %d bytes error.", sizeof(uv_buf_t));
            return;
        }
        new_buf->base = buf->base;
        new_buf->len = nread;
        req->data = new_buf;
        uv_write(req, (uv_stream_t*)data_proxy->partner, new_buf, 1, true_server_proxy_written);
    }
}


/*********************************************************
 * Function     : Callback function while written data to
 *                proxy server
 * Parameters   : req - uv_write_t*
 *                status - status of writing
 * Return       : void
*********************************************************/
void true_server_proxy_written(uv_write_t* req, int status)
{
    if(status < 0)
    {
        log_printf(LOG_ERROR, "Writting error : %s.", uv_strerror(status));
        return;
    }
    uv_buf_t* buf = req->data;
    free(buf->base);
    free(buf);
    free(req);
}