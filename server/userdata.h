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

#ifndef TAN90_PROXY_SERVER_USERDATA_H
#define TAN90_PROXY_SERVER_USERDATA_H

#include "../common/common.h"

typedef struct TcpControlConnectionUserData data_control_t;
typedef struct TcpProxyConnectionUserData data_proxy_t;

struct TcpControlConnectionUserData
{
    uv_tcp_t* control;              // control connection
    map_t* idle_tcp;                // idle tcp from true client waiting for serve
    map_t* all_tcp;                 // all tcp from proxy client
    struct sockaddr_in addr;        // address of control connection
};


struct TcpProxyConnectionUserData
{
    data_control_t* data_control;
    uv_tcp_t* partner;
    struct sockaddr_in addr;
    int shutdown;
};

#endif