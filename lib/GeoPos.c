#include <math.h>

#include "proteus/GeoPos.h"

#include "proteus/GeoVec.h"
#include "proteus/ScalarConv.h"


void proteus_GeoPos_advance(proteus_GeoPos* p, const proteus_GeoVec* v)
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
