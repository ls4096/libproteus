#include "tests.h"
#include "tests_assert.h"

#include "proteus/Weather.h"

#define WEATHER_DIR_1 "./test_data/weather_f1/"
#define WEATHER_DIR_2 "./test_data/weather_f1/"

int test_Weather_run()
{
	// TODO: Need a more complete test. This just sanity checks that we read the data,
	//       and that some of the weather items were filled in correctly (temperature and dewpoint).

	if (0 != proteus_Weather_init(WEATHER_DIR_1, WEATHER_DIR_2))
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
