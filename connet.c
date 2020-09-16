#include "core.h"
#include "connet.h"


extern msec64 current_msec;

pooList_t init_connet()
{	
	pooList_t list;
	init_conn_queue();
	init_timer();
	init_epoll();
	list = create_pool_list();
	return list;
}

void free_connet(pooList_t list)
{
	free_all_conn();
	free_pool_list(list);
	free_timer();
	free_epoll();
}

void connet_cycle()
{
	msec64 t,delta;
	t = find_timer();
	process_events(t,1);
	if(current_msec -delta) {
		expire_timers();
		delta = current_msec;
	}
}

