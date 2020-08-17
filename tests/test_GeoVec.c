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


	// Negative-magnitude vector added to half its opposite (with positive magnitude)
	// should result in half its original magnitude (positive sign magnitude but with opposite angle).
	for (double a = 0.0; a < 360.0; a += 3.7)
	{
		for (double m = 0.1; m < 10.0; m += 0.27)
		{
			v.angle = a;
			v.mag = -m; // "Negative" magnitude

			vCopy.angle = v.angle; // Same angle
			vCopy.mag = 0.5 * m; // Positive magnitude (half the original)

			proteus_GeoVec_add(&v, &vCopy);

			// This updated vCopy (opposite angle, same magnitude) is what we expect to have for updated v after adding original vCopy to original v...
			vCopy.angle += 180.0;
			if (vCopy.angle > 360.0)
			{
				vCopy.angle -= 360.0;
			}

			EQUALS_DBL(v.angle, vCopy.angle);
			EQUALS_DBL(v.mag, vCopy.mag);
		}
	}


	// Negative-magnitude vector added to 1.5x its opposite (with positive magnitude)
	// should result in half its original magnitude (positive sign magnitude with same angle).
	for (double a = 0.0; a < 360.0; a += 3.7)
	{
		for (double m = 0.1; m < 10.0; m += 0.27)
		{
			v.angle = a;
			v.mag = -m; // "Negative" magnitude

			vCopy.angle = v.angle; // Same angle
			vCopy.mag = 1.5 * m; // Positive magnitude (1.5x the original)

			proteus_GeoVec_add(&v, &vCopy);

			// This updated vCopy (same angle, half its magnitude but positive) is what we expect to have for updated v after adding original vCopy to original v...
			vCopy.mag = 0.5 * m;

			EQUALS_DBL(v.angle, vCopy.angle);
			EQUALS_DBL(v.mag, vCopy.mag);
		}
	}


	// Vector addition should be commutative.
	for (double a0 = 0.0; a0 < 360.0; a0 += 3.7)
	{
		for (double m0 = 0.1; m0 < 10.0; m0 += 0.27)
		{
			for (double a1 = 0.0; a1 < 360.0; a1 += 3.7)
			{
				for (double m1 = 0.1; m1 < 10.0; m1 += 1.13)
				{
					proteus_GeoVec v0 = { .angle = a0, .mag = m0 };
					proteus_GeoVec v1 = { .angle = a1, .mag = m1 };
					proteus_GeoVec v2 = { .angle = a1, .mag = m1 };

					proteus_GeoVec_add(&v1, &v0);
					proteus_GeoVec_add(&v0, &v2);

					EQUALS_DBL(v0.angle, v1.angle);
					EQUALS_DBL(v0.mag, v1.mag);
				}
			}
		}
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
