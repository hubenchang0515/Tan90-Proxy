#include "common.h"
#include "trueclient.h"

void true_client_has_connection(uv_stream_t* tcp, int status)
{
    if(status < 0)
    {
        log_printf(LOG_ERROR, "Connection error : %s.", uv_strerror(status));
        return;
    }
    data_control_t* data_control = tcp->data;
    

    /* accept connection */
    uv_tcp_t* connection = malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, connection);
    uv_accept(tcp, (uv_stream_t*) connection);

    /* has no control connection */
    if(data_control->control == NULL)
    {
        log_printf(LOG_ERROR, "Get connection from true client but no control connection.");
        uv_close((uv_handle_t*)connection, free_self);
        return;
    }

    /* bind user data */
    data_proxy_t* data = malloc(sizeof(data_proxy_t));
    if(data == NULL)
    {
        log_printf(LOG_ERROR, "Malloc %s bytes error.", sizeof(data_proxy_t));
        uv_close((uv_handle_t*)connection, free_self);
        return;
    }
    connection->data = data;
    data->data_control = data_control;

    /* get peer ip:port */
    int len = sizeof(data_control->addr);
    uv_tcp_getpeername(connection, (struct sockaddr*)&(data->addr), &len);
    log_printf(LOG_INFO, "Get connection from true client %s:%d.",
                    inet_ntoa(data->addr.sin_addr), htons(data->addr.sin_port));

    /* add to idle map */
    tcpmap_set(data_control->idle_tcp, connection, NULL);

    /* regist read call-back */
    uv_read_start((uv_stream_t*) connection, allocer, true_client_can_read);
}

void true_client_can_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
    data_proxy_t* data_proxy = stream->data;
    data_control_t* data_control = data_proxy->data_control;
    if(nread < 0 && nread != UV_EOF) // error
    {

    }
    else if(nread == UV_EOF) // has no data
    {
        log_printf(LOG_INFO, "Loss connection from true client %s:%d.",
                    inet_ntoa(data_proxy->addr.sin_addr), htons(data_proxy->addr.sin_port));
        
        tcpmap_remove(data_control->all_tcp, data_proxy->partner);
        tcpmap_remove(data_control->idle_tcp, (uv_tcp_t*)stream); // may hadn't been served
        uv_close((uv_handle_t*)stream, free_with_data);
        uv_close((uv_handle_t*)(data_proxy->partner), free_with_data);
    }
    else if(nread > 0 && nread < buf->len) // read completely
    {

    }
    else // read uncompletely
    {
        
    }
}


