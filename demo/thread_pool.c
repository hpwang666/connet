#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include "connet.h"


void ptz_handle(void *data, tpArg_t tpArg);
static int got_sig_term = 0;
static void on_sig_term(int sig)
{
	got_sig_term = 1;
	printf("term......\n\r");
}

int main()
{
	
	threadTask_t task;
	signal(SIGTERM, on_sig_term);
	signal(SIGQUIT, on_sig_term);
	signal(SIGINT, on_sig_term);
	
	char *a=(char *)calloc(1,32);
	char *b=(char *)calloc(1,32);

	sprintf(a,"this is tp arg0");
	sprintf(b,"this is handle arg0");
	threadPool_t tp = thread_pool_init("ecs");
	tp->tpArg->arg0=a;

	task = thread_task_alloc(tp, ptz_handle,b,strlen(b));
	if(task)
		thread_task_post(tp,task);

	while(!got_sig_term);
	thread_pool_destroy(tp);
	return 0;
}


void ptz_handle(void *data, tpArg_t tpArg)
{
	printf("in thread:%s\r\n",(char *)data);
	printf("in thread arg0:%s\r\n",(char *)tpArg->arg0);
	return ;
}


