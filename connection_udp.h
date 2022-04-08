#ifndef _CONNECTION_UDP_H
#define _CONNECTION_UDP_H

#include "core.h"




#ifdef __cplusplus
extern "C"
{
#endif

conn_t create_listening_udp(int port);
int connect_peer_udp(char *ip,int port,conn_t *c);

#ifdef __cplusplus
}
#endif

#endif
