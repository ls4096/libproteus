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

#include "proteus/ScalarConv.h"

int test_ScalarConv_run()
{
	double previousLatD = 1.0; // Start high, since we expect this value to decrease.
	double previousLonD = 0.0; // Start low, since we expect this value to increase.

	for (double lat = 0.0; lat <= 90.0; lat += 0.1)
	{
		// As latitude increases, the "length" of a degree of latitude increases.
		// This means the number of degrees in a fixed distance decreases.
		IS_TRUE(proteus_ScalarConv_m2dlat(100.0, lat) < previousLatD);
		previousLatD = proteus_ScalarConv_m2dlat(100.0, lat);

		// As latitude increases, the "length" of a degree of longitude decreases.
		// This means the number of degrees in a fixed distance increases.
		IS_TRUE(proteus_ScalarConv_m2dlon(100.0, lat) > previousLonD);
		previousLonD = proteus_ScalarConv_m2dlon(100.0, lat);
	}

	return 0;
}
