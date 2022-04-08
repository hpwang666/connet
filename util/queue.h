/*
 * jabberd - Jabber Open Source Server
 * Copyright (c) 2002 Jeremie Miller, Thomas Muldowney,
 *                    Ryan Eatmon, Robert Norris
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA02111-1307USA
 */
#ifndef _QUEUE_H
#define _QUEUE_H

/*
 * priority queues
 */

typedef struct queueNode_st  *queueNode_t;
struct queueNode_st {
    void  *data;
    queueNode_t  next;
	size_t  priority;
	size_t  dataSize;
};

typedef struct queue_st {
    queueNode_t  cache;

    queueNode_t  head;
    queueNode_t  tail;
	pthread_mutex_t lock;
    size_t     queueSize;
	size_t 	nodeSize;
    size_t		cacheSize;
	
	

} *queue_t;


queue_t queue_new(size_t queueSize,size_t nodeSize); 
void queue_free(queue_t q); 
int queue_push(queue_t q, size_t priority, size_t len,void *data);
void queue_cache(queue_t q,queueNode_t node); 
queueNode_t queue_get(queue_t q);
 



#endif    /* INCL_UTIL_H */


