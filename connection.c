
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


static connQueue_t connQueue;
static conn_t sentinel;
static int _recv(conn_t c, u_char *buf, size_t size);
static int _send(conn_t c, u_char *buf, size_t size);

int init_conn_queue()
{
	connQueue = (connQueue_t) calloc(1,sizeof(struct connQueue_st));
	sentinel = (conn_t) calloc(1,sizeof(struct conn_st));
	connQueue->head=connQueue->tail = sentinel;
	connQueue->cache= NULL;
	return 0;
}
conn_t create_listening(int port)
{
	struct  sockaddr_in sa;
    int fd;
	int ret=0;
	conn_t c ;
	event_t    rev;
    if((fd=socket(AF_INET,SOCK_STREAM,0))<0)
    {
       exit(-1);
    }
    nonblocking(fd);
    sa.sin_family=AF_INET;
    sa.sin_port=htons(port);
    sa.sin_addr.s_addr=htonl(INADDR_ANY);
 
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
	
	c = get_conn(fd);
	rev = c->read;
	rev->handler = event_accept;
	add_event(rev, READ_EVENT);
	return c;
}
int connect_peer(char *ip,int port,conn_t *c)
{
	int fd;
	struct sockaddr_in sa;
	int rc;
	
	if( port <= 0 || ip == NULL) return -1;
	
	sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr(ip);	//inet_addr()完成地址格式转换
    sa.sin_port = htons(port);			//端口
	
    if((fd = socket(AF_INET,SOCK_STREAM,0)) < 0) return -1;

	nonblocking(fd);
	keepalive(fd);
	
    rc = connect(fd,(struct sockaddr*)&sa, sizeof(struct sockaddr_in));
	
    if(rc == 0)
    {
        debug("CONN:already ok?");
		*c= get_conn(fd);
		add_event((*c)->read, READ_EVENT);
		return AIO_OK;
    }
	
<<<<<<< HEAD
	printf(">>>conn errno %d \n",errno);
=======
	printf("connect errno:%d \n",errno);
	//如果 erron ！= EINPROGRESS  则是错误 ，就不应该返回AIO_AGAIN
	//if(rc == -1 && errno !=  EINPROGRESS)
	
>>>>>>> 407789846c265814af6b749c18d144a42e514be3
	if(rc == -1 && (CONN_WOULDBLOCK || CONN_INPROGRESS))//非阻塞都会执行到这一步
    {
        debug("CONN:need check\r\n");
		*c= get_conn(fd);
		memcpy((*c)->peer_ip,ip,strlen(ip));
		
		(*c)->peer_port = port;
		add_event((*c)->write, WRITE_EVENT);
		return AIO_AGAIN;
    }
	
	debug("CONN is something:%d\r\n",rc);
	close(fd);
	return AIO_ERR;
}


int test_connect(conn_t c)
{
    int        err = 0;
	int ret =0;
    socklen_t  len=sizeof(int);
	/*
	 * BSDs and Linux return 0 and set a pending error in err
	 * Solaris returns -1 and sets errno
	 */
	ret = getsockopt(c->fd, SOL_SOCKET, SO_ERROR, (void *) &err, &len);
	if (err) {
		return AIO_ERR;
	}
	
	if(ret == -1 ) return AIO_ERR;
    return AIO_OK;
}

//get a conn from queue
conn_t get_conn(int fd)
{
	conn_t c;
	event_t    rev,wev;
	buf_t buf;
	uintptr_t instance =0;
	if(connQueue->cache){
		c= connQueue->cache;
		connQueue->cache = connQueue->cache->next;
		connQueue->freeConn--;
		debug("get conn in cache\r\n");
		
		buf_init(c->readBuf);
		buf = c->readBuf;
		rev = c->read;
		wev = c->write;
		instance = c->instance;
		memset(c,0,sizeof(struct conn_st));
		
		c->readBuf = buf;
		c->read = rev;
		c->write = wev;
	}
	else {
		debug("get conn in calloc\r\n");
		c = (conn_t) calloc(1, sizeof(struct conn_st));
		c->readBuf = buf_new(32*1024);
		c->read = (event_t)calloc(1,sizeof(struct event_st));
		c->write = (event_t)calloc(1,sizeof(struct event_st));
	}
	c->next=c->prev = NULL;
	
	connQueue->tail->next = c; //add the new conn to the tail
	c->prev = connQueue->tail;
	connQueue->tail = c;
	connQueue->activeConn ++;
	
	//reinit the new conn
	
	memset(c->read,0,sizeof(struct event_st));
	memset(c->write,0,sizeof(struct event_st));
	
	c->instance = !instance;
	
	c->read->data = c;//conn被存储在event里，避免事件触发了要重新找conn
    c->write->data = c;
	c->fd = fd;
	c->recv = _recv;
	c->send = _send;
	
	return c;
}

static int _recv(conn_t c, u_char *buf, size_t size)
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


static int _send(conn_t c, u_char *buf, size_t size)
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


int free_conn(conn_t c)
{
	if(c->prev !=NULL)//if head
		c->prev->next = c->next;
	else connQueue->head = c->next;

	if(c->next!=NULL)//if tail
		c->next->prev = c->prev;
	else connQueue->tail = c->prev;
	
	c->next = connQueue->cache;
	c->prev = NULL;
	connQueue->cache = c; //into cache
	connQueue->freeConn++;
	connQueue->activeConn--;
	
	return 0;
}

int close_conn(conn_t c)
{
	int fd;
	if (c->fd == (-1)) {
        return 0;
    }

    if (c->read->timer_set) {
        del_timer(c->read);
    }

    if (c->write->timer_set) {
        del_timer(c->write);
    }
         
	if (c->read->active) {
		del_event(c->read,0,CLOSE_EVENT);
	}

	if (c->write->active ) {
		del_event(c->write,0,CLOSE_EVENT);
	}
        
    c->closed = 1;
    free_conn(c);
    fd = c->fd;
    c->fd = -1;
	close(fd);
	return 0;
}

int free_all_conn()
{
	conn_t c;
	conn_t findConn = connQueue->tail;
	
	while(findConn !=sentinel){//所有连接会被添加到cache
		c = findConn->prev;
		close_conn(findConn);
		findConn = c;
		printf("free conn head\r\n");
	}
	
	findConn = connQueue->cache;
	while(findConn !=NULL){
		buf_free(findConn->readBuf);
		free(findConn->read);
		free(findConn->write);
		c = findConn->next;
		free(findConn);
		findConn=c;
		printf("free conn cache\r\n");
	}
	free(sentinel);
	free(connQueue);
	connQueue=NULL;
	return 0;
}

int event_accept(event_t ev)
{
	struct  sockaddr_in sa;
	socklen_t addrlen = sizeof(sa);
	conn_t lc,c;
	int newfd;
	lc= ev->data;
	newfd =accept(lc->fd, (struct sockaddr*)&sa, &addrlen);
	nonblocking(newfd);
	keepalive(newfd);
	c = get_conn(newfd);
	set_conn_info(c);
	c->write->ready = 1;//默认其可写，不再判断
	lc->ls_handler(c);
	return 0;
}

//#define TCP_KEEPIDLE            4       /* Start keeplives after this period */
//#define TCP_KEEPINTVL           5       /* Interval between keepalives */
//#define TCP_KEEPCNT             6       /* Number of keepalives before death */

void keepalive(int sock)
{
	int keep = 1;
	int keepidle = 30;
	int keepintvl = 10;
	int keepcnt = 3;
	
	int tcp_nodelay = 1;
	setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, (char*)&keep, sizeof(keep));
	setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, (char*)&keepidle, sizeof(keepidle));
	setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, (char*)&keepintvl, sizeof(keepintvl));
	setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, (char*)&keepcnt, sizeof(keepcnt));
	
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,(const void *) &tcp_nodelay, sizeof(int));
}

int set_conn_info(conn_t c)
{
    struct sockaddr_in sa = {0};//sockaddr_in
    socklen_t namelen = sizeof(sa);
    char peer_ip[200] = {'\0'};
    char local_ip[200] = {'\0'};
	char *tmp;
	
    getsockname(c->fd, (struct sockaddr*)&sa, &namelen);
	tmp = inet_ntoa((&sa)->sin_addr);
	strncpy(local_ip,tmp,200);
   
    if(0 == strcmp(local_ip, "unix:@")){
    	snprintf(local_ip, sizeof(local_ip), "anonymous-sock-%d", c->fd);
    }
    strncpy(c->local_ip, local_ip, sizeof(c->local_ip) - 1);
    c->local_port =  ntohs((&sa)->sin_port);

    /* Server mode: get remote info by fd,
	   Client mode: known when connecting */
    
    memset(&sa, 0, namelen);
	getpeername(c->fd, (struct sockaddr *)&sa, &namelen);
	tmp = inet_ntoa((&sa)->sin_addr);
	strncpy(peer_ip,tmp,200);
	if( 0 == strncmp(peer_ip,"0.0.0.0",7)) return -1;
	
	if(strncmp(peer_ip,"0.0.0.0",7) == 0) return -1;
	strncpy(c->peer_ip, peer_ip, sizeof(c->peer_ip) - 1);
	c->peer_port = ntohs((&sa)->sin_port);
<<<<<<< HEAD
=======
	
>>>>>>> 407789846c265814af6b749c18d144a42e514be3
	return 0;
}