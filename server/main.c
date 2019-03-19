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

#include "userdata.h"
#include "proxyclient.h"
#include "trueclient.h"

int main(int argc, char* argv[])
{
    /* Load config file */
    config_t* config = NULL;
    if(argc == 1)
    {
        config = config_load("server.ini");
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
        log_printf(LOG_INFO, "Server exit.");
        return EXIT_FAILURE;
    }

    /* Traverse configs */
    char** groups = config_get_groups(config, NULL);
    for(gsize i = 0; groups[i] != NULL; i++)
    {
        /* get ip:port to ip:port */
        char* true_client_ip = config_get_value(config, groups[i], "true_client_ip");
        int true_client_port = atoi(config_get_value(config, groups[i], "true_client_port"));

        char* proxy_client_ip = config_get_value(config, groups[i], "proxy_client_ip");
        int proxy_client_port = atoi(config_get_value(config, groups[i], "proxy_client_port"));

        log_printf(LOG_INFO, "Config Proxy (%s) %s:%d ----- %s:%d", 
                    groups[i], true_client_ip, true_client_port, proxy_client_ip, proxy_client_port);

        /* bind port to listen true client */
        int uv_err = 0;
        uv_tcp_t true_client;
        uv_tcp_init(loop, &true_client);
        struct sockaddr_in addr;
        uv_ip4_addr(true_client_ip, true_client_port, &addr);
        uv_err = uv_tcp_bind(&true_client, (const struct sockaddr *)&addr, 0);
        if(uv_err < 0)
        {
            log_printf(LOG_ERROR, "Bind %s:%d error : %s.",
                        true_client_ip, true_client_port, uv_strerror(uv_err));
            continue;
        }

        /* bind port to listen proxy client */
        uv_tcp_t proxy_client;
        uv_tcp_init(loop, &proxy_client);
        uv_ip4_addr(proxy_client_ip, proxy_client_port, &addr);
        uv_err = uv_tcp_bind(&proxy_client, (const struct sockaddr *)&addr, 0);
        if(uv_err < 0)
        {
            log_printf(LOG_ERROR, "Bind %s:%d error : %s.",
                        proxy_client_ip, proxy_client_port, uv_strerror(uv_err));
            continue;
        }
        
        /* listen */
        uv_err = uv_listen((uv_stream_t*)&true_client, 100, true_client_has_connection);
        if(uv_err < 0)
        {
            log_printf(LOG_ERROR, "Listen %s:%d error : %s.",
                        true_client_ip, true_client_port, uv_strerror(uv_err));
        }

        uv_err = uv_listen((uv_stream_t*)&proxy_client, 100, proxy_client_has_connection);
        if(uv_err < 0)
        {
            log_printf(LOG_ERROR, "Listen %s:%d error : %s.",
                        proxy_client_ip, proxy_client_port, uv_strerror(uv_err));
        }

        /* bind user data */
        data_control_t userdata;
        userdata.control = NULL;
        userdata.idle_tcp = tcpmap_create_map();
        userdata.all_tcp = tcpmap_create_map();
        true_client.data = &userdata;
        proxy_client.data = &userdata;
    }
    g_strfreev(groups);

    return uv_run(loop, UV_RUN_DEFAULT);
}

