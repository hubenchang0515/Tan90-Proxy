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

#include <stdio.h>
#include <stdlib.h>
#include <uv.h>

#include "../common/common.h"
#include "../common/config.h"
#include "../common/log.h"
#include "../common/tcpmap.h"

#include "proxyserver.h"

int main(int argc, char* argv[])
{
    /* Load config file */
    config_t* config = NULL;
    if(argc == 1)
    {
        config = config_load("client.ini");
    }
    else if(argc == 2)
    {
        config = config_load(argv[1]);
    }
    else
    {
        char buff[1024];
        filename(argv[0], buff, 1024);
        log_printf(LOG_ERROR, "Error Console Parameters.");
        log_printf(LOG_INFO, "Usage : %s [config-file] ", buff);
        log_printf(LOG_INFO, "        %s ", buff);
        log_printf(LOG_INFO, "        %s myconfig.ini ", buff);
        return EXIT_FAILURE;
    }

    /* check */
    if(config == NULL)
    {
        log_printf(LOG_INFO, "Client exit.");
        return EXIT_FAILURE;
    }


    /* Traverse configs */
    char** groups = config_get_groups(config, NULL);
    for(gsize i = 0; groups[i] != NULL; i++)
    {
        /* get ip:port to ip:port */
        char* proxy_server_ip = config_get_value(config, groups[i], "proxy_server_ip");
        int proxy_server_port = atoi(config_get_value(config, groups[i], "proxy_server_port"));

        char* true_server_ip = config_get_value(config, groups[i], "true_server_ip");
        int true_server_port = atoi(config_get_value(config, groups[i], "true_server_port"));

        log_printf(LOG_INFO, "Config Proxy (%s) %s:%d ----- %s:%d", 
                    groups[i], proxy_server_ip, proxy_server_port, 
                    true_server_ip, true_server_port);

        /* create control connection */
        uv_connect_t* connect_req = malloc(sizeof(uv_connect_t));
        if(connect_req == NULL)
        {
            log_printf(LOG_ERROR, "Malloc %d bytes error.", sizeof(uv_connect_t));
            break;
        }
        uv_tcp_t* control_connection = malloc(sizeof(uv_tcp_t));
        if(control_connection == NULL)
        {
            free(connect_req);
            log_printf(LOG_ERROR, "Malloc %d bytes error.", sizeof(uv_tcp_t));
            break;
        }
        uv_tcp_init(loop, control_connection);
        
        /* bind user data */
        data_control_t* userdata = malloc(sizeof(data_control_t));
        if(userdata == NULL)
        {
            free(control_connection);
            free(connect_req);
            log_printf(LOG_ERROR, "Malloc %d bytes error.", sizeof(uv_tcp_t));
            break;
        }
        userdata->req = connect_req;
        userdata->all_tcp = tcpmap_create_map();
        userdata->idle_tcp = tcpmap_create_map();
        userdata->control = NULL;
        connect_req->data = userdata;
        control_connection->data = userdata;

        /* connect to proxy server */
        uv_ip4_addr(proxy_server_ip, proxy_server_port, &(userdata->proxy_server_addr));
        uv_tcp_connect(connect_req, control_connection, 
                        (const struct sockaddr*)&(userdata->proxy_server_addr), 
                        proxy_server_control_connected);

        /* save true server ip:port */
        uv_ip4_addr(true_server_ip, true_server_port, &(userdata->true_server_addr));
    }
    g_strfreev(groups);

    #ifdef DEBUG
        lc_registe_statistic(SIGINT);
    #endif

    return uv_run(loop, UV_RUN_DEFAULT);
}