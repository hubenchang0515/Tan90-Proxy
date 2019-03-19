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

#include "proxyserver.h"
#include "trueserver.h"

/*********************************************************
 * Function     : Create control connection to proxy server
 * Parameters   : req - uv_connect_t*
 *                status - status of connection
 * Return       : void
*********************************************************/
void proxy_server_control_connected(uv_connect_t* req, int status)
{
    if(status < 0)
    {
        log_printf(LOG_ERROR, "Connection error : %s.", uv_strerror(status));
        return;
    }
    
    data_control_t* data_control = req->handle->data;
    struct sockaddr_in addr;
    int len = sizeof(addr);
    uv_tcp_getsockname((uv_tcp_t*)req->handle, (struct sockaddr*)&addr, &len);

    if(data_control->control == NULL) // control connection 
    {
        data_control->control = (uv_tcp_t*)req->handle;
        
        log_printf(LOG_INFO, "Connected control connection to %s:%d by %s:%d.",
                    inet_ntoa(data_control->proxy_server_addr.sin_addr), 
                    htons(data_control->proxy_server_addr.sin_port),
                    inet_ntoa(addr.sin_addr), htons(addr.sin_port));

        /* registe read */
        uv_read_start((uv_stream_t*)req->handle, allocer, proxy_server_control_read); 
    }
    else // proxy connection
    {
        log_printf(LOG_ERROR, "Connected control connection to %s:%d by %s:%d but control connection is exist.");
    }
    free(req);
}



/*********************************************************
 * Function     : Create proxy connection to proxy server
 * Parameters   : req - uv_connect_t*
 *                status - status of connection
 * Return       : void
*********************************************************/
void proxy_server_proxy_connected(uv_connect_t* req, int status)
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

    if(data_control->control == NULL)
    {
        log_printf(LOG_ERROR, "Connected proxy connection to proxy server %s:%d by %s:%d but no control connection.",
                    inet_ntoa(data_control->proxy_server_addr.sin_addr), 
                    htons(data_control->proxy_server_addr.sin_port),
                    inet_ntoa(addr.sin_addr), htons(addr.sin_port));
    }
    else
    {
        log_printf(LOG_INFO, "Connected proxy connection to proxy server %s:%d by %s:%d.",
                    inet_ntoa(data_control->proxy_server_addr.sin_addr), 
                    htons(data_control->proxy_server_addr.sin_port),
                    inet_ntoa(addr.sin_addr), htons(addr.sin_port));
        
        // store
        tcpmap_set(data_control->all_tcp, (uv_tcp_t*)(req->handle), NULL);
        tcpmap_set(data_control->idle_tcp, (uv_tcp_t*)(req->handle), NULL);

        // connect to true server
        uv_connect_t* connect_req = malloc(sizeof(uv_connect_t));
            if(connect_req == NULL)
            {
                log_printf(LOG_ERROR, "Malloc %d bytes error.", sizeof(uv_connect_t));
                return;
            }
            data_proxy_t* new_data_proxy = malloc(sizeof(data_proxy_t));
            if(new_data_proxy == NULL)
            {
                free(connect_req);
                log_printf(LOG_ERROR, "Malloc %d bytes error.", sizeof(data_proxy_t));
                return;
            }
            uv_tcp_t* true_server = malloc(sizeof(uv_tcp_t));
            if(true_server == NULL)
            {
                free(new_data_proxy);
                free(connect_req);
                log_printf(LOG_ERROR, "Malloc %d bytes error.", sizeof(data_proxy_t));
                return;
            }
            uv_tcp_init(loop, true_server);

            new_data_proxy->req = connect_req;
            new_data_proxy->data_control = data_control;
            new_data_proxy->partner = NULL;
            connect_req->data = new_data_proxy;
            true_server->data = new_data_proxy;
            uv_tcp_connect(connect_req, true_server, 
                            (const struct sockaddr *)&(data_control->true_server_addr), 
                            true_server_proxy_connected);

            // registe read callback
            //uv_read_start((uv_stream_t*)(req->handle), allocer, proxy_server_proxy_read);
    }
    
    free(req);
}


/*********************************************************
 * Function     : Read from control connection
 * Parameters   : stream - uv_stream_t*
 *                nread - how many bytes read
 *                buf - data
 * Return       : void
*********************************************************/
void proxy_server_control_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
    data_control_t* data = stream->data;
    if(nread < 0 || nread == EOF) // loss control connect
    {
        if(nread == UV_EOF)
        {
            log_printf(LOG_INFO, "Loss control connection from %s:%d.",
                            inet_ntoa(data->proxy_server_addr.sin_addr), 
                            htons(data->proxy_server_addr.sin_port));
        }
        else
        {
            log_printf(LOG_ERROR, "Exceptionally loss control connection from %s:%d.",
                            inet_ntoa(data->proxy_server_addr.sin_addr), 
                            htons(data->proxy_server_addr.sin_port));
        }

        /* Disconnect all proxy tcp */
        guint length = 0;
        uv_tcp_t** keys = tcpmap_get_all_keys(data->all_tcp, &length);
        for(guint i = 0; i < length; i++)
        {
            uv_tcp_t* proxy_client_proxy = keys[i];
            data_proxy_t* data_proxy = proxy_client_proxy->data;

            /* disconnect */
            log_printf(LOG_INFO, "Disconnect proxy connection to proxy server %s:%d.",
                        inet_ntoa(data_proxy->data_control->proxy_server_addr.sin_addr), 
                        htons(data_proxy->data_control->proxy_server_addr.sin_port));
            uv_close((uv_handle_t*)proxy_client_proxy, free_with_data);
            if(data_proxy->partner != NULL)
            {
                uv_close((uv_handle_t*)(data_proxy->partner), free_with_data);
            }
        }
        g_free(keys);
        tcpmap_clear(data->all_tcp);

        /* Reset */
        data->control = NULL;

        /* close : 
            control connection uv_tcp_t was defined in main
            and will be reused so not free */
        uv_close((uv_handle_t*)(stream), NULL);
    }
    else
    {
        switch((unsigned char)buf->base[0])
        {
        case CMD_NEW_PROXY:
            log_printf(LOG_INFO, "Get command CMD_NEW_PROXY(%d).", (unsigned char)buf->base[0]);
            uv_connect_t* connect_req = malloc(sizeof(uv_connect_t));
            if(connect_req == NULL)
            {
                log_printf(LOG_ERROR, "Malloc %d bytes error.", sizeof(uv_connect_t));
                return;
            }
            data_proxy_t* data_proxy = malloc(sizeof(data_proxy_t));
            if(data_proxy == NULL)
            {
                free(connect_req);
                log_printf(LOG_ERROR, "Malloc %d bytes error.", sizeof(data_proxy_t));
                return;
            }
            uv_tcp_t* proxy_server = malloc(sizeof(uv_tcp_t));
            if(proxy_server == NULL)
            {
                free(data_proxy);
                free(connect_req);
                log_printf(LOG_ERROR, "Malloc %d bytes error.", sizeof(data_proxy_t));
                return;
            }
            uv_tcp_init(loop, proxy_server);

            /* connect to proxy server */
            data_proxy->req = connect_req;
            data_proxy->data_control = data;
            data_proxy->partner = NULL;
            connect_req->data = data_proxy;
            proxy_server->data = data_proxy;
            uv_tcp_connect(connect_req, proxy_server, 
                            (const struct sockaddr *)&(data->proxy_server_addr), 
                            proxy_server_proxy_connected);
            
            break;

        default:
            log_printf(LOG_ERROR, "Unsupported control command 0x%x.", 
                        (unsigned char)buf->base[0]);
        }
    }
    
}


/*********************************************************
 * Function     : Read data from proxy server
 * Parameters   : stream - uv_stream_t*
 *                nread - how many bytes read
 *                buf - data
 * Return       : void
*********************************************************/
void proxy_server_proxy_read(uv_stream_t* stream, ssize_t nread, const uv_buf_t* buf)
{
    data_proxy_t* data_proxy = stream->data;
    data_control_t* data_control = data_proxy->data_control;
    if(nread < 0 || nread == UV_EOF)
    {
        if(nread == UV_EOF) // disconnected
        {
            log_printf(LOG_INFO, "Loss proxy connection to proxy server %s:%d.",
                            inet_ntoa(data_control->proxy_server_addr.sin_addr), 
                            htons(data_control->proxy_server_addr.sin_port));
        }
        else // error
        {
            log_printf(LOG_ERROR, "Exceptionally loss control connection from %s:%d.",
                            inet_ntoa(data_control->proxy_server_addr.sin_addr), 
                            htons(data_control->proxy_server_addr.sin_port));
        }
        
        uv_close((uv_handle_t*)(stream), free_with_data);
        if(data_proxy->partner != NULL)
        {
            uv_close((uv_handle_t*)(data_proxy->partner), free_with_data);
        }
        tcpmap_remove(data_control->idle_tcp, (uv_tcp_t*)(stream));
        tcpmap_remove(data_control->all_tcp, (uv_tcp_t*)(stream));
    }
    else if(data_proxy->partner == NULL)
    {
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
        uv_write(req, (uv_stream_t*)data_proxy->partner, new_buf, 1, proxy_server_proxy_written);
    }
}


/*********************************************************
 * Function     : Callback function while written data to
 *                true server
 * Parameters   : req - uv_write_t*
 *                status - status of writing
 * Return       : void
*********************************************************/
void proxy_server_proxy_written(uv_write_t* req, int status)
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