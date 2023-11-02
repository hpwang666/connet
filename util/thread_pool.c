#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/syscall.h> 

#include "thread_pool.h"

 

static void thread_pool_exit_handler(void *data, tpArg_t);
static void *thread_pool_cycle(void *data);
int               threadPool_task_id;


threadPool_t thread_pool_init(const char *name)
{
    int             err;
    pthread_t       tid;
    int      n;
    pthread_attr_t  attr;
	threadPool_t tp;
	
	tp = (threadPool_t)calloc(1,sizeof(struct threadPool_st));
	tp->queue = (threadPoolQueue_t)calloc(1,sizeof(struct threadPoolQueue_st));
	tp->tpArg = (tpArg_t)calloc(1,sizeof(struct tpArg_st));
    thread_pool_queue_init(tp->queue);

	pthread_mutex_init(&tp->mtx, NULL);
	pthread_cond_init(&tp->cond, NULL);
   
	tp->threads = 8;
	tp->max_queue = 64;
	sprintf(tp->name,"%s",name);
	
	
    err = pthread_attr_init(&attr);
    if (err) {
        printf("pthread_attr_init() failed\r\n");
        return NULL;
    }
	
	//设立线程的独立性，无法使用join
    err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (err) {
        printf("pthread_attr_setdetachstate() failed\r\n");
        exit(-1);
    }

#if 0
    err = pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN);
    if (err) {
        ngx_printfor(NGX_LOG_ALERT, log, err,
                      "pthread_attr_setstacksize() failed");
        return NULL;
    }
#endif

    for (n = 0; n < tp->threads; n++) {
        err = pthread_create(&tid, &attr, thread_pool_cycle, tp);
        if (err) {
            printf("pthread_create() failed\r\n");
            exit(-1);
        }
    }
    (void) pthread_attr_destroy(&attr);

    return tp;
}

static volatile char  exit_lock;
int thread_pool_destroy(threadPool_t tp)
{
    int           n;
    threadTask_t    task;
   

    for (n = 0; n < tp->threads; n++) {
        exit_lock = 1;
		task = thread_task_alloc(tp,thread_pool_exit_handler,NULL,0);
        if (thread_task_post(tp, task) != 0) {
            return -1;
        }
		
        while (exit_lock ) {
            usleep(1);
        }
    }
    (void) pthread_cond_destroy(&tp->cond);
    (void) pthread_mutex_destroy(&tp->mtx);
	while(tp->queue->first !=NULL){
		task = tp->queue->first->next;
		free(tp->queue->first);
		tp->queue->first=task;
	}
	while(tp->queue->cache !=NULL){
		task = tp->queue->cache->next;
		free(tp->queue->cache);
		tp->queue->cache=task;
	}

	free(tp->tpArg);
	free(tp->queue);
	free(tp);
	
	return 0;
}


 static void thread_pool_exit_handler(void *data, tpArg_t tpArg)
{
    exit_lock = 0;
	printf("thread exit\n");
    pthread_exit(0);
}

threadTask_t thread_task_alloc(threadPool_t tp, tpHandler handle,void *data,size_t len)
{
    threadTask_t  task;
	pthread_mutex_lock(&tp->mtx);
	if(len > 1024) {
		 printf("thread_task_alloc() failed :data is overflowed (1024) \r\n");
		 pthread_mutex_unlock(&tp->mtx);
		 printf("null 0\n");
		 return NULL;		
	}
	
	if(tp->queue->cache == NULL){
		//printf("calloc \r\n");
		task =(threadTask_t) calloc(1, sizeof(struct threadTask_st));
		if (task == NULL) {
			pthread_mutex_unlock(&tp->mtx);
			printf("null 1\n");
			return NULL;
		}
	}
	else{
		//printf("cache \r\n");
		task = tp->queue->cache;
		tp->queue->cache = task->next;
	}
    task->handle = handle;
	
	if(data)
		memcpy(task->ctx,(u_char *)data,len);
	task->next = NULL;
	pthread_mutex_unlock(&tp->mtx);
    return task;
}


int thread_task_post(threadPool_t tp, threadTask_t task)
{
    pthread_mutex_lock(&tp->mtx);
       
    if (tp->waiting >= tp->max_queue) {
		
        task->next = tp->queue->cache;
		tp->queue->cache = task;
		printf("thread pool \"%s\" queue overflow: %d tasks waiting\r\n",
                      tp->name, tp->waiting);	  
		pthread_mutex_unlock(&tp->mtx);	  
        return -1;
    }
    task->id = threadPool_task_id++;
    task->next = NULL;

    if (pthread_cond_signal(&tp->cond) != 0) {
        (void) pthread_mutex_unlock(&tp->mtx);
        return -1;
    }

   //*tp->queue.last = task;
   // tp->queue.last = &task->next;
	
	if(tp->queue->last == NULL){
		tp->queue->last = tp->queue->first = task;
	}
	else{
		tp->queue->last->next = task;
		tp->queue->last = task;
	}
    tp->waiting++;
    pthread_mutex_unlock(&tp->mtx);
	
    return 0;
}


 static void *thread_pool_cycle(void *data)
{
    threadPool_t tp =(threadPool_t) data;

    int             err;
    sigset_t        set;
    struct threadTask_st  	*task;

    printf("thread in pool \"%s\" started\r\n", tp->name);

    sigfillset(&set);
    sigdelset(&set, SIGILL);
    sigdelset(&set, SIGFPE);
    sigdelset(&set, SIGSEGV);
    sigdelset(&set, SIGBUS);

    err = pthread_sigmask(SIG_BLOCK, &set, NULL);
    if (err) {
        printf("pthread_sigmask() failed\r\n");
        return NULL;
    }

    for ( ;; ) {
        if (pthread_mutex_lock(&tp->mtx) != 0) {
            return NULL;
        }

        /* the number may become negative */
        tp->waiting--;
        while (tp->queue->first == NULL) {
            if (pthread_cond_wait(&tp->cond, &tp->mtx)!= 0)
            {
                pthread_mutex_unlock(&tp->mtx);
                return NULL;
            }
        }

        task = tp->queue->first;
        tp->queue->first = task->next;

        if (tp->queue->first == NULL) {
            tp->queue->last = tp->queue->first;
        }

        if (pthread_mutex_unlock(&tp->mtx) != 0) {
            return NULL;
        }

		//printf("task: %d run in thread pool \"%s\" \r\n",task->id, tp->name);
		//printf("tid: %ld \r\n",syscall(SYS_gettid));
        task->handle(task->ctx, tp->tpArg);
        
		pthread_mutex_lock(&tp->mtx);
        task->next = tp->queue->cache;
		tp->queue->cache = task;
		pthread_mutex_unlock(&tp->mtx);
    }
}
