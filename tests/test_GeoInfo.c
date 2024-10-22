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

#include "proteus/GeoInfo.h"

#define GEO_INFO_DATA_DIR "./test_data/geo/"

int test_GeoInfo_run()
{
	if (0 != proteus_GeoInfo_init(GEO_INFO_DATA_DIR))
	{
		return 1;
	}

	proteus_GeoPos p;


	p.lat = 44.6473;
	p.lon = -63.5804;
	IS_FALSE(proteus_GeoInfo_isWater(&p));

	p.lat = 44.6291;
	p.lon = -63.4592;
	IS_FALSE(proteus_GeoInfo_isWater(&p));

	p.lat = 44.6535;
	p.lon = -63.5638;
	IS_TRUE(proteus_GeoInfo_isWater(&p));

	p.lat = 44.5596;
	p.lon = -63.4970;
	IS_TRUE(proteus_GeoInfo_isWater(&p));


	double lat_min, lat_max;
	double lon_min, lon_max;

	lat_min = 44.08;
	lat_max = 44.54;
	lon_min = -63.271;
	lon_max = -52.0;
	for (double lat = lat_min; lat < lat_max; lat += 0.01)
	{
		for (double lon = lon_min; lon < lon_max; lon += 0.05)
		{
			IS_TRUE(proteus_GeoInfo_isWater(&p));
		}
	}

	lat_min = -60.0;
	lat_max = -49.0;
	lon_min = 89.0;
	lon_max = 120.0;
	for (double lat = lat_min; lat < lat_max; lat += 0.1)
	{
		for (double lon = lon_min; lon < lon_max; lon += 0.25)
		{
			IS_TRUE(proteus_GeoInfo_isWater(&p));
		}
	}

	return 0;
}
