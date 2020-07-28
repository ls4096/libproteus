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

#ifndef _proteus_Compass_h_
#define _proteus_Compass_h_

#include <proteus/proteus.h>

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


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _proteus_Compass_h_
