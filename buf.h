/*
 * bio.h
 *
 *  Created on: 2014年10月22日
 *      Author: lily
 */

#ifndef _BIO_BUF_H
#define _BIO_BUF_H


typedef struct buf_st{
	u_char *heap;
	u_char *head, *tail;
	size_t capacity;
	size_t size;
}*buf_t;


#ifdef __cplusplus
extern "C"
{
#endif
buf_t    buf_new(size_t);
void         buf_free(buf_t b);
int          buf_extend(buf_t b, size_t grow);
int         buf_append(buf_t b, const void* src, size_t len);
int         buf_consume(buf_t b, size_t use);
int         buf_init(buf_t);

#ifdef __cplusplus
}
#endif

#endif /* BIO_H_ */
