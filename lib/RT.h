#ifndef _XERXZEMA_RT_
#define _XERXZEMA_RT_

#include <stdint.h>

extern "C" {

typedef void* xerxzema_scheduler;
typedef void(*xerxzema_scheduler_fn)(void* data);

void xerxzema_print(const char* fmt, ...);
xerxzema_scheduler xerxzema_create_scheduler();
void xerxzema_destroy_scheduler(xerxzema_scheduler);
void xerxzema_runone(xerxzema_scheduler scheduler, xerxzema_scheduler_fn startup, void* data);
void xerxzema_run(xerxzema_scheduler scheduler, xerxzema_scheduler_fn startup, void* data);
void xerxzema_schedule_relative(xerxzema_scheduler scheduler, xerxzema_scheduler_fn fn,
								uint64_t nanoseconds);
void xerxzema_schedule_now(xerxzema_scheduler scheduler, xerxzema_scheduler_fn fn,
						   uint64_t nanoseconds);

};

#endif
