#ifndef _CORE_H
#define _CORE_H


#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/un.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include <unistd.h>

#include <errno.h>
#include <fcntl.h>  
#include <stddef.h>  //offsetof

#include "rbtree.h"
#include "buf.h"
#include "palloc.h"
#include "sstr.h"

#define AIO_OK   	(0)
#define AIO_AGAIN   (-1)
#define AIO_ERR     (-2)

typedef enum{CONN_SERVER, CONN_CLIENT} conn_type;
typedef struct conn_st 			*conn_t;
typedef struct connQueue_st  	*connQueue_t;
typedef struct event_st *event_t;

typedef int (*io_handler)(conn_t c, u_char *buf, size_t size);
typedef int (*event_handler)(event_t ev);
typedef int (*listen_handler)(conn_t c,void *arg);




#endif
