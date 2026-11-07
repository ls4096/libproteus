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

#include <unistd.h>

#include "tests.h"
#include "tests_assert.h"

#include "proteus/BackgroundWorker.h"

#define MAX_JOBS 16
#define SECONDS_PER_ITER 1


static int testWorkerFunc(void* arg)
{
	const int a = (intptr_t)arg;
	if (a == 1 || a == 2)
	{
		usleep(a * 10000);
	}

	if (a > 2)
	{
		return 1;
	}

	return 0;
}


int test_BackgroundWorker_run()
{
	// Invalid parameter values
	EQUALS(2, proteus_BackgroundWorker_init(0, 0));
	EQUALS(2, proteus_BackgroundWorker_init(MAX_JOBS, 0));
	EQUALS(2, proteus_BackgroundWorker_init(0, SECONDS_PER_ITER));

	EQUALS(0, proteus_BackgroundWorker_init(MAX_JOBS, SECONDS_PER_ITER));

	// Already initialized
	EQUALS(1, proteus_BackgroundWorker_init(MAX_JOBS, SECONDS_PER_ITER));

	// Invalid job
	EQUALS(-2, proteus_BackgroundWorker_addJob(0, 0, 0));

	int job1 = proteus_BackgroundWorker_addJob(&testWorkerFunc, 0, (void*)1);
	IS_TRUE(job1 >= 0);

	int job2 = proteus_BackgroundWorker_addJob(&testWorkerFunc, 1, (void*)2);
	IS_TRUE(job2 >= 0);
	IS_TRUE(job2 != job1);

	// Invalid jobs IDs for removal
	EQUALS(-1, proteus_BackgroundWorker_removeJob(-1));
	EQUALS(-1, proteus_BackgroundWorker_removeJob(MAX_JOBS));
	EQUALS(-1, proteus_BackgroundWorker_removeJob(MAX_JOBS + 100));

	sleep(2 * SECONDS_PER_ITER);

	int job3 = proteus_BackgroundWorker_addJob(&testWorkerFunc, 0, (void*)1);
	IS_TRUE(job3 >= 0);
	IS_TRUE(job3 != job1);
	IS_TRUE(job3 != job2);

	int job4 = proteus_BackgroundWorker_addJob(&testWorkerFunc, 2, (void*)2);
	IS_TRUE(job4 >= 0);
	int job5 = proteus_BackgroundWorker_addJob(&testWorkerFunc, 1, (void*)3);
	IS_TRUE(job5 >= 0);

	// Invalid jobs IDs for removal
	EQUALS(-1, proteus_BackgroundWorker_removeJob(-1));
	EQUALS(-1, proteus_BackgroundWorker_removeJob(MAX_JOBS));
	EQUALS(-1, proteus_BackgroundWorker_removeJob(MAX_JOBS + 100));

	sleep(2 * SECONDS_PER_ITER);

	// Invalid jobs IDs for removal
	EQUALS(-1, proteus_BackgroundWorker_removeJob(-1));
	EQUALS(-1, proteus_BackgroundWorker_removeJob(MAX_JOBS));
	EQUALS(-1, proteus_BackgroundWorker_removeJob(MAX_JOBS + 100));

	EQUALS(0, proteus_BackgroundWorker_removeJob(job3));
	EQUALS(1, proteus_BackgroundWorker_removeJob(job3)); // Already removed

	EQUALS(0, proteus_BackgroundWorker_removeJob(job5));
	EQUALS(0, proteus_BackgroundWorker_removeJob(job1));
	EQUALS(0, proteus_BackgroundWorker_removeJob(job2));
	EQUALS(0, proteus_BackgroundWorker_removeJob(job4));

	// Already removed
	EQUALS(1, proteus_BackgroundWorker_removeJob(job1));
	EQUALS(1, proteus_BackgroundWorker_removeJob(job2));
	EQUALS(1, proteus_BackgroundWorker_removeJob(job3));
	EQUALS(1, proteus_BackgroundWorker_removeJob(job4));
	EQUALS(1, proteus_BackgroundWorker_removeJob(job5));


	int jobIds[MAX_JOBS];

	// Add max jobs
	for (int i = 0; i < MAX_JOBS; i++)
	{
		jobIds[i] = proteus_BackgroundWorker_addJob(&testWorkerFunc, i >> 2, 0);
		IS_TRUE(jobIds[i] >= 0);
	}

	// Reached max jobs for BackgroundWorker
	IS_TRUE(proteus_BackgroundWorker_addJob(&testWorkerFunc, 0, 0) < 0);
	IS_TRUE(proteus_BackgroundWorker_addJob(&testWorkerFunc, 0, 0) < 0);

	sleep(SECONDS_PER_ITER);

	// Remove one job and add one job
	for (int i = 0; i < MAX_JOBS; i++)
	{
		EQUALS(0, proteus_BackgroundWorker_removeJob(jobIds[i]));
		jobIds[i] = proteus_BackgroundWorker_addJob(&testWorkerFunc, i >> 1, 0);
		IS_TRUE(jobIds[i] >= 0);
	}

	// Reached max jobs for BackgroundWorker
	IS_TRUE(proteus_BackgroundWorker_addJob(&testWorkerFunc, 0, 0) < 0);
	IS_TRUE(proteus_BackgroundWorker_addJob(&testWorkerFunc, 0, 0) < 0);

	sleep(SECONDS_PER_ITER);

	// Remove all jobs
	for (int i = 0; i < MAX_JOBS; i++)
	{
		EQUALS(0, proteus_BackgroundWorker_removeJob(jobIds[i]));
	}

	// Attempt to remove all jobs again
	for (int i = 0; i < MAX_JOBS; i++)
	{
		EQUALS(1, proteus_BackgroundWorker_removeJob(jobIds[i]));
	}


	return 0;
}
