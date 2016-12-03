#pragma once

#include <stdint.h>

namespace xerxzema
{

class Scheduler
{
public:
	void run();
	uint64_t calibrate_nanosleep();
};

uint64_t now();
void start_clock();

};
