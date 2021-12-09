#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

 
#include "connet.h"

int init_udp_conn(conn_t c,void *arg);

static int got_sig_term = 0;
static void on_sig_term(int sig)
{
	got_sig_term = 1;
	printf("term......\n\r");
}

		
int server_read_handle(event_t ev)
{
	int r;
	
	conn_t c = (conn_t)ev->data;
	conn_t pc = (conn_t)c->data;//lc->ls_arg
	buf_t buf=c->readBuf;
	
	do{
		buf_extend(buf, 4096);
		r = c->recv(c,buf->tail,4096);
		if(r<=0){
			printf("没有数据:%s:%d\n",c->peer_ip,c->peer_port);
			break;
		}
		else{
			buf->size += r;
			buf->tail += r;
			
			printf("%s:%d recv>>%s\n",c->peer_ip,c->peer_port,buf->head);
			buf_consume(buf, r);
			pc->send(pc,(u_char *)"ok ok ok~~",10);
		}
	}while(ev->ready);
	return 0;
}

int server_read_handle2(event_t ev)
{
	int r;
	
	conn_t c = (conn_t)ev->data;
	conn_t pc = (conn_t)c->data;//lc->ls_arg
	buf_t buf=c->readBuf;
	
	do{
		buf_extend(buf, 4096);
		r = c->recv(c,buf->tail,4096);
		if(r<=0){
			printf("没有数据:%s:%d\n",c->peer_ip,c->peer_port);
			break;
		}
		else{
			buf->size += r;
			buf->tail += r;
			
			printf("%s:%d r>>>%s\n",c->peer_ip,c->peer_port,buf->head);
			buf_consume(buf, r);
		}
	}while(ev->ready);
	return 0;
}


int server_write_handle(event_t ev)
{
	int n;
	conn_t c =(conn_t) ev->data;
	n = c->send(c,(u_char *)"send again",10);
	if(!c->write->ready) printf("only send %d bytes\n",n);
	return 0;
}

int init_udp_conn(conn_t c,void *arg)
{
	c->read->handler = server_read_handle;
	c->write->handler = server_write_handle;
	c->data = arg;//lc->ls_arg
	return 0;
}

int main()
{
	
	conn_t lc,pc,pc1;
	pooList_t list ;
	msec64 t,delta=0;
	
	signal(SIGTERM, on_sig_term);
	signal(SIGQUIT, on_sig_term);
	signal(SIGINT, on_sig_term);
	
	init_conn_queue();
	init_timer();
	init_epoll();
	list = create_pool_list();

	connect_peer_udp("172.16.10.210",60000,&pc);
	pc->read->handler =  server_read_handle2;
	pc->write->handler = server_write_handle;
	pc->data = NULL;
	
	lc = create_listening_udp(11000);
	init_udp_conn(lc,pc);
	
	
	while(!got_sig_term)
	{
		t = find_timer();
		process_events(t,1);
		if(get_current_ms() -delta) {
			expire_timers();
			delta = get_current_ms();
		}
	}
	
	free_all_conn();
	free_pool_list(list);
	free_timer();
	free_epoll();
	
	return 0;
}

