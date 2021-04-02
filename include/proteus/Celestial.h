/**
 * Copyright (C) 2021 ls4096 <ls4096@8bitbyte.ca>
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

#ifndef _proteus_Celestial_h_
#define _proteus_Celestial_h_

#include <proteus/proteus.h>
#include <proteus/GeoPos.h>

#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/**
 * Equatorial coordinates
 */
typedef struct
{
	double ra; // Right Ascension, in hours
	double dec; // Declination, in degrees
} proteus_CelestialEquatorialCoord;

/**
 * Horizontal coordinates
 */
typedef struct
{
	double az; // Azimuth, in degrees
	double alt; // Altitude, in degrees
} proteus_CelestialHorizontalCoord;

/**
 * Obtains the Julian Day value for the given Unix time.
 *
 * Parameters
 * 	t [in]: the time to convert
 *
 * Returns the Julian Day.
 */
PROTEUS_API double proteus_Celestial_getJulianDayForTime(time_t t);


/**
 * List of celestial object identifiers
 */
#define PROTEUS_CELESTIAL_OBJ_SUN			(0)
#define PROTEUS_CELESTIAL_OBJ_Achernar			(1)
#define PROTEUS_CELESTIAL_OBJ_Aldebaran			(2)
#define PROTEUS_CELESTIAL_OBJ_Rigel			(3)
#define PROTEUS_CELESTIAL_OBJ_Capella			(4)
#define PROTEUS_CELESTIAL_OBJ_Betelgeuse		(5)
#define PROTEUS_CELESTIAL_OBJ_Canopus			(6)
#define PROTEUS_CELESTIAL_OBJ_Sirius			(7)
#define PROTEUS_CELESTIAL_OBJ_Procyon			(8)
#define PROTEUS_CELESTIAL_OBJ_Pollux			(9)
#define PROTEUS_CELESTIAL_OBJ_Regulus			(10)
#define PROTEUS_CELESTIAL_OBJ_Acrux			(11)
#define PROTEUS_CELESTIAL_OBJ_Spica			(12)
#define PROTEUS_CELESTIAL_OBJ_Hadar			(13)
#define PROTEUS_CELESTIAL_OBJ_Arcturus			(14)
#define PROTEUS_CELESTIAL_OBJ_Rigil_Kentaurus		(15)
#define PROTEUS_CELESTIAL_OBJ_Antares			(16)
#define PROTEUS_CELESTIAL_OBJ_Vega			(17)
#define PROTEUS_CELESTIAL_OBJ_Altair			(18)
#define PROTEUS_CELESTIAL_OBJ_Deneb			(19)
#define PROTEUS_CELESTIAL_OBJ_Fomalhaut			(20)
#define PROTEUS_CELESTIAL_OBJ_Polaris			(21)

#define PROTEUS_CELESTIAL_OBJ_STAR_MIN (PROTEUS_CELESTIAL_OBJ_Achernar)
#define PROTEUS_CELESTIAL_OBJ_STAR_MAX (PROTEUS_CELESTIAL_OBJ_Polaris)
#define PROTEUS_CELESTIAL_OBJ_MAX (PROTEUS_CELESTIAL_OBJ_STAR_MAX)


/**
 * Obtains the equatorial coordinates for the requested celestial object.
 *
 * Parameters
 * 	jd [in]: the Julian Day
 * 	obj [in]: the requested celestial object identifier
 * 	ec [out]: the equatorial coordinate struct to fill with the result
 *
 * Returns 0 on success; non-zero otherwise.
 */
PROTEUS_API int proteus_Celestial_getEquatorialForObject(
	double jd,
	int obj,
	proteus_CelestialEquatorialCoord* ec
);

/**
 * Obtains the horizontal coordinates for provided equatorial coordinates
 * at the provided geographic position at the given time.
 *
 * Parameters
 * 	jd [in]: the Julian Day
 * 	pos [in]: the observer's geographic position
 * 	ec [in]: the provided equatorial coordinate struct
 * 	atmosEffect [in]: set to true to apply atmospheric refraction effects; false otherwise
 * 	airPressure [in]: air pressure (in hectoPascals) for atmospheric refraction calculation
 * 	airTemp [in]: air temperature (in degrees Celsius) for atmospheric refraction calculation
 * 	hc [out]: the horizontal coordinate struct to fill with the result
 *
 * Returns 0 on success; non-zero otherwise.
 */
PROTEUS_API int proteus_Celestial_convertEquatorialToHorizontal(
	double jd,
	const proteus_GeoPos* pos,
	const proteus_CelestialEquatorialCoord* ec,
	bool atmosEffect,
	double airPressure,
	double airTemp,
	proteus_CelestialHorizontalCoord* hc
);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _proteus_Celestial_h_
