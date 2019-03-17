#include "common.h"
#include "trueclient.h"

void true_client_has_connection(uv_stream_t* tcp, int status)
{
    if(status < 0)
    {
        log_printf(LOG_ERROR, "Connection error : %s\n", uv_strerror(status));
        return;
    }
    data_control_t* data_control = tcp->data;

    /* accept connection */
    uv_tcp_t* connection = malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, connection);
    uv_accept(tcp, (uv_stream_t*) connection);

    /* add to idle queue */
    tcpmap_push_idle(data_control->idle_queue, connection);

    /* regist read call-back */
    uv_read_start((uv_stream_t*) connection, allocer, true_client_can_read);
}

void true_client_can_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
    if(nread < 0 && nread != UV_EOF) // error
    {

    }
    else if(nread == UV_EOF) // has no data
    {

    }
    else if(nread > 0 && nread < buf->len) // read completely
    {

    }
    else // read uncompletely
    {
        
    }
}


