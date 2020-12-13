/**
 * Copyright (C) 2020 ls4096 <ls4096@8bitbyte.ca>
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

#ifndef _proteus_Wave_h_
#define _proteus_Wave_h_

#include <stdbool.h>

#include <proteus/proteus.h>
#include <proteus/GeoPos.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/**
 * A structure containing wave condition parameters
 */
typedef struct
{
	float waveHeight; // Wave height, in metres
} proteus_WaveData;

/**
 * Initializes the wave data processing system.
 *
 * Assumes two forecast points, 12 hours apart (used for temporal interpolation).
 *
 * Parameters
 * 	f1File [in]: the path to the file with the first forecast point data
 * 	f2File [in]: the path to the file with the second forecast point data
 *
 * Returns
 * 	0, on success
 * 	any other value, on failure
 */
PROTEUS_API int proteus_Wave_init(const char* f1File, const char* f2File);

/**
 * Provides wave information, if available, at the given geographical position.
 *
 * Parameters
 * 	pos [in]: the geographical position to be queried
 * 	od [out]: the wave data structure to be populated
 *
 * Returns
 * 	true, if wave data is available and valid at the provided position
 * 	false, if wave data is not available or not valid at the provided position
 */
PROTEUS_API bool proteus_Wave_get(const proteus_GeoPos* pos, proteus_WaveData* wd);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _proteus_Wave_h_
