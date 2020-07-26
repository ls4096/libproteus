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

#ifndef _proteus_GeoVec_h_
#define _proteus_GeoVec_h_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/**
 * A geographical vector, usually for describing a change in position
 * or a velocity, depending on context.
 */
typedef struct
{
	double angle; // in degrees
	double mag; // Unit depends on context (typically metres or metres/second).
} proteus_GeoVec;

/**
 * Adds the vector w to the vector v.
 *
 * Parameters
 * 	v [in/out]: the vector being modified
 * 	w [in]: the vector being added to vector v
 */
void proteus_GeoVec_add(proteus_GeoVec* v, const proteus_GeoVec* w);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _proteus_GeoVec_h_
