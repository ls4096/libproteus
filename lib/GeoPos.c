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

#include "proteus/GeoPos.h"
#include "proteus/GeoVec.h"
#include "proteus/ScalarConv.h"

PROTEUS_API void proteus_GeoPos_advance(proteus_GeoPos* p, const proteus_GeoVec* v)
{
	const double vx = v->mag * sin(proteus_ScalarConv_deg2rad(v->angle));
	const double vy = v->mag * cos(proteus_ScalarConv_deg2rad(v->angle));

	const double lat = p->lat;

	p->lat += proteus_ScalarConv_m2dlat(vy, lat);

	// Clip latitudes outside the range of [-90, 90].
	if (p->lat > 90.0)
	{
		p->lat = 90.0;
	}
	else if (p->lat < -90.0)
	{
		p->lat = -90.0;
	}

	p->lon += proteus_ScalarConv_m2dlon(vx, lat);

	// Clip longitudes outside the range of [-180, 180].
	if (p->lon > 180.0)
	{
		p->lon -= 360.0;
	}
	else if (p->lon < -180.0)
	{
		p->lon += 360.0;
	}
}
