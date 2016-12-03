#include "Scheduler.h"

#include <time.h>
#include <stdio.h>
#include <algorithm>

namespace xerxzema
{

timespec diff(timespec start, timespec end)
{
	timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

static struct timespec begin;

void start_clock()
{
	clock_gettime(CLOCK_REALTIME, &begin);
}



uint64_t now()
{
	static uint64_t counter = 0;
	static uint64_t last_stamp = 0;

	struct timespec current;
	uint64_t stamp = 0;
	clock_gettime(CLOCK_REALTIME, &current);
	struct timespec delta = diff(begin, current);
	stamp += delta.tv_sec * 1000000000;
	stamp += delta.tv_nsec;

	return stamp;
}

void Scheduler::run()
{
	struct timespec begin;
	struct timespec next;
	struct timespec remaining;
	struct timespec short_sleep = {0,1};

	uint64_t current = 0;
	uint64_t sys_current = 0;
	uint64_t last_sys_current = 0;
	uint64_t step_size = 100000;

	uint64_t max_sleep = calibrate_nanosleep();

	start_clock();

	uint64_t task_start = 0.5 * 1000000000;

	uint64_t sleep_skip = 0;
	uint64_t late_events = 0;

	uint64_t total_events = 0;
	double late_total = 0;

	while(true)
	{
		//in here we call now() and get the authoritative time
		//if now() hasn't changed since last now()
		//we fall back to clock_gettime to calculate roughly
		//where in the timesync window we actually are.
		sys_current = now();
		if(sys_current > last_sys_current)
		{
			current = sys_current;
			last_sys_current = sys_current;
			clock_gettime(CLOCK_REALTIME, &begin);
		}
		else
		{
			clock_gettime(CLOCK_REALTIME, &next);
			auto d = diff(begin, next);
			current = sys_current;
			current += d.tv_sec * 1000000000;
			current += d.tv_nsec;
		}

		while(task_start < current + step_size)
		{
			//fire it off...
			total_events++;
			if(current > task_start && current - task_start > step_size)
			{
				late_events++;
				late_total += current - task_start;
				printf("late %f\n", late_total / late_events);
				printf("ratio %f\n", (double)late_events / (double)total_events);
				printf("eps %f\n", (double)total_events / (double)(current / 1000000000));
			}
			task_start += 100000;
			//printf("FIRE! %ld\n", current);
		}

		sys_current = now();
		if(sys_current > last_sys_current)
		{
			current = sys_current;
			last_sys_current = sys_current;
			clock_gettime(CLOCK_REALTIME, &begin);
		}
		else
		{
			clock_gettime(CLOCK_REALTIME, &next);
			auto d = diff(begin, next);
			current = sys_current;
			current += d.tv_sec * 1000000000;
			current += d.tv_nsec;
		}

		auto when = task_start - current;
		when *= .5;
		if(when > max_sleep && task_start > current)
		{
			short_sleep.tv_nsec = when;
			nanosleep(&short_sleep, &remaining);
		}
		else
		{
			sleep_skip++;
		}

	}
}

uint64_t Scheduler::calibrate_nanosleep()
{

	struct timespec begin;
	struct timespec next;
	struct timespec short_sleep = {0,1};
	uint64_t sleep_time = 0;

	for(int i = 0; i < 10; i++)
	{
		clock_gettime(CLOCK_REALTIME, &begin);
		nanosleep(&short_sleep, &next);
		clock_gettime(CLOCK_REALTIME, &next);
		sleep_time += diff(begin, next).tv_nsec;
	}
	return sleep_time / 10;

};


};
