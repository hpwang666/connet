
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _PALLOC_H
#define _PALLOC_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>


/*
 * MAX_ALLOC_FROM_POOL should be (pagesize - 1), i.e. 4095 on x86.
 * On Windows NT it decreases a number of locked pages in a kernel.
 */
#define DEFAULT_POOL_SIZE    (4 * 1024) 
#define MAX_ALLOC_FROM_POOL  (DEFAULT_POOL_SIZE - 1)
#define ALIGNMENT   sizeof(unsigned long)
/*在32位平台下，unsigned long 为4字节，64位平台为8字节*/
#define align_ptr(p, a)                                                   \
    (u_char *) (((uintptr_t) (p) + ((uintptr_t) a - 1)) & ~((uintptr_t) a - 1))
	
typedef struct pool_st  		*pool_t;
typedef struct pooList_st 		*pooList_t;

struct pool_st {
    u_char          *last;
    u_char          *end;
    size_t            	failed;
    size_t            	max;
	pool_t     		next;
    pool_t         	current;
	pooList_t    	list;
};

struct pooList_st{
    pool_t  	cache;
	size_t 		cacheSize;
} ;

#ifdef __cplusplus
extern "C"
{
#endif

pooList_t create_pool_list();
void free_pool_list(pooList_t list);
pool_t get_pool(pooList_t list,size_t size);
void destroy_pool(pool_t pool);
void reset_pool(pool_t pool);

void *palloc(pool_t pool, size_t size);
void *pcalloc(pool_t pool, size_t size);

#ifdef __cplusplus
}
#endif
#endif /* _PALLOC_H */
