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

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * This file contains a list of some simple but useful scalar value conversion functions.
 */

// Converts metres to nautical miles.
double proteus_ScalarConv_m2nm(double m);

// Converts nautical miles to metres.
double proteus_ScalarConv_nm2m(double m);

// Converts metres to geographical degrees of latitude, at the provided latitude.
double proteus_ScalarConv_m2dlat(double m, double lat);

// Converts metres to geographical degrees of longitude, at the provided latitude.
double proteus_ScalarConv_m2dlon(double m, double lat);

// Converts degrees to radians.
double proteus_ScalarConv_deg2rad(double deg);

// Converts radians to degrees.
double proteus_ScalarConv_rad2deg(double rad);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _proteus_ScalarConv_h_
