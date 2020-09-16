#ifndef _TIMER_H
#define _TIMER_H

#include "core.h"

#define timer_abs(value)       (((value) >= 0) ? (value) : - (value))
#define TIMER_LAZY_DELAY 200

#ifdef __cplusplus
extern "C"
{
#endif

void time_update(void);
int init_timer(void);
int free_timer();

void expire_timers(void);
int del_timer(event_t ev);
int add_timer(event_t ev, msec64 timer);
msec64 find_timer(void);
msec64 get_current_ms();

#ifdef __cplusplus
}
#endif

#endif
