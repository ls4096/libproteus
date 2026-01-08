/**
 * Copyright (C) 2022 ls4096 <ls4096@8bitbyte.ca>
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

#ifndef _ScalarConv_internal_h_
#define _ScalarConv_internal_h_


#include "Constants.h"


// Converts metres to nautical miles.
static inline double ScalarConv_m2nm(double m)
{
	return (m * (1.0 / PROTEUS_M_IN_NAUTICAL_MILE));
}

// Converts nautical miles to metres.
static inline double ScalarConv_nm2m(double nm)
{
	return (nm * PROTEUS_M_IN_NAUTICAL_MILE);
}

// Converts degrees to radians.
static inline double ScalarConv_deg2rad(double deg)
{
	return (deg * (1.0 / PROTEUS_DEG_IN_RAD));
}

// Converts radians to degrees.
static inline double ScalarConv_rad2deg(double rad)
{
	return (rad * PROTEUS_DEG_IN_RAD);
}


#endif // _ScalarConv_internal_h_
