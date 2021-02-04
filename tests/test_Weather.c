/**
 * Copyright (C) 2020-2021 ls4096 <ls4096@8bitbyte.ca>
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
	//       and that some of the weather items were filled in correctly (mostly just temperature and dewpoint).

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

static int test_spatial_interpolation_1p00();

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
	EQUALS_FLT(12.166f, wx.windGust);

	p.lat = -75.0;
	p.lon = -180.0;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(246.061f - 273.15f, wx.temp);
	EQUALS_FLT(243.322f - 273.15f, wx.dewpoint);
	EQUALS_FLT(18.066f, wx.windGust);

	p.lat = -75.0;
	p.lon = 180.0;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(246.061f - 273.15f, wx.temp);
	EQUALS_FLT(243.322f - 273.15f, wx.dewpoint);
	EQUALS_FLT(18.066f, wx.windGust);


	if (test_spatial_interpolation_1p00() != 0)
	{
		return 1;
	}


	// Ensure no discontinuity across 1 degree north latitude...
	// There was a bug here where the band of latitude between the equator and 1 degree north was not
	// interpolating with all the expected points properly. If fixed, the checks below ought to pass.

	static const float EXPECTED_TEMP = 296.761f - 273.15f;
	static const float EXPECTED_DEWPOINT = 294.322f - 273.15f;
	static const float EXPECTED_WIND_GUST = 7.33380556f;

	p.lat = 1.0;
	p.lon = -80.0;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(EXPECTED_TEMP, wx.temp);
	EQUALS_FLT(EXPECTED_DEWPOINT, wx.dewpoint);
	EQUALS_FLT(EXPECTED_WIND_GUST, wx.windGust);

	p.lat = 0.999999;
	p.lon = -80.0;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(EXPECTED_TEMP, wx.temp);
	EQUALS_FLT(EXPECTED_DEWPOINT, wx.dewpoint);
	EQUALS_FLT(EXPECTED_WIND_GUST, wx.windGust);

	p.lat = 1.000001;
	p.lon = -80.0;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(EXPECTED_TEMP, wx.temp);
	EQUALS_FLT(EXPECTED_DEWPOINT, wx.dewpoint);
	EQUALS_FLT(EXPECTED_WIND_GUST, wx.windGust);


	return 0;
}

static int test_spatial_interpolation_1p00()
{
	proteus_GeoPos p;
	proteus_Weather wx;

	// A
	p.lat = -30.0;
	p.lon = -20.0;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(289.061f - 273.15f, wx.temp);

	// B
	p.lat = -30.0;
	p.lon = -19.0;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(289.261f - 273.15f, wx.temp);

	// C
	p.lat = -31.0;
	p.lon = -20.0;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(288.461f - 273.15f, wx.temp);

	// D
	p.lat = -31.0;
	p.lon = -19.0;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(288.761f - 273.15f, wx.temp);


	// 0.9A + 0.1B
	p.lat = -30.0;
	p.lon = -19.9;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT((0.9 * 289.061f + 0.1 * 289.261f) - 273.15f, wx.temp);

	// 0.9A + 0.1C
	p.lat = -30.1;
	p.lon = -20.0;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT((0.9 * 289.061f + 0.1 * 288.461f) - 273.15f, wx.temp);

	// 0.9C + 0.1D
	p.lat = -31.0;
	p.lon = -19.9;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT((0.9 * 288.461f + 0.1 * 288.761f) - 273.15f, wx.temp);

	// 0.9 * (0.9A + 0.1B) + 0.1 * (0.9C + 0.1D)
	p.lat = -30.1;
	p.lon = -19.9;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(((0.9 * 289.061f + 0.1 * 289.261f) * 0.9 + (0.9 * 288.461f + 0.1 * 288.761f) * 0.1) - 273.15f, wx.temp);


	return 0;
}


#define WEATHER_DIR_0P50_1 "./test_data/weather_0p50_f1/"
#define WEATHER_DIR_0P50_2 "./test_data/weather_0p50_f1/"

static int test_spatial_interpolation_0p50();

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
	EQUALS_FLT(11.3767f, wx.windGust);

	p.lat = -75.0;
	p.lon = -180.0;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(245.3f - 273.15f, wx.temp);
	EQUALS_FLT(241.278f - 273.15f, wx.dewpoint);
	EQUALS_FLT(21.1767f, wx.windGust);

	p.lat = -75.0;
	p.lon = 180.0;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(245.3f - 273.15f, wx.temp);
	EQUALS_FLT(241.278f - 273.15f, wx.dewpoint);
	EQUALS_FLT(21.1767f, wx.windGust);


	if (test_spatial_interpolation_0p50() != 0)
	{
		return 1;
	}


	return 0;
}

static int test_spatial_interpolation_0p50()
{
	proteus_GeoPos p;
	proteus_Weather wx;

	// A
	p.lat = -30.0;
	p.lon = -20.0;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(289.8f - 273.15f, wx.temp);

	// B
	p.lat = -30.0;
	p.lon = -19.5;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(289.9f - 273.15f, wx.temp);

	// C
	p.lat = -30.5;
	p.lon = -20.0;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(289.3f - 273.15f, wx.temp);

	// D
	p.lat = -30.5;
	p.lon = -19.5;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(289.5f - 273.15f, wx.temp);


	// 0.8A + 0.2B
	p.lat = -30.0;
	p.lon = -19.9;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT((0.8 * 289.8f + 0.2 * 289.9f) - 273.15f, wx.temp);

	// 0.8A + 0.2C
	p.lat = -30.1;
	p.lon = -20.0;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT((0.8 * 289.8f + 0.2 * 289.3f) - 273.15f, wx.temp);

	// 0.8C + 0.2D
	p.lat = -30.5;
	p.lon = -19.9;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT((0.8 * 289.3f + 0.2 * 289.5f) - 273.15f, wx.temp);

	// 0.8 * (0.8A + 0.2B) + 0.2 * (0.8C + 0.2D)
	p.lat = -30.1;
	p.lon = -19.9;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(((0.8 * 289.8f + 0.2 * 289.9f) * 0.8 + (0.8 * 289.3f + 0.2 * 289.5f) * 0.2) - 273.15f, wx.temp);


	return 0;
}


#define WEATHER_DIR_0P25_1 "./test_data/weather_0p25_f1/"
#define WEATHER_DIR_0P25_2 "./test_data/weather_0p25_f1/"

static int test_spatial_interpolation_0p25();

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
	EQUALS_FLT(4.02764f, wx.windGust);

	p.lat = -75.0;
	p.lon = -180.0;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(244.333f - 273.15f, wx.temp);
	EQUALS_FLT(240.74f - 273.15f, wx.dewpoint);
	EQUALS_FLT(17.8276f, wx.windGust);

	p.lat = -75.0;
	p.lon = 180.0;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(244.333f - 273.15f, wx.temp);
	EQUALS_FLT(240.74f - 273.15f, wx.dewpoint);
	EQUALS_FLT(17.8276f, wx.windGust);


	if (test_spatial_interpolation_0p25() != 0)
	{
		return 1;
	}


	return 0;
}

static int test_spatial_interpolation_0p25()
{
	proteus_GeoPos p;
	proteus_Weather wx;

	// A
	p.lat = 30.0;
	p.lon = 140.0;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(301.233f - 273.15f, wx.temp);

	// B
	p.lat = 30.0;
	p.lon = 140.25;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(301.333f - 273.15f, wx.temp);

	// C
	p.lat = 30.25;
	p.lon = 140.0;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(301.433f - 273.15f, wx.temp);

	// D
	p.lat = 30.25;
	p.lon = 140.25;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(301.233f - 273.15f, wx.temp);


	// 0.6A + 0.4B
	p.lat = 30.0;
	p.lon = 140.1;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT((0.6 * 301.233f + 0.4 * 301.333f) - 273.15f, wx.temp);

	// 0.6A + 0.4C
	p.lat = 30.1;
	p.lon = 140.0;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT((0.6 * 301.233f + 0.4 * 301.433f) - 273.15f, wx.temp);

	// 0.6C + 0.4D
	p.lat = 30.25;
	p.lon = 140.1;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT((0.6 * 301.433f + 0.4 * 301.233f) - 273.15f, wx.temp);

	// 0.6 * (0.6A + 0.4B) + 0.4 * (0.6C + 0.4D)
	p.lat = 30.1;
	p.lon = 140.1;
	proteus_Weather_get(&p, &wx, false);
	EQUALS_FLT(((0.6 * 301.233f + 0.4 * 301.333f) * 0.6 + (0.6 * 301.433f + 0.4 * 301.233f) * 0.4) - 273.15f, wx.temp);


	return 0;
}
