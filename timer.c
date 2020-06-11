
#include "timer.h"
#include "event.h"

msec64   current_msec;
void time_update(void)
{
    time_t           sec;
    int       		msec;
    struct timeval   tv;

    gettimeofday(&tv, NULL);
    sec = tv.tv_sec;
    msec = tv.tv_usec / 1000;
	current_msec = sec * 1000 + msec;
}


  static rbtree_t 		timerRBtree;
  static rbtreeNode_t  	timerSentinel;
int init_timer()
{
    timerRBtree = (rbtree_t) calloc(1,sizeof(struct rbtree_st));
	timerSentinel = (rbtreeNode_t) calloc(1,sizeof(struct rbtreeNode_st));
	rbtree_init(timerRBtree, timerSentinel);
	time_update();	
    return 0;
}

int free_timer()
{
	free(timerSentinel);
	free(timerRBtree);
	timerSentinel=NULL;
	timerRBtree=NULL;
	return 0;
}

void expire_timers(void)
{
    event_t        ev;
    rbtreeNode_t  node, root;


    for ( ;; ) {
        root = timerRBtree->root;

        if (root == timerSentinel) {
            return;
        }

        node = rbtree_min(root, timerSentinel);

        /* node->key > ngx_current_msec */

        if ((msec64) (node->key - current_msec) > 0) {
            return;
        }

        ev = (event_t) ((char *) node - offsetof(struct event_st, timer));

        rbtree_delete(timerRBtree, &ev->timer);

        ev->timer.left = NULL;
        ev->timer.right = NULL;
        ev->timer.parent = NULL;

        ev->timer_set = 0;
        ev->timedout = 1;
        ev->handler(ev);
    }
}


int del_timer(event_t ev)
{
	rbtree_delete(timerRBtree, &ev->timer);
    ev->timer.left = NULL;
    ev->timer.right = NULL;
    ev->timer.parent = NULL;

    ev->timer_set = 0;
	return 0;
}

int add_timer(event_t ev, msec64 timer)
{
    msec64      key;
    msec64  diff;

    key = current_msec + timer;

    if (ev->timer_set) {
        /*
         * Use a previous timer value if difference between it and a new
         * value is less than NGX_TIMER_LAZY_DELAY milliseconds: this allows
         * to minimize the rbtree operations for fast connections.
         */

        diff = key - ev->timer.key;

        if (timer_abs(diff) < TIMER_LAZY_DELAY) {
            return 0;
        }

        del_timer(ev);
    }
    ev->timer.key = key;
    rbtree_insert(timerRBtree, &ev->timer);//每次都是add的一个节点，所以找到节点就可以用offsetof回退到event的地址

    ev->timer_set = 1;
	return 0;
}


msec64 find_timer(void)
{
    msec64      timer;
    rbtreeNode_t  node, root;

    root = timerRBtree->root;
    if (root == timerSentinel) {
            return TIMER_LAZY_DELAY; //未添加定时器，不能返回-1，会一直阻塞
    }

    node = rbtree_min(root, timerSentinel);

    timer = (msec64) (node->key - current_msec);

    return (msec64) (timer > 0 ? timer : 0);
}



