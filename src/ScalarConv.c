/**
 * Copyright (C) 2020-2022 ls4096 <ls4096@8bitbyte.ca>
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

#include <math.h>

#include "proteus_internal.h"

#include "proteus/ScalarConv.h"
#include "ScalarConv_internal.h"

PROTEUS_API double proteus_ScalarConv_m2nm(double m)
{
	return ScalarConv_m2nm(m);
}

PROTEUS_API double proteus_ScalarConv_nm2m(double nm)
{
	return ScalarConv_nm2m(nm);
}

PROTEUS_API double proteus_ScalarConv_m2dlat(double m, double lat)
{
	const double rlat = ScalarConv_deg2rad(lat);

	// WGS84 spheroid calculation for length, in metres, of degree of latitude
	// at a given latitude.
	return (m / (111132.92 - \
		559.82 * cos(2 * rlat) + \
		1.175 * cos(4 * rlat) - \
		0.0023 * cos(6 * rlat)));
}

PROTEUS_API double proteus_ScalarConv_m2dlon(double m, double lat)
{
	const double rlat = ScalarConv_deg2rad(lat);

	// WGS84 spheroid calculation for length, in metres, of degree of longitude
	// at a given latitude.
	return (m / (111412.84 * cos(rlat) - \
		93.5 * cos(3 * rlat) + \
		0.118 * cos(5 * rlat)));
}

PROTEUS_API double proteus_ScalarConv_deg2rad(double deg)
{
	return ScalarConv_deg2rad(deg);
}

PROTEUS_API double proteus_ScalarConv_rad2deg(double rad)
{
	return ScalarConv_rad2deg(rad);
}
