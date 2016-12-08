#include <stdio.h>
#include <stdarg.h>

#include "RT.h"

#include "Scheduler.h"

using namespace xerxzema;

void xerxzema_print(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}

xerxzema_scheduler xerxzema_create_scheduler()
{
	return new Scheduler();
}

void xerxzema_destroy_scheduler(xerxzema_scheduler sch)
{
	delete (Scheduler*)sch;
}

void xerxzema_run(xerxzema_scheduler scheduler, xerxzema_scheduler_fn startup, void* data)
{
	((Scheduler*)scheduler)->run();
}

void xerxzema_runone(xerxzema_scheduler scheduler, xerxzema_scheduler_fn startup, void* data)
{
	((Scheduler*)scheduler)->run();
}
