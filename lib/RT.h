#ifndef _XERXZEMA_RT_
#define _XERXZEMA_RT_

#include <stdint.h>

extern "C" {

void xerxzema_print(const char* fmt, ...);
void xerxzema_schedule(void* scheduler, void(*fn)(void*), void* state, uint64_t when);


};

#endif
