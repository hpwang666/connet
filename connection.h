#ifndef _CONNECTION_H
#define _CONNECTION_H

#include "core.h"

#define nonblocking(s)  fcntl(s, F_SETFL, fcntl(s, F_GETFL) | O_NONBLOCK)
#define CONN_WOULDBLOCK (errno == EWOULDBLOCK || errno == EINTR || errno == EAGAIN)
#define CONN_INPROGRESS (errno == EINPROGRESS)
#define CONN_ConnRefused (errno == ECONNREFUSED)

struct conn_st {
    void           	*data;
    event_t        	read;
    event_t        	write;
    int        		fd;
	conn_type     	type;
	buf_t 			readBuf;
	

	conn_t 		next;
	conn_t 		prev;
	char 		local_ip[32];
	int 		local_port;
	char 		peer_ip[32];
	int 		peer_port;
	
	listen_handler    ls_handler;// handler of accepted connection 
    io_handler         recv;
    io_handler         send;
   
    int               sent;
	//unsigned 			triggered:1;  /*used to detect this fd if being triggered*/
    unsigned         	instance:1; /* used to detect the stale events in kqueue and epoll */
    unsigned            timedout:1;
   
    unsigned            closed:1;
};

struct connQueue_st{//Á¬½Ó³Ø
	conn_t head;
	conn_t tail;
	conn_t cache;
	int activeConn;
	int freeConn;
};  

#ifdef __cplusplus
extern "C"
{
#endif

int init_conn_queue();
conn_t create_listening(int port);

conn_t get_conn(int fd);
int close_conn(conn_t c);
int free_conn(conn_t c);
int free_all_conn();
int event_accept(event_t ev);

int connect_peer(char *ip,int port,conn_t *c);
int test_connect(conn_t c);
void keepalive(int sock);
void set_conn_info(conn_t c);

#ifdef __cplusplus
}
#endif

#endif
