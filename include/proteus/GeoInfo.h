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

#ifndef _proteus_GeoInfo_h_
#define _proteus_GeoInfo_h_

#include <stdbool.h>

#include <proteus/proteus.h>
#include <proteus/GeoPos.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/**
 * Initializes the geographic information (water/land data) processing system.
 *
 * Parameters
 * 	dataDir [in]: the path to the directory with the geographic information files
 *
 * Returns
 * 	0, on success
 * 	any other value, on failure
 */
PROTEUS_API int proteus_GeoInfo_init(const char* dataDir);

/**
 * Indicates whether or not water is present at the given geographical position.
 *
 * Parameters
 * 	pos [in]: the geographical position to be queried
 *
 * Returns
 * 	true, if on water
 * 	false, if on land
 */
PROTEUS_API bool proteus_GeoInfo_isWater(const proteus_GeoPos* pos);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _proteus_GeoInfo_h_
