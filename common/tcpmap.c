#include "tcpmap.h"
#include "log.h"
#include <stdlib.h>

/*********************************************************
 * Function     : Create a queue to store idle tcp 
 * Parameters   : void
 * Return       : queue_t*
*********************************************************/
queue_t* tcpmap_create_queue(void)
{
    return g_queue_new();
}


/*********************************************************
 * Function     : Free queue
 * Parameters   : queue - queue_t*
 * Return       : void
*********************************************************/
void tcpmap_free_queue(queue_t* queue)
{
    g_queue_free(queue);
}


/*********************************************************
 * Function     : Pop the tcp at head
 * Parameters   : queue - queue_t*
 * Return       : uv_tcp_t*
*********************************************************/
uv_tcp_t* tcpmap_pop_idle(queue_t* queue)
{
    return g_queue_pop_head(queue);
}


/*********************************************************
 * Function     : Push the tcp to the tail
 * Parameters   : queue - queue_t*
 *                tcp - uv_tcp_t*
 * Return       : void
*********************************************************/
void tcpmap_push_idle(queue_t* queue, uv_tcp_t* tcp)
{
    g_queue_push_tail(queue, tcp);
}


/*********************************************************
 * Function     : Create a hash table
 * Parameters   : void
 * Return       : map_t*
*********************************************************/
map_t* tcpmap_create_map(void)
{
    return g_hash_table_new(g_direct_hash, g_direct_equal);
}


/*********************************************************
 * Function     : Free a hash table
 * Parameters   : map - map_t*
 * Return       : void
*********************************************************/
void tcpmap_free_map(map_t* map)
{
    g_hash_table_destroy(map);
}


/*********************************************************
 * Function     : Get value from map
 * Parameters   : map - map_t*
 *                tcp - tcp_t* 
 * Return       : tcp_t*
*********************************************************/
uv_tcp_t* tcpmap_get(map_t* map, uv_tcp_t* tcp)
{
    return g_hash_table_lookup(map, tcp);
}


/*********************************************************
 * Function     : Get the first key key map
 * Parameters   : map - map_t*
 * Return       : tcp_t*
*********************************************************/
uv_tcp_t* tcpmap_get_first_key(map_t* map)
{
    guint length = 0;
    uv_tcp_t** keys = tcpmap_get_all_keys(map, &length);
    uv_tcp_t* ret = keys[0];
    g_free(keys);
    return ret;
}


/*********************************************************
 * Function     : Set key-value to map
 * Parameters   : map - map_t*
 *                tcp1 - tcp_t* 
 *                tcp2 - tcp_t* 
 * Return       : int
*********************************************************/
int tcpmap_set(map_t* map, uv_tcp_t* tcp1, uv_tcp_t* tcp2)
{
    if(g_hash_table_contains(map, tcp1) || g_hash_table_contains(map, tcp2))
    {
        log_printf(LOG_ERROR, "Map(%p) has the key tcp(0x%p or 0x%p) already.",
                    map, tcp1, tcp2);

        return EXIT_FAILURE;
    }

    g_hash_table_insert(map, tcp1, tcp2);
    //g_hash_table_insert(map, tcp2, tcp1);
}

/*********************************************************
 * Function     : Set key-value to map
 * Parameters   : map - map_t*
 *                tcp - tcp_t* 
 * Return       : int
*********************************************************/
int tcpmap_remove(map_t* map, uv_tcp_t* tcp)
{
    return g_hash_table_remove(map, tcp);
}


/*********************************************************
 * Function     : Get all keys of map
 * Parameters   : map - map_t*
 *                len - return number if keys
 * Return       : uv_tcp_t* []
*********************************************************/
uv_tcp_t** tcpmap_get_all_keys(map_t* map, guint* len)
{
    return (uv_tcp_t**)g_hash_table_get_keys_as_array(map, len);
}


/*********************************************************
 * Function     : Clear the map
 * Parameters   : map - map_t*
 * Return       : void
*********************************************************/
void tcpmap_clear(map_t* map)
{
    g_hash_table_remove_all(map);
}