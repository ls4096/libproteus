#include "tests.h"
#include "tests_assert.h"

#include "proteus/GeoVec.h"

int test_GeoVec_run()
{
	const proteus_GeoVec zeroVec = { .angle = 0.0, .mag = 0.0 };

	proteus_GeoVec v;
	proteus_GeoVec vCopy;


	// Zero vector added to zero vector should not change.
	v.angle = 0.0;
	v.mag = 0.0;
	vCopy.angle = v.angle;
	vCopy.mag = v.mag;

	proteus_GeoVec_add(&v, &zeroVec);
	EQUALS_DBL(v.angle, vCopy.angle);
	EQUALS_DBL(v.mag, vCopy.mag);


	// Zero vector added to non-zero vector should not change non-zero vector.
	for (double a = 0.0; a < 360.0; a += 4.7)
	{
		for (double m = 0.1; m < 10.0; m += 2.3)
		{
			v.angle = a;
			v.mag = m;
			vCopy.angle = v.angle;
			vCopy.mag = v.mag;

			proteus_GeoVec_add(&v, &zeroVec);
			EQUALS_DBL(v.angle, vCopy.angle);
			EQUALS_DBL(v.mag, vCopy.mag);


			// Vector added to itself should not change angle, and magnitude should double.
			proteus_GeoVec_add(&v, &v);
			EQUALS_DBL(v.angle, vCopy.angle);
			EQUALS_DBL(v.mag, vCopy.mag * 2.0);
		}
	}


	// Vector added to its opposite (by magnitude) should result in zero vector.
	for (double a = 0.0; a < 360.0; a += 3.7)
	{
		v.angle = a;
		v.mag = 1.0;
		vCopy.angle = v.angle;
		vCopy.mag = -v.mag;

		proteus_GeoVec_add(&v, &vCopy);
		EQUALS_DBL(v.angle, zeroVec.angle);
		EQUALS_DBL(v.mag, zeroVec.mag);
	}


	// Vector added to its opposite (by angle) should result in zero vector.
	for (double a = 0.0; a < 360.0; a += 3.7)
	{
		v.angle = a;
		v.mag = 1.0;
		vCopy.angle = v.angle + 180.0 - (v.angle > 180.0 ? 360.0 : 0.0);
		vCopy.mag = v.mag;

		proteus_GeoVec_add(&v, &vCopy);
		EQUALS_DBL(v.angle, zeroVec.angle);
		EQUALS_DBL(v.mag, zeroVec.mag);
	}


	// Northward pointing vector added to its mirror (through y axis) should result in zero angle.
	for (double a = 0.0; a < 360.0; a += 3.7)
	{
		if (a >= 90.0 && a < 180.0)
		{
			// Skip southward pointing vectors.
			a += 180.0;
		}

		v.angle = a;
		v.mag = 1.5;
		vCopy.angle = 360.0 - v.angle;
		vCopy.mag = v.mag;

		proteus_GeoVec_add(&v, &vCopy);
		if (v.angle >= (360.0 - PROTEUS_DBL_EPSILON))
		{
			v.angle -= 360.0;
		}
		EQUALS_DBL(v.angle, 0.0);

		if (a < 60.0 || a > 300.0)
		{
			IS_TRUE(v.mag > vCopy.mag);
		}
		else if (a == 60.0 || a == 300.0)
		{
			EQUALS_DBL(v.mag, vCopy.mag);
		}
		else
		{
			IS_TRUE(v.mag < vCopy.mag);
		}
	}


	// Southward pointing vector added to its mirror (through y axis) should result in zero angle.
	for (double a = 90.1; a < 270.0; a += 3.7)
	{
		v.angle = a;
		v.mag = 1.5;
		vCopy.angle = 360.0 - v.angle;
		vCopy.mag = v.mag;

		proteus_GeoVec_add(&v, &vCopy);
		if (v.angle >= (360.0 - PROTEUS_DBL_EPSILON))
		{
			v.angle -= 360.0;
		}
		EQUALS_DBL(v.angle, 180.0);

		if (a > 120.0 && a < 240.0)
		{
			IS_TRUE(v.mag > vCopy.mag);
		}
		else if (a == 120.0 || a == 240.0)
		{
			EQUALS_DBL(v.mag, vCopy.mag);
		}
		else
		{
			IS_TRUE(v.mag < vCopy.mag);
		}
	}


	// Eastward pointing vector added to its mirror (through x axis) should result in 90.0 angle.
	for (double a = 0.1; a < 180.0; a += 3.7)
	{
		v.angle = a;
		v.mag = 1.5;
		vCopy.angle = 180.0 - v.angle;
		vCopy.mag = v.mag;

		proteus_GeoVec_add(&v, &vCopy);
		if (v.angle >= (360.0 - PROTEUS_DBL_EPSILON))
		{
			v.angle -= 360.0;
		}
		EQUALS_DBL(v.angle, 90.0);

		if (a > 30.0 && a < 150.0)
		{
			IS_TRUE(v.mag > vCopy.mag);
		}
		else if (a == 300.0 || a == 150.0)
		{
			EQUALS_DBL(v.mag, vCopy.mag);
		}
		else
		{
			IS_TRUE(v.mag < vCopy.mag);
		}
	}


	// Westward pointing vector added to its mirror (through x axis) should result in 270.0 angle.
	for (double a = 180.1; a < 360.0; a += 3.7)
	{
		v.angle = a;
		v.mag = 1.7;
		vCopy.angle = 540.0 - v.angle;
		vCopy.mag = v.mag;

		proteus_GeoVec_add(&v, &vCopy);
		if (v.angle >= (360.0 - PROTEUS_DBL_EPSILON))
		{
			v.angle -= 360.0;
		}
		EQUALS_DBL(v.angle, 270.0);

		if (a > 210.0 && a < 330.0)
		{
			IS_TRUE(v.mag > vCopy.mag);
		}
		else if (a == 210.0 || a == 330.0)
		{
			EQUALS_DBL(v.mag, vCopy.mag);
		}
		else
		{
			IS_TRUE(v.mag < vCopy.mag);
		}
	}


	return 0;
}
