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


#include "common.h"
#include "tcpto.h"


void tcp_to_has_connection(uv_stream_t* tcp, int status)
{
    if(status < 0)
    {
        log_printf(LOG_ERROR, "Connection error : %s.", uv_strerror(status));
        return;
    }

    /* accept connection */
    uv_tcp_t* connection = malloc(sizeof(uv_tcp_t));
    if(connection == NULL)
    {
        log_printf(LOG_ERROR, "Malloc %s bytes error.", sizeof(uv_tcp_t));
        return;
    }
    uv_tcp_init(loop, connection);
    uv_accept(tcp, (uv_stream_t*) connection);

    data_control_t* data_control = tcp->data;

    /* Get control connection */
    if(data_control->control == NULL) // [WARNING] Use tcp->data as connection for control
    {
        data_control->control = connection;
        connection->data = data_control;
        /* regist read call-back */
        uv_read_start((uv_stream_t*)connection, allocer, tcp_to_control_can_read); 
        /* print ip:port */
        int len = sizeof(data_control->addr);
        uv_tcp_getpeername(connection, (struct sockaddr*)&(data_control->addr), &len);
        log_printf(LOG_INFO, "Get control connection from %s:%d.",
                    inet_ntoa(data_control->addr.sin_addr), htons(data_control->addr.sin_port));
    }
    else // proxy connection
    {
        /* bind with idle queue */
        data_proxy_t* data_proxy = malloc(sizeof(data_proxy_t));
        if(data_proxy == NULL)
        {
            log_printf(LOG_ERROR, "Malloc %s bytes error.", sizeof(data_proxy_t));
            return;
        }
        
        data_proxy->partner = tcpmap_pop_idle(data_control->idle_queue);
        connection->data = data_proxy;
        /* print ip:port */
        int len = sizeof(struct sockaddr_in);
        uv_tcp_getpeername(connection, (struct sockaddr*)&(data_proxy->addr), &len);
        if(data_proxy->partner == NULL)
        {
            log_printf(LOG_ERROR, "Get connection from %s:%d but no idle connection.",
                        inet_ntoa(data_proxy->addr.sin_addr), htons(data_proxy->addr.sin_port));

            /* close and free */
            uv_close((uv_handle_t*)connection, free_self);
            return;
        }
        else
        {
            log_printf(LOG_INFO, "Get proxy connection from %s:%d.",
                        inet_ntoa(data_proxy->addr.sin_addr), htons(data_proxy->addr.sin_port));

            /* store to map */
            tcpmap_set(data_control->all_tcp, connection, data_proxy->partner);
        }

        /* regist read call-back */
        uv_read_start((uv_stream_t*)connection, allocer, tcp_to_proxy_can_read);
    }
}


void tcp_to_control_can_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
    data_control_t* data = stream->data;
    if(nread < 0 && nread != UV_EOF) // error
    {

    }
    else if(nread == UV_EOF) // has no data (means disconnected)
    {
        log_printf(LOG_INFO, "Loss control connection from %s:%d.",
                        inet_ntoa(data->addr.sin_addr), htons(data->addr.sin_port));

        /* Disconnect all proxy tcp */
        guint length = 0;
        uv_tcp_t** keys = (uv_tcp_t**)g_hash_table_get_keys_as_array(data->all_tcp, &length);
        for(guint i = 0; i < length; i++)
        {
            uv_tcp_t* tcp_to_proxy = keys[i];
            data_proxy_t* data_proxy = tcp_to_proxy->data;

            /* disconnect */
            log_printf(LOG_INFO, "Disconnect proxy connection from %s:%d.",
                        inet_ntoa(data_proxy->addr.sin_addr), htons(data_proxy->addr.sin_port));
            uv_close((uv_handle_t*)(data_proxy->partner), free_self);
            uv_close((uv_handle_t*)tcp_to_proxy, free_with_data);
        }
        g_free(keys);

        /* close and free */
        //free(stream);
        

        /* Reset  */
        data->control = NULL;
    }
    else if(nread > 0 && nread < buf->len) // read completely
    {

    }
    else // read uncompletely
    {
        
    }


    free(buf->base);
}


void tcp_to_proxy_can_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
    data_proxy_t* data = stream->data;
    data_control_t* data_control = data->data_control;
    if(nread < 0 && nread != UV_EOF) // error
    {

    }
    else if(nread == UV_EOF) // has no data (means disconnected)
    {
        log_printf(LOG_INFO, "Loss proxy connection from %s:%d.",
                        inet_ntoa(data->addr.sin_addr), htons(data->addr.sin_port));

        /* disconnect */
        uv_close((uv_handle_t*)(data->partner), free_self);
        uv_close((uv_handle_t*)stream, free_with_data);

        /* remove from map */
        //tcpmap_remove(data_control->all_tcp, (uv_tcp_t*)stream);

        /* free */
        // free(data->partner);
        // free(data);
        // free(stream);
    }
    else if(nread > 0 && nread < buf->len) // read completely
    {
        // TODO 转发消息
    }
    else // read uncompletely
    {
        // TODO 转发消息
    }


    free(buf->base);
}


