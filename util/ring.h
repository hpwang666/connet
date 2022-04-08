/*
 * 	ring_queue.c
 *  Created on: 2019年11月11日
 *	Author: whp
 */

#ifndef _RING_H
#define _RING_H
#include <pthread.h>
//单链表适用于但生产者，单消费者的模式下，比如net网络模型(单进程  单线程)
//环形队列适用于多生产单消费情景，而且消费时，不需要加锁

//数据从tail开始写，tail处可以直接写入，head==tail表示 里面没有数据。
typedef struct ring_st *ring_t;
typedef struct ringNode_st  *ringNode_t;

struct ring_st{
	pthread_mutex_t lock;
		
    ringNode_t  head;
    ringNode_t  tail;

    size_t      size;//总大小
	size_t  	nodeDataSize;
};

struct ringNode_st {
	size_t   dataLen;
    void  *data;
	unsigned long long pkgTime;
    ringNode_t  next;
};



ring_t     		ring_new(size_t size,size_t dataSize);
int       		ring_free(ring_t q);
int 			ring_push(ring_t q, void * ,size_t);
ringNode_t    	ring_get(ring_t q);    //get  得到数据，但没有改变head的位置，还是占据着
int    			ring_pull(ring_t q);   //pull 更新head的位置

#endif


