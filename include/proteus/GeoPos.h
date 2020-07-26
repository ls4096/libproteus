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

#ifndef _proteus_GeoPos_h_
#define _proteus_GeoPos_h_

#include "proteus/GeoVec.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/**
 * A geographical position
 */
typedef struct
{
	double lat; // in decimal degrees (negative values south of Equator)
	double lon; // in decimal degrees (negative values west of Prime Meridian)
} proteus_GeoPos;

/**
 * Adds the positional difference vector v to the position p.
 *
 * Parameters
 * 	p [in/out]: the position being modified
 * 	v [in]: the vector being added to position p
 */
void proteus_GeoPos_advance(proteus_GeoPos* p, const proteus_GeoVec* v);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _proteus_GeoPos_h_
