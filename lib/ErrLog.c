#include "ErrLog.h"

#include "proteus/Logging.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

static int _logFd = -1;

void proteus_Logging_setOutputFd(int logFd)
{
	_logFd = logFd;
}

void ErrLog_log(const char* id, const char* msg, ...)
{
	if (_logFd < 0)
	{
		return;
	}

	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);

	char fmt[4096];
	sprintf(fmt, "[%ld.%03ld] %s: %s\n", ts.tv_sec, ts.tv_nsec / 1000000, id, msg);

	va_list arg;
	va_start(arg, msg);
	vdprintf(_logFd, fmt, arg);
	va_end(arg);
}
