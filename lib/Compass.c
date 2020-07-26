#include "proteus/Compass.h"


double proteus_Compass_diff(double a, double b)
{
	double c = b - a;

	if (c < 0.0)
	{
		c += 360.0;
	}

	return ((c > 180.0) ? (c - 360.0) : c);
}
