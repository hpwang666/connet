#ifndef _CONNET_H
#define _CONNET_H

#include "core.h"
#include "connection.h"
#include "timer.h"
#include "event.h"


void connet_cycle();
void free_connet(pooList_t list);
pooList_t init_connet();


#endif
