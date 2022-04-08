#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

/* priority jqueues */

#include "queue.h"

queue_t queue_new(size_t queueSize,size_t nodeSize) 
{
    queue_t q;
    queueNode_t n0=NULL,n1=NULL;
    q = (queue_t) calloc(1,sizeof(struct queue_st));
    q->head=NULL;
	q->tail=NULL;
	q->cache = NULL;
	q->queueSize = q->cacheSize = queueSize;
	q->nodeSize =  nodeSize;
        
    n1 = (queueNode_t) calloc(1,sizeof(struct queueNode_st));    
	n1->next=NULL;
	while(queueSize-1){
		n0 = (queueNode_t) calloc(1,sizeof(struct queueNode_st));
		n0->data = calloc(1,q->nodeSize);
		n0->priority=0;
		n0->dataSize=0;
		
		n0->next=n1;
		n1=n0;
		queueSize--;
	}
    q->cache = n0;
    pthread_mutex_init(&(q->lock), NULL);
    return q;
}

void queue_free(queue_t q) 
{
    queueNode_t n0=NULL,n1=NULL;
	n0 = q->head;
	while(n0){
		n1=n0->next;
		free(n0->data);
		free(n0);
		n0=n1;
	}
	n0 =q->cache;
	while(n0){
		n1=n0->next;
		free(n0->data);
		free(n0);
		n0=n1;
	}
	pthread_mutex_destroy(&(q->lock));
	free(q);
}

int queue_push(queue_t q, size_t priority, size_t len,void *data) 
{
    queueNode_t n=NULL,scan=NULL,n1=NULL;
        
        
    if(len > q->nodeSize ) return -1;
	
    pthread_mutex_lock(&((queue_t)q)->lock);
    /* node from the cache */
	  n = q->cache;
    if(n != NULL){
        q->cache = n->next;
        --(q->cacheSize);
    }
    else {
        pthread_mutex_unlock(&((queue_t)q)->lock);
        return -1;
	}
    memcpy(n->data,data,len);
    n->priority = priority;
    n->dataSize=len;
    n->next = NULL;

    /* first one */
    if(q->tail == NULL && q->head == NULL) {
        q->tail = n;
        q->head = n;
        pthread_mutex_unlock(&((queue_t)q)->lock);
        return 0;
    }
    /* find the first node with priority >= to us */
        
        
        /* find the first node with priority >= to us */
    for(scan = q->head; scan != NULL && scan->priority >= priority; ){
		n1 = scan;
		scan = scan->next;
	}

    /* didn't find one, so we have lowest priority - push us on the tail */
    if(scan == NULL) {
		q->tail->next=n;
        q->tail = n;
    }
    else{
		if(n1)
			n1->next = n;
		if(scan==q->head)  q->head = n;
		n->next = scan;
	}
    pthread_mutex_unlock(&((queue_t)q)->lock);
    return 0;
}


void queue_cache(queue_t q,queueNode_t node) 
{
    /* node to cache for later reuse */
	pthread_mutex_lock(&((queue_t)q)->lock);
    node->next = q->cache; //when run first time q->cache =NULL so here is safe
	q->cache = node;
	++q->cacheSize;
	pthread_mutex_unlock(&((queue_t)q)->lock);
}

queueNode_t queue_get(queue_t q)
{
    queueNode_t n;

    pthread_mutex_lock(&((queue_t)q)->lock);
    if(q->head == NULL){
		pthread_mutex_unlock(&((queue_t)q)->lock);
		return NULL;
	}
        
	n = q->head;
    q->head = n->next;
    if(q->head == NULL){    
		q->tail = NULL;
    }
    pthread_mutex_unlock(&((queue_t)q)->lock);
    return n;
}