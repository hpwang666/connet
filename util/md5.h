
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */


#ifndef _MD5_H
#define _MD5_H

#include "core.h"

typedef struct md5_st *md5_t;
struct md5_st{
    uint64_t  bytes;
    uint32_t  a, b, c, d;
    u_char    buffer[64];
	u_char    tmp[16];
	char	result[64];
} ;

#ifdef __cplusplus
extern "C"
{
#endif
void md5_update(md5_t ctx, const void *data, size_t size);
void md5_final(md5_t ctx);

#ifdef __cplusplus
}
#endif

#endif 
