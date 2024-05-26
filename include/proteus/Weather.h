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

#ifndef _proteus_Weather_h_
#define _proteus_Weather_h_

#include <stdbool.h>
#include <stdint.h>

#include <proteus/proteus.h>
#include <proteus/GeoPos.h>
#include <proteus/GeoVec.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/**
 * Various precipitation conditions
 */
#define PROTEUS_WX_COND_RAIN (0x01) // Rain
#define PROTEUS_WX_COND_SNOW (0x02) // Snow
#define PROTEUS_WX_COND_ICEP (0x04) // Ice pellets
#define PROTEUS_WX_COND_FRZR (0x08) // Freezing rain


/**
 * A structure containing various weather parameters
 */
typedef struct
{
	proteus_GeoVec wind; // Wind vector, using metres/second for magnitude
	                     // NOTE: Angle indicates direction wind is blowing FROM.

	float windGust; // Wind gust speed, in metres/second.
	float temp; // Temperature, in degrees Celsius
	float dewpoint; // Dew point temperature, in degrees Celsius
	float pressure; // Mean sea level pressure, in hectoPascals (hPa)
	float cloud; // Cloud coverage, in percent
	float visibility; // Visibility, in metres
	float prate; // Precipitation rate, in liquid millimetres/hour

	uint8_t cond; // Precipitation conditions (rain, snow, ice pellets, freezing rain)
} proteus_Weather;

#define PROTEUS_WEATHER_SOURCE_DATA_GRID_1P00 (0) // 1.00 degree grid
#define PROTEUS_WEATHER_SOURCE_DATA_GRID_0P50 (1) // 0.50 degree grid
#define PROTEUS_WEATHER_SOURCE_DATA_GRID_0P25 (2) // 0.25 degree grid

/**
 * Initializes the weather processing system.
 *
 * Assumes two forecast points, 3 hours apart (used for temporal interpolation).
 *
 * Parameters
 * 	sourceDataGrid [in]: the source data grid resolution
 * 	f1Dir [in]: the path to the directory with the first forecast point data
 * 	f2Dir [in]: the path to the directory with the second forecast point data
 *
 * Returns
 * 	0, on success
 * 	any other value, on failure
 */
PROTEUS_API int proteus_Weather_init(int sourceDataGrid, const char* f1Dir, const char* f2Dir);

/**
 * Provides weather information at the given geographical position.
 *
 * Parameters
 * 	pos [in]: the geographical position to be queried
 * 	wx [out]: the weather data structure to be populated
 * 	windOnly [in]: whether to supply only wind data (wind vector and gust speed)
 * 	               Setting this to true is useful as an optimization if only
 * 	               wind data is required, as the other values will not be
 * 	               unnecessarily computed.
 *
 * Returns
 * 	true, if weather data is available and valid at the provided position
 * 	false, if weather data is not available or not valid at the provided position
 */
PROTEUS_API bool proteus_Weather_get(const proteus_GeoPos* pos, proteus_Weather* wx, bool windOnly);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _proteus_Weather_h_
