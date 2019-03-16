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

#ifndef TAN90_PROXY_TCPMAP_H
#define TAN90_PROXY_TCPMAP_H

#include <glib.h>
#include <uv.h>

typedef GQueue queue_t;
typedef GHashTable map_t;

/* store idle tcp connection */
queue_t* tcpmap_create_queue(void);
void tcpmap_free_queue(queue_t* queue);
uv_tcp_t* tcpmap_pop_idle(queue_t* queue);
void tcpmap_push_idle(queue_t* queue, uv_tcp_t* tcp);

/* store tcp mapping */
map_t* tcpmap_create_map(void);
void tcpmap_free_map(map_t* map);
uv_tcp_t* tcpmap_get(map_t* map, uv_tcp_t* tcp);
int tcpmap_set(map_t* map, uv_tcp_t* tcp1, uv_tcp_t* tcp2);
int tcpmap_remove(map_t* map, uv_tcp_t* tcp);
uv_tcp_t** tcpmap_get_all_keys(map_t* map, guint* len);
void tcpmap_clear(map_t* map);


#endif