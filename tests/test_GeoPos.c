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

#include "proteus/GeoPos.h"

int test_GeoPos_run()
{
	const proteus_GeoVec zeroVec = { .angle = 0.0, .mag = 0.0 };

	proteus_GeoVec v;

	proteus_GeoPos pos;
	proteus_GeoPos posCopy;


	// Position advanced by zero vector should not change position.
	pos.lat = 0.0;
	pos.lon = 0.0;
	posCopy.lat = pos.lat;
	posCopy.lon = pos.lon;

	proteus_GeoPos_advance(&pos, &zeroVec);
	EQUALS_DBL(pos.lat, posCopy.lat);
	EQUALS_DBL(pos.lon, posCopy.lon);


	// With different angles...
	for (double a = 0.0; a < 360.0; a += 2.0)
	{
		// With different magnitudes...
		for (double m = 0.1; m < 1000.0; m += 80.3)
		{
			pos.lat = -10.0 + (a * 0.1);
			pos.lon = 10.0 - (m * 0.05);

			posCopy.lat = pos.lat;
			posCopy.lon = pos.lon;

			v.angle = a;
			v.mag = m;

			proteus_GeoPos_advance(&pos, &v);

			if (a > 270)
			{
				IS_TRUE(pos.lat > posCopy.lat);
				IS_TRUE(pos.lon < posCopy.lon);
			}
			else if (a == 270.0)
			{
				EQUALS(pos.lat, posCopy.lat);
				IS_TRUE(pos.lon < posCopy.lon);
			}
			else if (a > 180.0)
			{
				IS_TRUE(pos.lat < posCopy.lat);
				IS_TRUE(pos.lon < posCopy.lon);
			}
			else if (a == 180.0)
			{
				IS_TRUE(pos.lat < posCopy.lat);
				EQUALS(pos.lon, posCopy.lon);
			}
			else if (a > 90.0)
			{
				IS_TRUE(pos.lat < posCopy.lat);
				IS_TRUE(pos.lon > posCopy.lon);
			}
			else if (a == 90.0)
			{
				EQUALS(pos.lat, posCopy.lat);
				IS_TRUE(pos.lon > posCopy.lon);
			}
			else if (a > 0.0)
			{
				IS_TRUE(pos.lat > posCopy.lat);
				IS_TRUE(pos.lon > posCopy.lon);
			}
			else // a == 0.0
			{
				IS_TRUE(pos.lat > posCopy.lat);
				EQUALS(pos.lon, posCopy.lon);
			}
		}
	}


	return 0;
}
