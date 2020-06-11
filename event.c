#include "event.h"
#include "timer.h"
#include "connection.h"

static int epfd = -1;
static struct epoll_event  event_list[64];
static int           nevents = 64;

int init_epoll()
{
	epfd=epoll_create(256);
	return 0;
}

int free_epoll()
{
	close(epfd);
	return 0;
}
int add_event(event_t ev, int event)
{
    int          	op;
    int             events,prev;
    event_t         e;
    conn_t    		c;
    struct epoll_event   ee;

    c =(conn_t) ev->data; //在get_connection() ,获取c时，就将rcv->data = c

    events = event;

    if (event == READ_EVENT) {
        e = c->write;
        events = EPOLLIN|EPOLLRDHUP|EPOLLHUP;
		prev = 0;
    } else {
        e = c->read;
        events = EPOLLOUT;
		prev =  EPOLLIN|EPOLLRDHUP|EPOLLHUP;
    }
	
	if (!e->active) prev = 0;
    if (e->active||ev->active) {//已经激活了就不要添加了,所以还要判断write的active
        op = EPOLL_CTL_MOD;
    } else {
        op = EPOLL_CTL_ADD;
    }

    ee.events = events | prev |EPOLLET; //全部边缘触发模式
    ee.data.ptr = (void *) ((uintptr_t) c | c->instance);

    if (epoll_ctl(epfd, op, c->fd, &ee) == -1) {
        return -1;
    }
    ev->active = 1;
    return 0;
}


int del_event(event_t ev, int event, int flags)
{
    int                  op;
    int             prev;
    event_t         e;
    conn_t     		c;
    struct epoll_event   ee;

    /*
     * when the file descriptor is closed, the epoll automatically deletes
     * it from its queue, so we do not need to delete explicitly the event
     * before the closing the file descriptor
     */

    if (flags & CLOSE_EVENT) {
        ev->active = 0;
        return 0;
    }

    c = ev->data;

    if (event == READ_EVENT) {
        e = c->write;
        prev = EPOLLOUT;

    } else {
        e = c->read;
        prev = EPOLLIN|EPOLLRDHUP|EPOLLHUP;
    }

    if (e->active) {
        op = EPOLL_CTL_MOD;
        ee.events = prev ;
        ee.data.ptr = (void *) ((uintptr_t) c | c->instance);

    } else {
        op = EPOLL_CTL_DEL;
        ee.events = 0;
        ee.data.ptr = NULL;
    }

    if (epoll_ctl(epfd, op, c->fd, &ee) == -1) {
        return -1;
    }
    ev->active = 0;
    return 0;
}


int process_events(int timer, int flags)
{
    int        events;
    int        revents;
	uintptr_t	instance;
    int         i;
    int          		err;
    event_t       rev, wev;

    conn_t  		c;

	//timer是最新的超时事件timer，为0表示有超时事件，直接返回
    events = epoll_wait(epfd, event_list, (int) nevents, timer);
	// if (flags & UPDATE_TIME)
		time_update();
	err = (events == -1) ? errno : 0;
    if (err) {
        if (err == EINTR) {
			//这里没有设置定时器，所以不会触发。
           return 0;
          
        } else {
            return -1;
        }
    }
	
    for (i = 0; i < events; i++) {
		//printf("%d  %x\n",events,event_list[i].events);
	
        c = event_list[i].data.ptr;

        instance = (uintptr_t) c & 1;
        c = (conn_t) ((uintptr_t) c & (uintptr_t) ~1);
        revents = event_list[i].events;
		if (c->fd == -1 || c->instance != instance) {
				/*
				 * the stale event from a file descriptor
				 * that was just closed in this iteration
				 */
				continue;
		}
		
        if (revents & (EPOLLERR|EPOLLHUP|EPOLLRDHUP)) {
			/*当连接意外断开后，会再keepalive设定时间内返回两次
				EPOLLRDHUP + EPOLLHUP+EPOLLERR+EPOLLIN
				EPOLLRDHUP + EPOLLHUP+EPOLLIN*/
            /*
             * if the error events were returned, add EPOLLIN and EPOLLOUT
             * to handle the events at least in one active handler
             */
			printf("err & hup\n");
            revents |= EPOLLIN|EPOLLOUT;
        }
		
		rev = c->read;
        if ((revents & EPOLLIN) && rev->active) {

            rev->ready = 1;
            rev->available = -1;
            rev->handler(rev);
        }
		
        wev = c->write;
        if ((revents & EPOLLOUT) && wev->active ) {
			
            wev->ready = 1;
            wev->handler(wev);
			del_event(wev,0,CLOSE_EVENT);//WRITE  会重复触发，需要清除
        }
    }

    return 0;
}



