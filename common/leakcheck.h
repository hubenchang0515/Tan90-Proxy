#ifndef LEAKCHECK_H
#define LEAKCHECK_H

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

typedef struct lc_detail_node_t lc_detail_node_t;
typedef struct lc_statistic_node_t lc_statistic_node_t;

extern lc_detail_node_t* lc_detail_list_head;

void* lc_malloc(size_t bytes, const char* file, const char* function, size_t line);
int lc_free(void* ptr);
void lc_detail(void);
void lc_statistic(void);

void* lc_signal_statistic(int sig);
void* lc_signal_detail(int sig);
void lc_register_statistic(int sig);
void lc_register_detail(int sig);

#ifdef malloc
	#undef malloc
#endif
#define malloc(bytes) lc_malloc(bytes, __FILE__, __FUNCTION__, __LINE__)

#ifdef free
	#undef free
#endif
#define free(ptr) lc_free(ptr)

#endif