#include "../common/common.h"
#include "trueclient.h"
#include "proxyclient.h"

/*********************************************************
 * Function     : Callback function while get new connection
 *                from true client
 * Parameters   : tcp - uv_stream_t*
 *                status - status of connection
 * Return       : void
*********************************************************/
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
    data->partner = NULL;

    /* get peer ip:port */
    int len = sizeof(data_control->addr);
    uv_tcp_getpeername(connection, (struct sockaddr*)&(data->addr), &len);
    log_printf(LOG_INFO, "Get connection from true client %s:%d.",
                    inet_ntoa(data->addr.sin_addr), htons(data->addr.sin_port));

    /* add to idle map */
    tcpmap_set(data_control->idle_tcp, connection, NULL);

    /* regist read call-back */
    //uv_read_start((uv_stream_t*) connection, allocer, true_client_read);

    /* tell proxy client */
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
    new_buf->base = malloc(sizeof(char));
    if(new_buf->base == NULL)
    {
        free(req);
        free(new_buf);
        log_printf(LOG_ERROR, "Malloc %d bytes error.", sizeof(uv_buf_t));
        return;
    }
    new_buf->base[0] = CMD_NEW_PROXY;
    new_buf->len = 1;
    req->data = new_buf;
    uv_write(req, (uv_stream_t*)data_control->control, new_buf, 1, proxy_client_control_written);
    log_printf(LOG_INFO, "Send command CMD_NEW_PROXY(%u)", (unsigned char)(new_buf->base[0]));
}


/*********************************************************
 * Function     : Callback function while read data from
 *                true client
 * Parameters   : tcp - uv_stream_t*
 *                nread - how many bytes read
 *                buf - data
 * Return       : void
*********************************************************/
void true_client_read(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
    data_proxy_t* data_proxy = stream->data;
    data_control_t* data_control = data_proxy->data_control;
    if(nread < 0 || nread == UV_EOF)
    {
        if(nread == UV_EOF) // disconnected
        {
            log_printf(LOG_INFO, "Loss connection from true client %s:%d.",
                        inet_ntoa(data_proxy->addr.sin_addr), htons(data_proxy->addr.sin_port));
        }
        else // error
        {
            log_printf(LOG_ERROR, "Exceptionally loss connection from true client %s:%d.",
                            inet_ntoa(data_proxy->addr.sin_addr), htons(data_proxy->addr.sin_port));
        }
        
        tcpmap_remove(data_control->all_tcp, data_proxy->partner);
        tcpmap_remove(data_control->idle_tcp, (uv_tcp_t*)stream); // may hadn't been served
        uv_close((uv_handle_t*)stream, free_with_data);
        if(data_proxy->partner != NULL)
        {
            uv_close((uv_handle_t*)(data_proxy->partner), free_with_data);
        }
    }
    else if(data_proxy->partner == NULL) // hasn't been bind to proxy client
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
            log_printf(LOG_ERROR, "Malloc %d bytes error.", sizeof(uv_buf_t));
            return;
        }
        new_buf->base = buf->base;
        new_buf->len = nread;
        req->data = new_buf;
        uv_write(req, (uv_stream_t*)data_proxy->partner, new_buf, 1, true_client_proxy_written);
    }
}


/*********************************************************
 * Function     : Callback function while written data to
 *                proxy client
 * Parameters   : req - uv_write_t*
 *                status - status of writing
 * Return       : void
*********************************************************/
void true_client_proxy_written(uv_write_t* req, int status)
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