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

#ifndef _proteus_ScalarConv_h_
#define _proteus_ScalarConv_h_

#include <proteus/proteus.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/**
 * Below are a number of simple but useful scalar value conversion functions.
 */

// Converts metres to nautical miles.
PROTEUS_API double proteus_ScalarConv_m2nm(double m);

// Converts nautical miles to metres.
PROTEUS_API double proteus_ScalarConv_nm2m(double m);

// Converts metres to geographical degrees of latitude, at the provided latitude.
PROTEUS_API double proteus_ScalarConv_m2dlat(double m, double lat);

// Converts metres to geographical degrees of longitude, at the provided latitude.
PROTEUS_API double proteus_ScalarConv_m2dlon(double m, double lat);

// Converts degrees to radians.
PROTEUS_API double proteus_ScalarConv_deg2rad(double deg);

// Converts radians to degrees.
PROTEUS_API double proteus_ScalarConv_rad2deg(double rad);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _proteus_ScalarConv_h_
