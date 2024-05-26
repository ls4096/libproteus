/**
 * Copyright (C) 2020-2024 ls4096 <ls4096@8bitbyte.ca>
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

#include <stdbool.h>

#include "tests.h"
#include "tests_assert.h"

#include "proteus/Ocean.h"

#define OCEAN_DATA_FILE_1 "./test_data/ocean/f1.csv"
#define OCEAN_DATA_FILE_2 "./test_data/ocean/f1.csv"

static int test_spatial_interpolation();
static int test_out_of_bounds_geo();

static bool validLonLat(double lon, double lat);

int test_Ocean_run()
{
	// TODO: Need a more complete test. This just sanity checks that we read the data,
	//       and that some of the weather items were filled in correctly (sea surface temperature and salinity).

	if (0 != proteus_Ocean_init(OCEAN_DATA_FILE_1, OCEAN_DATA_FILE_2))
	{
		return 1;
	}

	proteus_GeoPos p;
	proteus_OceanData od;


	p.lat = 55.0;
	p.lon = -100.0;
	IS_FALSE(proteus_Ocean_get(&p, &od));

	p.lat = -80.0;
	p.lon = 50.0;
	IS_FALSE(proteus_Ocean_get(&p, &od));

	p.lat = 40.0;
	p.lon = -60.0;
	IS_TRUE(proteus_Ocean_get(&p, &od));
	EQUALS_FLT(15.43f, od.surfaceTemp);
	EQUALS_FLT(35.318f, od.salinity);
	EQUALS_FLT(0.0f, od.ice);

	p.lat = -65.2;
	p.lon = 70.4;
	IS_TRUE(proteus_Ocean_get(&p, &od));
	EQUALS_FLT(-1.809f, od.surfaceTemp);
	EQUALS_FLT(33.661f, od.salinity);
	EQUALS_FLT(100.0f, od.ice);


	p.lat = -36.0;
	p.lon = 0.0;
	IS_TRUE(proteus_Ocean_get(&p, &od));
	EQUALS_FLT(18.601f, od.surfaceTemp);
	EQUALS_FLT(35.328f, od.salinity);
	EQUALS_FLT(0.0f, od.ice);

	p.lat = -36.0;
	p.lon = -0.0;
	IS_TRUE(proteus_Ocean_get(&p, &od));
	EQUALS_FLT(18.601f, od.surfaceTemp);
	EQUALS_FLT(35.328f, od.salinity);
	EQUALS_FLT(0.0f, od.ice);


	p.lat = -36.0;
	p.lon = -180.0;
	IS_TRUE(proteus_Ocean_get(&p, &od));
	EQUALS_FLT(19.611f, od.surfaceTemp);
	EQUALS_FLT(35.609f, od.salinity);
	EQUALS_FLT(0.0f, od.ice);

	p.lat = -36.0;
	p.lon = 180.0;
	IS_TRUE(proteus_Ocean_get(&p, &od));
	EQUALS_FLT(19.611f, od.surfaceTemp);
	EQUALS_FLT(35.609f, od.salinity);
	EQUALS_FLT(0.0f, od.ice);


	if (test_spatial_interpolation() != 0)
	{
		return 1;
	}

	if (test_out_of_bounds_geo() != 0)
	{
		return 1;
	}


	return 0;
}

static int test_spatial_interpolation()
{
	// Some basic spatial interpolation checks

	proteus_GeoPos p;
	proteus_OceanData od;


	// A
	p.lat = -40.0;
	p.lon = 20.0;
	IS_TRUE(proteus_Ocean_get(&p, &od));
	EQUALS_FLT(22.699f, od.surfaceTemp);
	EQUALS_FLT(35.278f, od.salinity);
	EQUALS_FLT(0.0f, od.ice);

	// B
	p.lat = -40.0;
	p.lon = 20.4;
	IS_TRUE(proteus_Ocean_get(&p, &od));
	EQUALS_FLT(23.228f, od.surfaceTemp);
	EQUALS_FLT(35.418f, od.salinity);
	EQUALS_FLT(0.0f, od.ice);

	// C
	p.lat = -40.4;
	p.lon = 20.0;
	IS_TRUE(proteus_Ocean_get(&p, &od));
	EQUALS_FLT(19.223f, od.surfaceTemp);
	EQUALS_FLT(35.288f, od.salinity);
	EQUALS_FLT(0.0f, od.ice);

	// D
	p.lat = -40.4;
	p.lon = 20.4;
	IS_TRUE(proteus_Ocean_get(&p, &od));
	EQUALS_FLT(22.964f, od.surfaceTemp);
	EQUALS_FLT(35.392f, od.salinity);
	EQUALS_FLT(0.0f, od.ice);


	// 0.75A + 0.25B
	p.lat = -40.0;
	p.lon = 20.1;
	IS_TRUE(proteus_Ocean_get(&p, &od));
	EQUALS_FLT((22.699f * 0.75f) + (23.228f * 0.25f), od.surfaceTemp);
	EQUALS_FLT((35.278f * 0.75f) + (35.418f * 0.25f), od.salinity);
	EQUALS_FLT(0.0f, od.ice);

	// 0.75A + 0.25C
	p.lat = -40.1;
	p.lon = 20.0;
	IS_TRUE(proteus_Ocean_get(&p, &od));
	EQUALS_FLT((22.699f * 0.75f) + (19.223f * 0.25f), od.surfaceTemp);
	EQUALS_FLT((35.278f * 0.75f) + (35.288f * 0.25f), od.salinity);
	EQUALS_FLT(0.0f, od.ice);

	// 0.75C + 0.25D
	p.lat = -40.4;
	p.lon = 20.1;
	IS_TRUE(proteus_Ocean_get(&p, &od));
	EQUALS_FLT((19.223f * 0.75f) + (22.964f * 0.25f), od.surfaceTemp);
	EQUALS_FLT((35.288f * 0.75f) + (35.392f * 0.25f), od.salinity);
	EQUALS_FLT(0.0f, od.ice);

	// 0.75 * (0.75A + 0.25B) + 0.25 * (0.75C + 0.25D)
	p.lat = -40.1;
	p.lon = 20.1;
	IS_TRUE(proteus_Ocean_get(&p, &od));
	EQUALS_FLT(((22.699f * 0.75f) + (23.228f * 0.25f)) * 0.75f + ((19.223f * 0.75f) + (22.964f * 0.25f)) * 0.25f, od.surfaceTemp);
	EQUALS_FLT(((35.278f * 0.75f) + (35.418f * 0.25f)) * 0.75f + ((35.288f * 0.75f) + (35.392f * 0.25f)) * 0.25f, od.salinity);
	EQUALS_FLT(0.0f, od.ice);

	return 0;
}

static int test_out_of_bounds_geo()
{
	// Some basic checks using out-of-bounds geographic coordinates

	for (double lon = -360.0; lon <= 360.0; lon += 0.1)
	{
		for (double lat = -180.0; lat <= 180.0; lat += 0.1)
		{
			if (validLonLat(lon, lat))
			{
				continue;
			}

			const proteus_GeoPos p = { .lat = lat, .lon = lon };
			proteus_OceanData od;

			IS_FALSE(proteus_Ocean_get(&p, &od));
		}
	}

	for (double lon = -36000.0; lon <= 36000.0; lon += 9.9)
	{
		for (double lat = -18000.0; lat <= 18000.0; lat += 9.9)
		{
			if (validLonLat(lon, lat))
			{
				continue;
			}

			const proteus_GeoPos p = { .lat = lat, .lon = lon };
			proteus_OceanData od;

			IS_FALSE(proteus_Ocean_get(&p, &od));
		}
	}


	return 0;
}

static bool validLonLat(double lon, double lat)
{
	return (lon >= -180.0 && lon <= 180.0 && lat >= -90.0 && lat <= 90.0);
}
