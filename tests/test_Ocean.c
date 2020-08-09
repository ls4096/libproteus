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

#include "proteus/Ocean.h"

#define OCEAN_DATA_FILE_1 "./test_data/ocean/f1.csv"
#define OCEAN_DATA_FILE_2 "./test_data/ocean/f1.csv"

static int test_spatial_interpolation();

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
	EQUALS_FLT(28.003f, od.surfaceTemp);
	EQUALS_FLT(35.396f, od.salinity);
	EQUALS_FLT(0.0f, od.ice);

	p.lat = -65.2;
	p.lon = 70.4;
	IS_TRUE(proteus_Ocean_get(&p, &od));
	EQUALS_FLT(-1.814f, od.surfaceTemp);
	EQUALS_FLT(33.737f, od.salinity);
	EQUALS_FLT(100.0f, od.ice);


	p.lat = -36.0;
	p.lon = 0.0;
	IS_TRUE(proteus_Ocean_get(&p, &od));
	EQUALS_FLT(14.381f, od.surfaceTemp);
	EQUALS_FLT(35.138f, od.salinity);
	EQUALS_FLT(0.0f, od.ice);

	p.lat = -36.0;
	p.lon = -0.0;
	IS_TRUE(proteus_Ocean_get(&p, &od));
	EQUALS_FLT(14.381f, od.surfaceTemp);
	EQUALS_FLT(35.138f, od.salinity);
	EQUALS_FLT(0.0f, od.ice);


	p.lat = -36.0;
	p.lon = -180.0;
	IS_TRUE(proteus_Ocean_get(&p, &od));
	EQUALS_FLT(15.617f, od.surfaceTemp);
	EQUALS_FLT(35.442f, od.salinity);
	EQUALS_FLT(0.0f, od.ice);

	p.lat = -36.0;
	p.lon = 180.0;
	IS_TRUE(proteus_Ocean_get(&p, &od));
	EQUALS_FLT(15.617f, od.surfaceTemp);
	EQUALS_FLT(35.442f, od.salinity);
	EQUALS_FLT(0.0f, od.ice);


	if (test_spatial_interpolation() != 0)
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
	EQUALS_FLT(13.612f, od.surfaceTemp);
	EQUALS_FLT(35.079f, od.salinity);
	EQUALS_FLT(0.0f, od.ice);

	// B
	p.lat = -40.0;
	p.lon = 20.4;
	IS_TRUE(proteus_Ocean_get(&p, &od));
	EQUALS_FLT(14.009f, od.surfaceTemp);
	EQUALS_FLT(35.055f, od.salinity);
	EQUALS_FLT(0.0f, od.ice);

	// C
	p.lat = -40.4;
	p.lon = 20.0;
	IS_TRUE(proteus_Ocean_get(&p, &od));
	EQUALS_FLT(14.531f, od.surfaceTemp);
	EQUALS_FLT(35.112f, od.salinity);
	EQUALS_FLT(0.0f, od.ice);

	// D
	p.lat = -40.4;
	p.lon = 20.4;
	IS_TRUE(proteus_Ocean_get(&p, &od));
	EQUALS_FLT(14.433f, od.surfaceTemp);
	EQUALS_FLT(35.09f, od.salinity);
	EQUALS_FLT(0.0f, od.ice);


	// 0.75A + 0.25B
	p.lat = -40.0;
	p.lon = 20.1;
	IS_TRUE(proteus_Ocean_get(&p, &od));
	EQUALS_FLT((13.612f * 0.75f) + (14.009f * 0.25f), od.surfaceTemp);
	EQUALS_FLT((35.079f * 0.75f) + (35.055f * 0.25f), od.salinity);
	EQUALS_FLT(0.0f, od.ice);

	// 0.75A + 0.25C
	p.lat = -40.1;
	p.lon = 20.0;
	IS_TRUE(proteus_Ocean_get(&p, &od));
	EQUALS_FLT((13.612f * 0.75f) + (14.531f * 0.25f), od.surfaceTemp);
	EQUALS_FLT((35.079f * 0.75f) + (35.112f * 0.25f), od.salinity);
	EQUALS_FLT(0.0f, od.ice);

	// 0.75C + 0.25D
	p.lat = -40.4;
	p.lon = 20.1;
	IS_TRUE(proteus_Ocean_get(&p, &od));
	EQUALS_FLT((14.531f * 0.75f) + (14.433f * 0.25f), od.surfaceTemp);
	EQUALS_FLT((35.112f * 0.75f) + (35.09f * 0.25f), od.salinity);
	EQUALS_FLT(0.0f, od.ice);

	// 0.75 * (0.75A + 0.25B) + 0.25 * (0.75C + 0.25D)
	p.lat = -40.1;
	p.lon = 20.1;
	IS_TRUE(proteus_Ocean_get(&p, &od));
	EQUALS_FLT(((13.612f * 0.75f) + (14.009f * 0.25f)) * 0.75f + ((14.531f * 0.75f) + (14.433f * 0.25f)) * 0.25f, od.surfaceTemp);
	EQUALS_FLT(((35.079f * 0.75f) + (35.055f * 0.25f)) * 0.75f + ((35.112f * 0.75f) + (35.09f * 0.25f)) * 0.25f, od.salinity);
	EQUALS_FLT(0.0f, od.ice);

	return 0;
}
