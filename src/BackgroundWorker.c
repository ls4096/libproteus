/**
 * Copyright (C) 2026 ls4096 <ls4096@8bitbyte.ca>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#include "proteus_internal.h"

#include "proteus/BackgroundWorker.h"
#include "ErrLog.h"

#define ERRLOG_ID "proteus_BgWorker"
#define BGWORKER_THREAD_NAME "proteus_BgWork"


typedef struct
{
	proteus_BgWorkerFunc func;
	unsigned int log2Freq;
	void* arg;
} WorkerJob;


static pthread_mutex_t _bgwLock;
static pthread_t _bgwThread;

static WorkerJob* _jobs = 0;
static unsigned int _maxJobs = 0;
static unsigned int _secondsPerIter = 0;


static void* bgWorkerThreadMain();
static bool shouldRunJob(const WorkerJob* job, unsigned int iter);


PROTEUS_API int proteus_BackgroundWorker_init(unsigned int maxJobs, unsigned int secondsPerIter)
{
	if (_maxJobs > 0)
	{
		// Already initialized
		return 1;
	}

	if (maxJobs < 1 || secondsPerIter < 1)
	{
		// Invalid parameter value
		return 2;
	}

	_maxJobs = maxJobs;
	_secondsPerIter = secondsPerIter;

	int rc = 0;

	if (0 != pthread_mutex_init(&_bgwLock, 0))
	{
		rc = -1;
		goto fail;
	}

	_jobs = malloc(_maxJobs * sizeof(WorkerJob));
	for (unsigned int i = 0; i < _maxJobs; i++)
	{
		_jobs[i].func = 0;
		_jobs[i].log2Freq = 0;
		_jobs[i].arg = 0;
	}

	if (0 != pthread_create(&_bgwThread, 0, &bgWorkerThreadMain, 0))
	{
		rc = -2;
		goto fail;
	}

#if defined(_GNU_SOURCE) && defined(__GLIBC__)
	if (0 != pthread_setname_np(_bgwThread, BGWORKER_THREAD_NAME))
	{
		ERRLOG1("Couldn't set thread name to %s. Continuing anyway.", BGWORKER_THREAD_NAME);
	}
#endif

fail:
	if (0 != rc)
	{
		free(_jobs);
		_jobs = 0;
		_maxJobs = 0;
		_secondsPerIter = 0;
	}

	return rc;
}

PROTEUS_API int proteus_BackgroundWorker_addJob(proteus_BgWorkerFunc func, unsigned int log2Freq, void* arg)
{
	if (!func)
	{
		return -2;
	}

	if (0 != pthread_mutex_lock(&_bgwLock))
	{
		ERRLOG("addJob: Failed to lock!");
		return -3;
	}

	int rc = -1;

	for (unsigned int i = 0; i < _maxJobs; i++)
	{
		if (!_jobs[i].func)
		{
			_jobs[i].func = func;
			_jobs[i].log2Freq = log2Freq;
			_jobs[i].arg = arg;

			rc = i;
			goto done;
		}
	}

done:
	if (0 != pthread_mutex_unlock(&_bgwLock))
	{
		ERRLOG("addJob: Failed to unlock!");
	}
	return rc;
}

PROTEUS_API int proteus_BackgroundWorker_removeJob(int id)
{
	if (id < 0 || id >= (int)_maxJobs)
	{
		return -1;
	}

	if (0 != pthread_mutex_lock(&_bgwLock))
	{
		ERRLOG("removeJob: Failed to lock!");
		return -2;
	}

	int rc = 1;

	if (_jobs[id].func)
	{
		_jobs[id].func = 0;
		_jobs[id].log2Freq = 0;
		_jobs[id].arg = 0;

		rc = 0;
	}

	if (0 != pthread_mutex_unlock(&_bgwLock))
	{
		ERRLOG("removeJob: Failed to unlock!");
	}
	return rc;
}


static void* bgWorkerThreadMain()
{
	unsigned int iter = 0;

	for (;;)
	{
		const time_t st = time(0);
		time_t dt;

		if (0 != pthread_mutex_lock(&_bgwLock))
		{
			ERRLOG("worker: Failed to lock!");
			goto sleepUntilNextIter;
		}

		for (unsigned int i = 0; i < _maxJobs; i++)
		{
			WorkerJob* job = _jobs + i;
			if (shouldRunJob(job, iter))
			{
				const int jobRc = job->func(job->arg);
				if (0 != jobRc)
				{
					ERRLOG2("worker: Job id %u returned %d!", i, jobRc);
				}
			}
		}

		if (0 != pthread_mutex_unlock(&_bgwLock))
		{
			ERRLOG("worker: Failed to unlock!");
		}

sleepUntilNextIter:
		dt = time(0) - st;
		if (dt < _secondsPerIter)
		{
			sleep(_secondsPerIter - dt);
		}

		iter++;
	}

	return 0;
}

static bool shouldRunJob(const WorkerJob* job, unsigned int iter)
{
	return (job->func && ((iter & ((1 << job->log2Freq) - 1)) == 0));
}
