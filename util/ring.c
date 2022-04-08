/*
 * 	ring_queue.c
 *  Created on: 2019年11月11日
 *	Author: whp
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "ring.h"
//单链表适用于单生产者，单消费者的模式下，比如net网络模型(单进程  单线程)
//环形队列适用于多生产 单消费情景，而且消费时，不需要加锁

//数据从tail开始写，tail处可以直接写入，head==tail表示 里面没有数据。


ring_t ring_new(size_t size,size_t dataSize)
{
	size_t i =0;
	ring_t q;
	ringNode_t  node0,node1;
	q = (ring_t) calloc(1, sizeof(struct ring_st));
	pthread_mutex_init(&((ring_t)q)->lock, NULL);
	q->size=size;
	q->nodeDataSize = dataSize;
	node0 = q->head=q->tail=(ringNode_t)calloc(1, sizeof(struct ringNode_st));
	node0->data = calloc(1, dataSize);
	node0->dataLen=0;
	while(i<size-1){
		node1 = (ringNode_t)calloc(1, sizeof(struct ringNode_st));
		node1->data = calloc(1, dataSize);
		node1->dataLen=0;
		node0->next = node1;
		node0 = node1;
		i++;
	}
	node0->next =q->head; 
	return q;
}

ringNode_t ring_get(ring_t q)
{
	ringNode_t node =NULL;
	pthread_mutex_lock(&((ring_t)q)->lock); 
	if(q->head == q->tail) //empty
	node= NULL;
	else node= q->head;
	pthread_mutex_unlock(&((ring_t)q)->lock);
	return node;
}

int ring_pull(ring_t q){
	
	pthread_mutex_lock(&((ring_t)q)->lock); //必须加，否则会影响size的统计，以及free
	q->head = q->head->next;
	pthread_mutex_unlock(&((ring_t)q)->lock);
	return 0;
}

int ring_push(ring_t q, void* data,size_t dataLen)
{
	pthread_mutex_lock(&((ring_t)q)->lock);
	if(q->tail->next == q->head) //full
	{
		//node = (ringNode_t)calloc(1, sizeof(struct ringNode_st));
		//node->next = q->head;
		//q->tail->next = node;//fix the ring
		printf("ring queue is full\n");
		pthread_mutex_unlock(&((ring_t)q)->lock);
		return -1;
	}
	if(data && dataLen <= q->nodeDataSize){
		memcpy(q->tail->data,data,dataLen);
		q->tail->dataLen = dataLen;
	}
	
	//q->tail->pkgTime =  osalGetCurrentMsec();
	q->tail = q->tail->next;
	
	pthread_mutex_unlock(&((ring_t)q)->lock);
	return 0;
}

int ring_free(ring_t q)
{
	ringNode_t  node ;
	size_t i=0;
	pthread_mutex_destroy(&((ring_t)q)->lock);
	for(i=0;i < q->size;i++)
	{
		node = q->head->next;
		free(q->head->data);
		free(q->head);
		q->head=node;
	}
	free(q);
	return 0;
}


