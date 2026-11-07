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

#ifndef _proteus_BackgroundWorker_h_
#define _proteus_BackgroundWorker_h_

#include <proteus/proteus.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


typedef int (*proteus_BgWorkerFunc)(void* arg);

/**
 * Initializes the background worker job execution system.
 *
 * Parameters
 * 	maxJobs [in]: the maximum number of jobs that the system will be expected to handle at the same time
 * 	secondsPerIter [in]: the number of seconds per base iteration between job runs
 *
 * Returns
 * 	0, on success
 * 	positive value, on invalid parameter or invalid state (i.e. already initialized)
 * 	negative value, on internal error
 */
PROTEUS_API int proteus_BackgroundWorker_init(unsigned int maxJobs, unsigned int secondsPerIter);

/**
 * Adds a job to be periodically executed.
 *
 * Parameters
 * 	func [in]: the function to be executed periodically
 * 	log2Freq [in]: the log2 of the number of base iterations to skip between job runs
 * 	arg [in]: optional data passed to the job func() each time it is called
 *
 * Returns
 * 	0 or positive value, on success, returning the job ID
 * 	negative value, on error
 */
PROTEUS_API int proteus_BackgroundWorker_addJob(proteus_BgWorkerFunc func, unsigned int log2Freq, void* arg);

/**
 * Removes a previously added job.
 *
 * Parameters
 * 	id [in]: the job ID of the job to remove (from return value of proteus_BackgroundWorker_addJob())
 *
 * Returns
 * 	0, on success
 * 	1, if the provided job ID does not reference a valid job
 * 	negative value, on error
 */
PROTEUS_API int proteus_BackgroundWorker_removeJob(int id);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _proteus_BackgroundWorker_h_
