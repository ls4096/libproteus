#include <math.h>

#include "proteus/GeoVec.h"

#include "proteus/ScalarConv.h"

#include "Constants.h"

void proteus_GeoVec_add(proteus_GeoVec* v, const proteus_GeoVec* w)
{
	const double vx = v->mag * sin(proteus_ScalarConv_deg2rad(v->angle));
	const double vy = v->mag * cos(proteus_ScalarConv_deg2rad(v->angle));

	// The new vector components are the sums of the components of
	// w and the original v.
	const double dx = vx + (w->mag * sin(proteus_ScalarConv_deg2rad(w->angle)));
	const double dy = vy + (w->mag * cos(proteus_ScalarConv_deg2rad(w->angle)));

	// With our updated component vectors, Pythagoras helps us obtain the updated vector magnitude.
	v->mag = sqrt((dx * dx) + (dy * dy));

	if (fabs(dy) < EPSILON)
	{
		// For very small dy values, just set the angle to either west or east
		// (depending on value of dx).
		if (dx < -EPSILON)
		{
			v->angle = 270.0;
		}
		else if (dx > EPSILON)
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
		v->angle = proteus_ScalarConv_rad2deg(atan(dx / dy));

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
