#ifndef _SSTR_H
#define _SSTR_H
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C"
{
#endif
u_char *str_nstr(u_char *s1, char *s2, size_t len);

#ifdef __cplusplus
}
#endif

typedef struct str_st			*str_t;


struct str_st{
	u_char 	*data;
	size_t  	size;
	size_t 		len;
};

//snprintf  使用时要注意  n 必须大于实际字符串，否则末尾会被\0替代
//所以以下全部约定字符串长度必须小于str_t的长度
#define str_t_snprintf(a,b,...)  if((a->size - a->len)>b){ 			\
								snprintf((char *)(a->data+a->len),b,__VA_ARGS__);	\
								a->len=strlen(a->data);}			\
								else printf("err in str_t_snprintf %s:%d\n",__func__,__LINE__)

#define str_t_sprintf(a,...)  snprintf((char *)(a->data+a->len),(a->size - a->len),__VA_ARGS__);	\
								a->len=strlen((char *)(a->data))			
								
														

#define str_t_dup(a,b,c)   b =(str_t) palloc(a,sizeof(struct str_st));\
						b->size = b->len=strlen(c)	;						\
						b->data=(u_char *) palloc(a,b->size+1);  \
						memcpy(b->data,c,b->size);
						
						
#define str_t_ndup(a,b,c)   b =(str_t) palloc(a,sizeof(struct str_st));\
						b->size = c	;						\
						b->len = 0;							\
						b->data=(u_char *) palloc(a,b->size)
						

#define str_t_append(a,b,c)  if((a->size - a->len)>c){ \
							memcpy(a->data+a->len,b,c); \
							a->len+=c;}\
							else printf("err in str_t_append %s:%d\n",__func__,__LINE__)
							
							
#define str_t_cat(a,b)  if((a->size - a->len)>b->len){ \
							memcpy(a->data+a->len,b->data,b->len); \
							a->len+=b->len;}\
							else printf("err in str_t_cat %s:%d\n",__func__,__LINE__)						
							
						
#define str_t_zero(a) memset(a->data,0,a->size);\
						a->len =0
						
#endif


