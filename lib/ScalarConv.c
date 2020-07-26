#include <math.h>

#include "proteus/ScalarConv.h"

#include "Constants.h"


double proteus_ScalarConv_m2nm(double m)
{
	return (m / M_IN_NAUTICAL_MILE);
}

double proteus_ScalarConv_nm2m(double nm)
{
	return (nm * M_IN_NAUTICAL_MILE);
}

double proteus_ScalarConv_m2dlat(double m, double lat)
{
	const double rlat = proteus_ScalarConv_deg2rad(lat);

	// WGS84 spheroid calculation for length, in metres, of degree of latitude
	// at a given latitude.
	return (m / (111132.92 - \
		559.82 * cos(2 * rlat) + \
		1.175 * cos(4 * rlat) - \
		0.0023 * cos(6 * rlat)));
}

double proteus_ScalarConv_m2dlon(double m, double lat)
{
	const double rlat = proteus_ScalarConv_deg2rad(lat);

	// WGS84 spheroid calculation for length, in metres, of degree of longitude
	// at a given latitude.
	return (m / (111412.84 * cos(rlat) - \
		93.5 * cos(3 * rlat) + \
		0.118 * cos(5 * rlat)));
}

double proteus_ScalarConv_deg2rad(double deg)
{
	return (deg / DEG_IN_RAD);
}

double proteus_ScalarConv_rad2deg(double rad)
{
	return (rad * DEG_IN_RAD);
}
