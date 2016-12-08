#include <stdio.h>
#include <stdarg.h>

#include "RT.h"

void xerxzema_print(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}
