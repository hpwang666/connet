
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


static int _recv_udp(conn_t c, u_char *buf, size_t size);
static int _send_udp(conn_t c, u_char *buf, size_t size);
static conn_t get_conn_udp(int fd);

conn_t create_listening_udp(int port)
{
	struct  sockaddr_in sa;
    int fd;
	conn_t c ;
	event_t    rev;
    if((fd=socket(AF_INET,SOCK_DGRAM,0))<0)
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
 
	c = get_conn_udp(fd);
	rev = c->read;
	rev->handler = NULL;
	add_event(rev, READ_EVENT);
	return c;
}
int connect_peer_udp(char *ip,int port,conn_t *c)
{
	int fd;
	struct sockaddr_in sa;
	int rc;
	
	if( port <= 0 || ip == NULL) return -1;
	
	sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr(ip);	//inet_addr()完成地址格式转换
    sa.sin_port = htons(port);			//端口
	
    if((fd = socket(AF_INET,SOCK_DGRAM,0)) < 0) return -1;

	nonblocking(fd);
	
    rc = connect(fd,(struct sockaddr*)&sa, sizeof(struct sockaddr_in));
	
    if(rc == 0)
    {
        debug("CONN:already ok?");
		*c= get_conn(fd);
		add_event((*c)->read, READ_EVENT);
		return AIO_OK;
    }
	

	//如果 erron ！= EINPROGRESS  则是错误 ，就不应该返回AIO_AGAIN
	//if(rc == -1 && errno !=  EINPROGRESS)

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


/*
相异于TCP通讯，UDP通讯没有粘包的问题，所以每个数据包都是一个独立的节点，所以有可能数据读完了
还有一个数据，所以需要连续的读取判断。
*/

static int _recv_udp(conn_t c, u_char *buf, size_t size)
{
    int       n;
    int     err;
    event_t  rev;
    rev = c->read;
    
	struct sockaddr_in clientaddr;
    socklen_t addrlen = sizeof(clientaddr);
   
	rev = c->read;
    n = recvfrom(c->fd, buf, size, 0,(struct sockaddr*)&clientaddr,&addrlen);
             
    if (n == 0) {
        rev->ready = 0;
        return 0;
    }
	
	sprintf(c->peer_ip,"%s",inet_ntoa((&clientaddr)->sin_addr));
	c->peer_port = ntohs((&clientaddr)->sin_port);
	
	
	err = errno;
	if (n > 0) {
		if (ioctl(c->fd, FIONREAD,&rev->available) == -1) {return -2;}
		//printf("n[%d] ava[%d]\n",n,rev->available);  
		rev->ready = (rev->available)?1:0;
		return n;
	}	
	if (err == EAGAIN || err == EINTR) {
		n = -1;
	} 
    rev->ready = 0;
    return n;
}


static int _send_udp(conn_t c, u_char *buf, size_t size)
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

//get a conn from queue
static conn_t get_conn_udp(int fd)
{
	conn_t c=get_conn(fd);
	c->recv = _recv_udp;
	c->send = _send_udp;
	
	return c;
}