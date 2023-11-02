
#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <pthread.h>
#include <semaphore.h>

typedef struct threadTask_st 		*threadTask_t;
typedef struct threadPool_st  		*threadPool_t;
typedef struct threadPoolQueue_st 	*threadPoolQueue_t;
typedef struct tpArg_st 			*tpArg_t;
typedef void (*tpHandler)(void *data, tpArg_t tpArg) ; 



struct threadPoolQueue_st{
    threadTask_t       first;
    threadTask_t       last;
	threadTask_t       cache;
} ;

struct tpArg_st { 
	void *arg0;
	void *arg1;
	void *arg2;
	size_t args;
};

struct threadTask_st {
    threadTask_t	next;
    int            	id;
    u_char			ctx[1024];
    tpHandler		handle;
};

struct threadPool_st {
    pthread_mutex_t		mtx;
	pthread_cond_t		cond;
    threadPoolQueue_t   queue;
    int                 waiting;
   
	tpArg_t				tpArg; //整个线程池的其他参数都在其内
    char			  	name[32];
    int					threads;
    int                 max_queue;
};

#define thread_pool_queue_init(q)               \
    (q)->first = NULL;                          \
    (q)->last = NULL;							\
	(q)->cache = NULL

#ifdef __cplusplus
extern "C"
{
#endif
	threadPool_t 	thread_pool_init(const char *name);
	int 			thread_pool_destroy(threadPool_t tp);
	threadTask_t 	thread_task_alloc(threadPool_t tp, tpHandler handle,void *data,size_t );
	int 			thread_task_post(threadPool_t tp, threadTask_t task);
#ifdef __cplusplus
}
#endif

#endif /* _NGX_THREAD_POOL_H_INCLUDED_ */
