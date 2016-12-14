#pragma once

#include <stdint.h>
#include <queue>
#include <algorithm>
#include <thread>
#include <atomic>

namespace xerxzema
{

struct CallbackState
{
	bool retry;
	uint64_t exec_time;
	//char* opaque[];
};

typedef void(*scheduler_callback)(void*);

struct CallbackData
{
	CallbackState* state;
	scheduler_callback fn;
	uint64_t when;
};

inline bool operator < (const CallbackData& lhs, const CallbackData& rhs)
{
	return lhs.when < rhs.when;
}
inline bool operator > (const CallbackData& lhs, const CallbackData& rhs)
{
	return lhs.when > rhs.when;
}

class Scheduler
{
public:
	Scheduler();
	void run();
	void run_async();
	void wait();
	void shutdown();
	void schedule(scheduler_callback callback, void* state, uint64_t when);
	uint64_t calibrate_nanosleep();
	inline void exit_when_empty() { exit_if_empty = true; }
private:
	std::priority_queue<CallbackData, std::vector<CallbackData>,
						std::greater<CallbackData>> tasks;
	bool exit_if_empty;
	std::thread main_thread;
	std::atomic<bool> running;
};

uint64_t now();
void start_clock();

};
