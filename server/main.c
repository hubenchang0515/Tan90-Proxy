#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

#include "common.h"

#include "proxyclient.h"
#include "trueclient.h"

int main(int argc, char* argv[])
{
    /* Load config file */
    config_t* config = config_load("server.config");
    if(config == NULL)
    {
        log_printf(LOG_INFO, "Server exit.");
        return EXIT_FAILURE;
    }

    /* Traverse configs */
    char** groups = config_get_groups(config, NULL);
    for(gsize i = 0; groups[i] != NULL; i++)
    {
        /* get ip:port to ip:port */
        char* from_ip = config_get_value(config, groups[i], "from_ip");
        int from_port = atoi(config_get_value(config, groups[i], "from_port"));

        char* to_ip = config_get_value(config, groups[i], "to_ip");
        int to_port = atoi(config_get_value(config, groups[i], "to_port"));

        log_printf(LOG_INFO, "Proxy(%s) %s:%d ----- %s:%d", 
                    groups[i], from_ip, from_port, to_ip, to_port);

        /* bind */
        int uv_err = 0;
        uv_tcp_t* true_client = malloc(sizeof(uv_tcp_t));
        uv_tcp_init(loop, true_client);
        struct sockaddr_in addr;
        uv_ip4_addr(from_ip, from_port, &addr);
        uv_err = uv_tcp_bind(true_client, (const struct sockaddr *)&addr, 0);
        if(uv_err < 0)
        {
            log_printf(LOG_ERROR, "Bind %s:%d error : %s.",
                        from_ip, from_port, uv_strerror(uv_err));
            continue;
        }

        uv_tcp_t* proxy_client = malloc(sizeof(uv_tcp_t));
        uv_tcp_init(loop, proxy_client);
        uv_ip4_addr(to_ip, to_port, &addr);
        uv_err = uv_tcp_bind(proxy_client, (const struct sockaddr *)&addr, 0);
        if(uv_err < 0)
        {
            log_printf(LOG_ERROR, "Bind %s:%d error : %s.",
                        to_ip, to_port, uv_strerror(uv_err));
            continue;
        }
        
        /* listen */
        uv_err = uv_listen((uv_stream_t*)true_client, 100, true_client_has_connection);
        if(uv_err < 0)
        {
            log_printf(LOG_ERROR, "Listen %s:%d error : %s.",
                        from_ip, from_port, uv_strerror(uv_err));
        }

        uv_err = uv_listen((uv_stream_t*)proxy_client, 100, proxy_client_has_connection);
        if(uv_err < 0)
        {
            log_printf(LOG_ERROR, "Listen %s:%d error : %s.",
                        to_ip, to_port, uv_strerror(uv_err));
        }

        /* bind user data */
        data_control_t* userdata = malloc(sizeof(data_control_t));
        if(userdata == NULL)
        {
            log_printf(LOG_ERROR, "Malloc %s bytes error.", sizeof(data_control_t));
            continue;
        }
        userdata->control = NULL;
        userdata->idle_tcp = tcpmap_create_map();
        userdata->all_tcp = tcpmap_create_map();
        true_client->data = userdata;
        proxy_client->data = userdata;
    }
    g_strfreev(groups);

    return uv_run(loop, UV_RUN_DEFAULT);
}

