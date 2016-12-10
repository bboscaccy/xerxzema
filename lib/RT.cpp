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
