#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>

/* 内存分配记录表节点 */
typedef struct lc_detail_node_t lc_detail_node_t;
struct lc_detail_node_t
{
	void* ptr;
	size_t bytes;
	const char* file;
	const char* function;
	size_t line;
	lc_detail_node_t* next;
};


/* 内存分配统计表节点 */
typedef struct lc_statistic_node_t lc_statistic_node_t;
struct lc_statistic_node_t
{
	const char* file;   // 所在文件
	size_t line;        // 所在行
	size_t times;       // 分配内存次数
	size_t bytes;       // 总字节数
	lc_statistic_node_t* next;
};

/* 内存分配明细表 */
lc_detail_node_t* lc_detail_list_head = NULL; // 头指针

/* 内存分配统计表 */
lc_statistic_node_t* lc_statistic_list_head = NULL; // 头指针


/* 分配内存，并记录到链表中 */
void* lc_malloc(size_t bytes, const char* file, const char* function, size_t line)
{
	if(0 == bytes)
	{
		return NULL;
	}
	
	/* 创建一个明细节点 */
	lc_detail_node_t* detail_node = malloc(sizeof(lc_detail_node_t));
	if(detail_node == NULL)
	{
		return NULL;
	}
	
	/* 记录内存分配的信息 */
	detail_node->bytes = bytes;
	detail_node->file = file;
	detail_node->function = function;
	detail_node->line = line;
	
	/* 分配内存 */
	detail_node->ptr = malloc(bytes);
	if(detail_node->ptr == NULL)
	{
		free(detail_node);
		return NULL;
	}
	
	/* 在统计表中查找同位置的记录 */
	lc_statistic_node_t* p = lc_statistic_list_head;
	for(; p != NULL; p = p->next)
	{
		if(0 == strcmp(p->file, file) && p->line == line)
		{
			break;
		}
	}
	
	if(p != NULL)
	{
		p->times += 1;
		p->bytes += bytes;
	}
	else
	{
		/* 创建统计表节点 */
		lc_statistic_node_t* statistic_node = (lc_statistic_node_t*)malloc(sizeof(lc_statistic_node_t));
		if(statistic_node == NULL)
		{
			free(detail_node->ptr);
			free(detail_node);
			return NULL;
		}
		
		/* 记录数据 */
		statistic_node->file = file;
		statistic_node->line = line;
		statistic_node->times = 1;
		statistic_node->bytes = bytes;
		
		/* 插入到统计表开头 */
		statistic_node->next = lc_statistic_list_head;
		lc_statistic_list_head = statistic_node;
	}
	
	/* 插入到明细表开头 */
	detail_node->next = lc_detail_list_head;
	lc_detail_list_head = detail_node;	
	
	return detail_node->ptr;
}


/* 释放内存，并清理链表 */
int lc_free(void* ptr)
{
	lc_detail_node_t* prev = NULL;
	lc_detail_node_t* curr = lc_detail_list_head;
	
	/* 从明细表中查找对应的节点 */
	for(; curr != NULL; )
	{
		if(curr->ptr == ptr)
		{
			break;
		}
		prev = curr;
		curr = curr->next;
	}
	
	
	if(curr == NULL)
	{
		return 1;
	}
	
	/* 清除明细表对应的节点,稍后释放内存 */
	if(prev != NULL)
	{
		prev->next = curr->next;
	}
	
	if(curr == lc_detail_list_head)
	{
		lc_detail_list_head = curr->next;
	}
	
	


	
	/* 从统计表中查找对应节点 */
	lc_statistic_node_t* prev2 = NULL;
	lc_statistic_node_t* curr2 = lc_statistic_list_head;
	for(; curr2 != NULL; curr2 = curr2->next)
	{
		if(0 == strcmp(curr2->file, curr->file) && curr2->line == curr->line)
		{
			break;
		}
		prev2 = curr2;
	}
	
	if(curr2 == NULL) // 理论上不可能
	{
		return 2;
	}

	/* 统计表对应节点数据更新 */
	curr2->times -= 1;
	curr2->bytes -= curr->bytes;
	
	/* 清除统计表归零节点 */
	if(curr2->times == 0 && curr2->bytes == 0) // 理论上必然同时归零
	{
		if(prev2 != NULL)
		{
			prev2->next = curr2->next;
		}
		
		if(curr2 == lc_statistic_list_head)
		{
			lc_statistic_list_head = curr2->next;
		}
		
		free(curr2);
	}
	
	
	/* 释放内存 */
	free(curr);
	free(ptr);
	
	return 0;
}


/* 打印明细表 */
void lc_detail(void)
{
	lc_detail_node_t* curr = lc_detail_list_head;
	size_t times = 0;
	size_t bytes = 0;
	/* 遍历打印链表 */
	printf("[Details]\n");
	for(; curr != NULL; curr = curr->next)
	{
		
		#if SIZE_MAX == UNSIGNED_LONG_MAX
			printf("In file <%s> line <%lu> function <%s> : leak %lu bytes\n", curr->file, curr->line, curr->function, curr->bytes);
		#else
			printf("In file <%s> line <%u> function <%s> : leak %u bytes\n", curr->file, curr->line, curr->function, curr->bytes);
		#endif
		times += 1;
		bytes += curr->bytes;
	}
	#if SIZE_MAX == UNSIGNED_LONG_MAX
			printf("Total : %lu times %lu bytes.\n\n", times, bytes);
		#else
			printf("Total : %u times %u bytes.\n\n", times, bytes);
		#endif
	
}


/* 打印统计表 */
void lc_statistic(void)
{
	lc_statistic_node_t* curr = lc_statistic_list_head;
	size_t times = 0;
	size_t bytes = 0;
	/* 遍历打印链表 */
	printf("[Statistics]\n");
	for(; curr != NULL; curr = curr->next)
	{
		
		#if SIZE_MAX == UNSIGNED_LONG_MAX
			printf("In file <%s> line <%lu> : leak %lu times %lu bytes\n", curr->file, curr->line, curr->times, curr->bytes);
		#else
			printf("In file <%s> line <%u> : leak %u times %u bytes\n", curr->file, curr->line, curr->times, curr->bytes);
		#endif
		
		times += curr->times;
		bytes += curr->bytes;
	}
	
	#if SIZE_MAX == UNSIGNED_LONG_MAX
		printf("Total : %lu times %lu bytes.\n\n", times, bytes);
	#else
		printf("Total : %u times %u bytes.\n\n", times, bytes);
	#endif
}


/* signal callback */
void lc_signal_statistic(int sig)
{
	(void)sig;
	lc_statistic();
	signal(sig, lc_signal_statistic);
}

void lc_signal_detail(int sig)
{
	(void)sig;
	lc_detail();
	signal(sig, lc_signal_detail);
}

/* registe signal */
void lc_registe_statistic(int sig)
{
	signal(sig, lc_signal_statistic);
}

void lc_registe_detail(int sig)
{
	signal(sig, lc_signal_statistic);
}