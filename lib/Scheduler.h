#pragma once

#include <stdint.h>

namespace xerxzema
{

struct CallbackState
{
	bool retry;
	uint64_t exec_time;
	char* opaque[];
};

typedef void(*scheduler_callback)(void*);

class Scheduler
{
public:
	void run();
	void schedule(scheduler_callback callback, void* state, uint64_t when);
	uint64_t calibrate_nanosleep();
};

uint64_t now();
void start_clock();

};
