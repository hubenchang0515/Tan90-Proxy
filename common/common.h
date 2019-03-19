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

#ifndef TAN90_PROXY_COMMON_H
#define TAN90_PROXY_COMMON_H

#include <uv.h>
#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "tcpmap.h"
#include "log.h"
#include "config.h"

#ifndef loop
#define loop uv_default_loop()
#endif

/* Command for control connection, only support 1byte(0~255) */
typedef enum ControlConnectionCommand cmd_t;
enum ControlConnectionCommand
{
    CMD_NEW_PROXY = 0,
};

void allocer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
void free_self(uv_handle_t *handle);
void free_with_data(uv_handle_t *handle);
const char* filename(const char* path, char* output, size_t size);

#endif