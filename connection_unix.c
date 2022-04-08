
#include "connection.h"
#include "event.h"
#include "timer.h"

#undef  _DEBUG
#define _DEBUG
#ifdef _DEBUG
	#define debug(...) printf(__VA_ARGS__)
#else
	#define debug(...)
#endif 

//Unix通讯也是属于stream流式方式，也需要考虑粘包的问题
static int _recv_unix(conn_t c, u_char *buf, size_t size);
static int _send_unix(conn_t c, u_char *buf, size_t size);

static int event_accept_unix(event_t ev);
static conn_t get_conn_unix(int fd);
conn_t create_listening_unix(const char *sockPath)
{
	struct  sockaddr_un sa;
    int fd;
	int ret=0;
	conn_t c ;
	event_t    rev;
	unlink(sockPath);
    if((fd=socket(AF_UNIX,SOCK_STREAM,0))<0)
    {
       exit(-1);
    }
    nonblocking(fd);
    sa.sun_family=AF_UNIX;
    strcpy(sa.sun_path,sockPath); 
 
    if((bind(fd,(struct sockaddr*)&sa,sizeof(sa))<0))
    {
        perror("bind failed");
		exit(-1);
    }
 
    if((ret=listen(fd,64))<0)
    {
        perror("listen failed");
        exit(-1);
    }
	
	c = get_conn_unix(fd);
	rev = c->read;
	rev->handler = event_accept_unix;
	add_event(rev, READ_EVENT);
	return c;
}
int connect_peer_unix(const char *sockPath,conn_t *c)
{
	int fd;
	struct sockaddr_un sa;
	int rc;
	
	if( sockPath== NULL) return -1;
	
	sa.sun_family = AF_UNIX;
    strcpy(sa.sun_path,sockPath); 
    if((fd = socket(AF_UNIX,SOCK_STREAM,0)) < 0) return -1;

	nonblocking(fd);
	
    rc = connect(fd,(struct sockaddr*)&sa, sizeof(struct sockaddr_un));
	
    if(rc == 0)
    {
        debug("CONN:already ok");
		*c= get_conn_unix(fd);
		add_event((*c)->read, READ_EVENT);
		return AIO_OK;
    }
	

	if(rc == -1)//非阻塞都会执行到这一步
    {
        debug("CONN:sock is not accessable\r\n");
		*c= get_conn_unix(fd);
		return AIO_ERR;
    }
	
	debug("CONN is something:%d\r\n",rc);
	close(fd);
	return AIO_ERR;
}



static int _recv_unix(conn_t c, u_char *buf, size_t size)
{
	int       n;
    int     err;
    event_t  rev;

    rev = c->read;
    
    n = recv(c->fd, buf, size, 0);
    if (n == 0) {
        rev->ready = 0;
        return 0;
    }
	err = errno;
	
	if (n > 0) {
		if (rev->available >= 0) {
			rev->available -= n;

			/*
			 * negative rev->available means some additional bytes
			 * were received between kernel notification and recv(),
			 * and therefore ev->ready can be safely reset even for
			 * edge-triggered event methods
			 */

			if (rev->available < 0) {
				rev->available = 0;
				rev->ready = 0;
			}

		} else if ((size_t) n == size) {//ready = 1
			if (ioctl(c->fd, FIONREAD,&rev->available) == -1) {
				return -2; 
			}
			else return n;// TRIGER AND READ AGAIN
		}
		if ((size_t) n < size) {
				rev->ready = 0;
				rev->available = 0;
		}
		return n;
	}	

	if (err == EAGAIN || err == EINTR) {
		n = -1;
	} 
    rev->ready = 0;
    return n;
}


static int _send_unix(conn_t c, u_char *buf, size_t size)
{
    int  n;
    int      err;
    event_t  wev;

    wev = c->write;
	n = send(c->fd, buf, size, 0);

	if (n > 0) {
		if ((size_t)n <  size) {
			wev->ready = 0;
		}
		c->sent += n;
		return n;
	}

	err = errno;
	if (n == 0) {
		wev->ready = 0;
		return n;
	}

	if (err == EAGAIN || err == EINTR) {
		wev->ready = 0;
		return -1;
	}
    return -1;
}



static conn_t get_conn_unix(int fd)
{
	conn_t c=get_conn(fd);
	c->recv = _recv_unix;
	c->send = _send_unix;
	
	return c;
}
static int event_accept_unix(event_t ev)
{
	struct  sockaddr_un sa;
	socklen_t addrlen = sizeof(sa);
	conn_t lc,c;
	int newfd;
	lc= ev->data;
	newfd =accept(lc->fd, (struct sockaddr*)&sa, &addrlen);
	nonblocking(newfd);
	c = get_conn_unix(newfd);
	c->write->ready = 1;//默认其可写，不再判断
	lc->ls_handler(c,lc->ls_arg);
	return 0;
}
