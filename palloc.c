
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */
#include "palloc.h"

#undef  _DEBUG
#define _DEBUG
#ifdef _DEBUG
	#define debug(...) printf(__VA_ARGS__)
#else
	#define debug(...)
#endif 

static void *palloc_block(pool_t pool, size_t size);


//从list中获取一个pool
pooList_t create_pool_list()
{
	pooList_t  list;

    list = (pooList_t)calloc(1, sizeof(struct pooList_st));
    if (list == NULL) {
        return NULL;
    }
	list->cache = NULL;
	list->cacheSize = 0;
    return list;
	
}

pool_t get_pool(pooList_t list,size_t size)
{
    pool_t  p;
	if(NULL == list) return NULL;
    if(list->cache){
		debug("get pool in cache\n");
		p = list->cache;
		list->cache = p->next;
	}
	else{
		debug("get pool in calloc\n");
		p =(pool_t) calloc(1,size);
		if (p == NULL) {
			return NULL;
		}
	}
	
    p->last = (u_char *) p + sizeof(struct pool_st);
    p->end = (u_char *) p + size;
   
    p->failed = 0;

    size = size - sizeof(struct pool_st);
    p->max = (size < MAX_ALLOC_FROM_POOL) ? size : MAX_ALLOC_FROM_POOL;
	p->next = NULL;
    p->current = p;
	p->list = list;
	memset(p->last,0,size);
    return p;
}


void destroy_pool(pool_t pool)
{
    pool_t     n,p =pool;
	pooList_t l=pool->list;
	while (p) {
		debug("destroy one pool\n");
		n = p->next;
        p->next = l->cache;
		l->cache=p;
		p=n;
    }
}

void free_pool_list(pooList_t list)
{
	pool_t     n,p =list->cache;
	while (p) {
		printf("free pool\n");
		n = p->next;
		free(p);
		p=n;
    }
	free(list);
}

void reset_pool(pool_t pool)
{
    pool_t        p;

    for (p = pool; p; p = p->next) {
        p->last = (u_char *) p + sizeof(struct pool_st);
        p->failed = 0;
		memset(p->last,0,(p->end-p->last));
    }
    pool->current = pool;
}


void *palloc(pool_t pool, size_t size)
{
	u_char      *m;
    pool_t  p;
    if (size <= pool->max) {
		
		p = pool->current;
		do {
			m = p->last;
			m = align_ptr(m, ALIGNMENT);
			
			if ((size_t) (p->end - m) >= size) {
				p->last = m + size;
				pool->current = p;
				return m;
			}
			p = p->next;
			debug("try next pool\n");
		} while (p);
		debug("try get new pool\n");
		return palloc_block(pool, size);
    }
	else printf("too large alloc size :%d\n",size);
	return NULL;
}


static void *palloc_block(pool_t pool, size_t size)
{
    u_char      *m;
    size_t      psize;
    pool_t 		p,newPool;
	pooList_t 	l=pool->list;
	
    psize = (size_t) (pool->end - (u_char *) pool);

	if(l->cache){
		debug("palloc_block in  cache\n");
		newPool = l->cache;
		l->cache = newPool->next;
		memset(newPool+sizeof(struct pool_st),0,psize-sizeof(struct pool_st));
	}
	else{
		debug("palloc_block in calloc\n");
		newPool =(pool_t) calloc(1,psize);
		if (newPool == NULL) {
			return NULL;
		}
	}

    m = (u_char *)newPool;

    newPool->end =(u_char*) newPool + psize;
    newPool->next = NULL;
    newPool->failed = 0;

    m += sizeof(struct pool_st);
    m = align_ptr(m, ALIGNMENT);
    newPool->last = m + size;

    /*这个逻辑主要是为了防止pool上的子节点过多，导致每次palloc循环pool->d.next链表
       * 将pool->current设置成最新的子节点之后，每次最大循环4次，不会去遍历整个缓存池链表
      */
    for (p = pool->current; p->next; p = p->next) {
        if (p->failed++ > 4) {
            pool->current = p->next;
        }
    }

    p->next = newPool;

    return m;
}

void *pcalloc(pool_t pool, size_t size)
{
    void *p;

    p = palloc(pool, size);
    if (p) {
        memset(p,0, size);
    }
    return p;
}
