#ifndef _EVENT_H
#define _EVENT_H

#include "core.h"

#define READ_EVENT  	(EPOLLIN|EPOLLRDHUP)
#define WRITE_EVENT 	EPOLLOUT
#define CLOSE_EVENT     (0x10)

#define  handle_read_event(e) (add_event(e,READ_EVENT))
#define  handle_write_event(e) (add_event(e,WRITE_EVENT))


struct event_st {
    void            *data;//conn
	
	int              		available;//从内核态没有读完的数据
    event_handler  			handler;
    struct rbtreeNode_st   	timer;
	
	
    /*
     * the event was passed or would be passed to a kernel;
     * in aio mode - operation was posted.
     */
    unsigned         active:1;

    /* the ready event; in aio mode 0 means that no operation can be posted */
    unsigned         ready:1;

    unsigned         error:1;

    unsigned         timedout:1;
    unsigned         timer_set:1;

};

#ifdef __cplusplus
extern "C"
{
#endif

int add_event(event_t ev, int event);
int del_event(event_t ev,int event ,int flags);
int process_events(int timer, int flags);
int init_epoll();
int free_epoll();

#ifdef __cplusplus
}
#endif

#if 0
	EPOLLERR 0x8
	EPOLLET -0x80000000
	EPOLLHUP 0x10
	EPOLLIN 0x1
	EPOLLMSG 0x400
	EPOLLONESHOT 0x40000000
	EPOLLOUT 0x4
	EPOLLPRI 0x2
	EPOLLRDBAND 0x80
	EPOLLRDHUP 0x2000
	EPOLLRDNORM 0x40
	EPOLLWRBAND 0x200
	EPOLLWRNORM 0x100
	EPOLL_CLOEXEC 0x80000
#endif
#endif
