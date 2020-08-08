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

#include "proteus/Weather.h"

static int test_grid_1p00();
static int test_grid_0p50();
static int test_grid_0p25();

int test_Weather_run()
{
	// TODO: Need more complete tests. These just perform some basic sanity checks that we read the data,
	//       and that some of the weather items were filled in correctly (temperature and dewpoint).

	if (test_grid_1p00() != 0)
	{
		return 1;
	}

	if (test_grid_0p50() != 0)
	{
		return 1;
	}

	if (test_grid_0p25() != 0)
	{
		return 1;
	}

	return 0;
}


#define WEATHER_DIR_1P00_1 "./test_data/weather_1p00_f1/"
#define WEATHER_DIR_1P00_2 "./test_data/weather_1p00_f1/"

static int test_grid_1p00()
{
	if (0 != proteus_Weather_init(PROTEUS_WEATHER_SOURCE_DATA_GRID_1P00, WEATHER_DIR_1P00_1, WEATHER_DIR_1P00_2))
	{
		return 1;
	}

	proteus_GeoPos p;
	proteus_Weather wx;


	p.lat = 44.0;
	p.lon = -63.0;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(293.161f - 273.15f, wx.temp);
	EQUALS_FLT(290.822f - 273.15f, wx.dewpoint);

	p.lat = -75.0;
	p.lon = -180.0;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(246.061f - 273.15f, wx.temp);
	EQUALS_FLT(243.322f - 273.15f, wx.dewpoint);

	p.lat = -75.0;
	p.lon = 180.0;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(246.061f - 273.15f, wx.temp);
	EQUALS_FLT(243.322f - 273.15f, wx.dewpoint);


	return 0;
}


#define WEATHER_DIR_0P50_1 "./test_data/weather_0p50_f1/"
#define WEATHER_DIR_0P50_2 "./test_data/weather_0p50_f1/"

static int test_grid_0p50()
{
	if (0 != proteus_Weather_init(PROTEUS_WEATHER_SOURCE_DATA_GRID_0P50, WEATHER_DIR_0P50_1, WEATHER_DIR_0P50_2))
	{
		return 1;
	}

	proteus_GeoPos p;
	proteus_Weather wx;


	p.lat = 44.0;
	p.lon = -63.0;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(294.2f - 273.15f, wx.temp);
	EQUALS_FLT(293.778f - 273.15f, wx.dewpoint);

	p.lat = -75.0;
	p.lon = -180.0;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(245.3f - 273.15f, wx.temp);
	EQUALS_FLT(241.278f - 273.15f, wx.dewpoint);

	p.lat = -75.0;
	p.lon = 180.0;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(245.3f - 273.15f, wx.temp);
	EQUALS_FLT(241.278f - 273.15f, wx.dewpoint);


	return 0;
}


#define WEATHER_DIR_0P25_1 "./test_data/weather_0p25_f1/"
#define WEATHER_DIR_0P25_2 "./test_data/weather_0p25_f1/"

static int test_grid_0p25()
{
	if (0 != proteus_Weather_init(PROTEUS_WEATHER_SOURCE_DATA_GRID_0P25, WEATHER_DIR_0P25_1, WEATHER_DIR_0P25_2))
	{
		return 1;
	}

	proteus_GeoPos p;
	proteus_Weather wx;


	p.lat = 44.0;
	p.lon = -63.0;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(294.533f - 273.15f, wx.temp);
	EQUALS_FLT(290.04f - 273.15f, wx.dewpoint);

	p.lat = -75.0;
	p.lon = -180.0;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(244.333f - 273.15f, wx.temp);
	EQUALS_FLT(240.74f - 273.15f, wx.dewpoint);

	p.lat = -75.0;
	p.lon = 180.0;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(244.333f - 273.15f, wx.temp);
	EQUALS_FLT(240.74f - 273.15f, wx.dewpoint);


	return 0;
}
