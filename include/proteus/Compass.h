/**
 * Copyright (C) 2020-2021 ls4096 <ls4096@8bitbyte.ca>
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

#ifndef _proteus_Compass_h_
#define _proteus_Compass_h_

#include <time.h>

#include <proteus/proteus.h>
#include <proteus/GeoPos.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/**
 * Obtains the difference between two compass angles.
 *
 * Specifically, this provides the answer to the question:
 * How many degrees to travel from compass heading "a" to
 * compass heading "b", where negative values indicate "to
 * the left" and positive values indicate "to the right".
 *
 * Parameters
 * 	a [in]: the starting angle in degrees
 * 	b [in]: the finishing angle in degrees
 *
 * Returns the difference in degrees between the two angles,
 * in the range (-180, 180].
 */
PROTEUS_API double proteus_Compass_diff(double a, double b);

/**
 * Initializes the compass magnetic declination calculation system.
 *
 * Parameters
 * 	magDecFile [in]: the path to the file with the compass data
 *
 * Returns
 * 	0, on success
 * 	any other value, on failure
 */
PROTEUS_API int proteus_Compass_init(const char* magDecFile);

/**
 * Obtains the compass magnetic declination (from true north)
 * for the requested geographical position and observation time.
 *
 * A successful return from proteus_Compass_init() is required
 * before meaningful values are returned from this function.
 *
 * Parameters
 * 	pos [in]: the geographic position
 * 	t [in]: the time instant of the observation
 *
 * Returns the compass magnetic declination in degrees,
 * in the range (-180, 180].
 */
PROTEUS_API double proteus_Compass_magdec(const proteus_GeoPos* pos, time_t t);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _proteus_Compass_h_
