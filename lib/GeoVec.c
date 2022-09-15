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

#include "proteus/GeoVec.h"
#include "ScalarConv_internal.h"
#include "Constants.h"

PROTEUS_API void proteus_GeoVec_add(proteus_GeoVec* v, const proteus_GeoVec* w)
{
	const double vx = v->mag * sin(ScalarConv_deg2rad(v->angle));
	const double vy = v->mag * cos(ScalarConv_deg2rad(v->angle));

	// The new vector components are the sums of the components of
	// w and the original v.
	const double dx = vx + (w->mag * sin(ScalarConv_deg2rad(w->angle)));
	const double dy = vy + (w->mag * cos(ScalarConv_deg2rad(w->angle)));

	// With our updated component vectors, Pythagoras helps us obtain the updated vector magnitude.
	v->mag = sqrt((dx * dx) + (dy * dy));

	if (fabs(dy) < PROTEUS_EPSILON)
	{
		// For very small dy values, just set the angle to either west or east
		// (depending on value of dx).
		if (dx < -PROTEUS_EPSILON)
		{
			v->angle = 270.0;
		}
		else if (dx > PROTEUS_EPSILON)
		{
			v->angle = 90.0;
		}
		else
		{
			// For very small dy and dx values, just set the angle to 0.
			v->angle = 0.0;
		}
	}
	else
	{
		// Normal angle calculation
		v->angle = ScalarConv_rad2deg(atan(dx / dy));

		if (dy < 0.0)
		{
			v->angle += 180.0;
		}
		else if (dx < 0.0)
		{
			v->angle += 360.0;
		}
	}
}
