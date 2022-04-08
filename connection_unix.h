#ifndef _CONNECTION_UNIX_H
#define _CONNECTION_UNIX_H

#include "core.h"




#ifdef __cplusplus
extern "C"
{
#endif


conn_t create_listening_unix(const char *sockPath);
int connect_peer_unix(const char *sockPath,conn_t *c);
#ifdef __cplusplus
}
#endif

#endif
