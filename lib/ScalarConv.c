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

#include <math.h>

#include "proteus_internal.h"

#include "proteus/ScalarConv.h"
#include "Constants.h"

PROTEUS_API double proteus_ScalarConv_m2nm(double m)
{
	return (m / M_IN_NAUTICAL_MILE);
}

PROTEUS_API double proteus_ScalarConv_nm2m(double nm)
{
	return (nm * M_IN_NAUTICAL_MILE);
}

PROTEUS_API double proteus_ScalarConv_m2dlat(double m, double lat)
{
	const double rlat = proteus_ScalarConv_deg2rad(lat);

	// WGS84 spheroid calculation for length, in metres, of degree of latitude
	// at a given latitude.
	return (m / (111132.92 - \
		559.82 * cos(2 * rlat) + \
		1.175 * cos(4 * rlat) - \
		0.0023 * cos(6 * rlat)));
}

PROTEUS_API double proteus_ScalarConv_m2dlon(double m, double lat)
{
	const double rlat = proteus_ScalarConv_deg2rad(lat);

	// WGS84 spheroid calculation for length, in metres, of degree of longitude
	// at a given latitude.
	return (m / (111412.84 * cos(rlat) - \
		93.5 * cos(3 * rlat) + \
		0.118 * cos(5 * rlat)));
}

PROTEUS_API double proteus_ScalarConv_deg2rad(double deg)
{
	return (deg / DEG_IN_RAD);
}

PROTEUS_API double proteus_ScalarConv_rad2deg(double rad)
{
	return (rad * DEG_IN_RAD);
}
