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

#include <math.h>

#include "proteus_internal.h"

#include "proteus/Celestial.h"
#include "proteus/ScalarConv.h"


static const double STAR_EPH_J2000[] = {
	// RA(h)	// DEC(deg)	// d-RA(mas/y)	// d-DEC(mas/y)
	1.628556,	-57.236757,	88.02,		-40.08,		// Achernar
	4.598677,	16.509301,	62.78,		-189.36,	// Aldebaran
	5.242298,	-8.201640,	1.87,		-0.56,		// Rigel
	5.278150,	45.997991,	75.52,		-427.13,	// Capella
	5.919529,	7.407063,	27.33,		10.86,		// Betelgeuse
	6.399195,	-52.695660,	19.99,		23.67,		// Canopus
	6.752481,	-16.716116,	-546.01,	-1223.08,	// Sirius
	7.655033,	5.224993,	-716.57,	-1034.58,	// Procyon
	7.755277,	28.026199,	-625.69,	-45.95,		// Pollux
	10.139532,	11.967207,	-249.40,	4.91,		// Regulus
	12.443311,	-63.099092,	-35.37,		-14.73,		// Acrux
	13.419883,	-11.161322,	-42.50,		-31.73,		// Spica
	14.063729,	-60.373039,	-33.96,		-25.06,		// Hadar
	14.261030,	19.182410,	-1093.45,	-1999.40,	// Arcturus
	14.660765,	-60.833976,	-3678.19,	481.84,		// Rigil Kentaurus
	16.490128,	-26.432002,	-10.16,		-23.21,		// Antares
	18.615640,	38.783692,	201.02,		287.46,		// Vega
	19.846388,	8.868322,	536.82,		385.54,		// Altair
	20.690532,	45.280338,	1.56,		1.55,		// Deneb
	22.960838,	-29.622236,	329.22,		-164.22,	// Fomalhaut
	2.529750,	89.264109,	44.22,		-11.74		// Polaris
};


static double computeObliquityForJulianCentury(double t);
static void computeStarEq(int obj, double jd, proteus_CelestialEquatorialCoord* ec);


PROTEUS_API double proteus_Celestial_getJulianDayForTime(time_t t)
{
	return ((double)t / 86400.0) + 2440587.5;
}

PROTEUS_API int proteus_Celestial_getEquatorialForObject(
	double jd,
	int obj,
	proteus_CelestialEquatorialCoord* ec
)
{
	if (obj < PROTEUS_CELESTIAL_OBJ_SUN || obj > PROTEUS_CELESTIAL_OBJ_Polaris)
	{
		// Invalid object
		return -1;
	}
	else if (obj > PROTEUS_CELESTIAL_OBJ_SUN)
	{
		// Star object
		computeStarEq(obj, jd, ec);
		return 0;
	}


	// Calculations for Sun below...

	const double N = jd - 2451545.0;
	const double T = N / 36525.0;
	const double L = fmod(280.460 + 0.9856474 * N, 360.0);
	const double G = fmod(357.528 + 0.9856003 * N, 360.0);

	const double LA = L +
		1.915 * sin(proteus_ScalarConv_deg2rad(G)) +
		0.020 * sin(proteus_ScalarConv_deg2rad(2.0 * G));

	const double LA_rad = proteus_ScalarConv_deg2rad(LA);
	const double E_rad = proteus_ScalarConv_deg2rad(computeObliquityForJulianCentury(T));

	const double RA_rad = atan2(cos(E_rad) * sin(LA_rad), cos(LA_rad));
	const double DEC_rad = asin(sin(E_rad) * sin(LA_rad));

	ec->ra = fmod(proteus_ScalarConv_rad2deg(RA_rad), 360.0) / 15.0;
	while (ec->ra < 0.0)
	{
		ec->ra += 24.0;
	}

	ec->dec = proteus_ScalarConv_rad2deg(DEC_rad);

	return 0;
}

PROTEUS_API int proteus_Celestial_convertEquatorialToHorizontal(
	double jd,
	const proteus_GeoPos* pos,
	const proteus_CelestialEquatorialCoord* ec,
	bool atmosEffect,
	double airPressure,
	double airTemp,
	proteus_CelestialHorizontalCoord* hc
)
{
	const double N = jd - 2451545.0;
	const double T = N / 36525.0;

	const double ERA_rad = 2.0 * M_PI * (0.7790572732640 + 1.00273781191135448 * N);

	const double E_PREC_sec = -0.0104506
		- 4612.16534 * T
		- 1.3915817 * T * T
		+ 4.4e-7 * T * T * T
		+ 2.9956e-5 * T * T * T * T;

	const double GMST_rad = ERA_rad - (E_PREC_sec * M_PI / 3600.0 / 180.0);

	const double LAT_rad = proteus_ScalarConv_deg2rad(pos->lat);
	const double LON_rad = proteus_ScalarConv_deg2rad(pos->lon);

	const double RA_rad = proteus_ScalarConv_deg2rad(ec->ra * 15.0);
	const double DEC_rad = proteus_ScalarConv_deg2rad(ec->dec);

	const double LMST_rad = GMST_rad + LON_rad;
	const double LHA_rad = LMST_rad - RA_rad;

	const double AZ_Y = sin(LHA_rad);
	const double AZ_X = cos(LHA_rad) * sin(LAT_rad) - tan(DEC_rad) * cos(LAT_rad);
	const double AZ_rad = atan2(AZ_Y, AZ_X);

	const double ALT_rad = asin(sin(LAT_rad) * sin(DEC_rad) + cos(LAT_rad) * cos(DEC_rad) * cos(LHA_rad));

	hc->az = fmod(proteus_ScalarConv_rad2deg(AZ_rad) + 180.0, 360.0);
	hc->alt = proteus_ScalarConv_rad2deg(ALT_rad);


	if (atmosEffect)
	{
		const double tanArg = proteus_ScalarConv_deg2rad(hc->alt + (10.3 / (hc->alt + 5.11)));
		const double refrArcMin = 1.02 *
			(1.0 / tan(tanArg)) *
			(airPressure / 1010.0) *
			(283.0 / (273.0 + airTemp));

		if (refrArcMin > 0.0)
		{
			hc->alt += (refrArcMin / 60.0);
		}
	}


	return 0;
}


static double computeObliquityForJulianCentury(double t)
{
	return (
		84381.406
		- 46.836769 * t
		- 0.0001831 * t * t
		+ 0.00200340 * t * t * t
		- 5.76e-7 * t * t * t * t
		- 4.34e-8 * t * t * t * t * t
	) / 3600.0;
}

static void computeStarEq(int obj, double jd, proteus_CelestialEquatorialCoord* ec)
{
	// Adjust object identifier value for positions in array.
	obj--;

	const double Y = (jd - 2451545.0) / 365.25;
	const double T = Y / 100.0;


	double RA_2000_hr = STAR_EPH_J2000[4 * obj] + (STAR_EPH_J2000[4 * obj + 2] * Y) / (1000.0 * 3600.0 * 15.0);
	double DEC_2000_deg = STAR_EPH_J2000[4 * obj + 1] + (STAR_EPH_J2000[4 * obj + 3] * Y) / (1000.0 * 3600.0);

	while (RA_2000_hr < 0.0) { RA_2000_hr += 24.0; }
	while (RA_2000_hr >= 24.0) { RA_2000_hr -= 24.0; }

	if (DEC_2000_deg < -90.0) { DEC_2000_deg = -90.0; }
	else if (DEC_2000_deg > 90.0) { DEC_2000_deg = 90.0; }


	const double RA_2000_rad = RA_2000_hr * 15.0 * M_PI / 180.0;
	const double DEC_2000_rad = proteus_ScalarConv_deg2rad(DEC_2000_deg);

	const double E_rad = proteus_ScalarConv_deg2rad(computeObliquityForJulianCentury(T));
	const double P_deg = ((5028.796195 * T) + (1.1054348 * T * T)) / 3600.0;

	const double RA_DELTA_hr = (P_deg / 15.0) * (cos(E_rad) + sin(E_rad) * sin(RA_2000_rad) * tan(DEC_2000_rad));
	const double DEC_DELTA_deg = P_deg * cos(RA_2000_rad) * sin(E_rad);


	ec->ra = RA_2000_hr + RA_DELTA_hr;
	ec->dec = DEC_2000_deg + DEC_DELTA_deg;

	while (ec->ra < 0.0) { ec->ra += 24.0; }
	while (ec->ra >= 24.0) { ec->ra -= 24.0; }

	if (ec->dec < -90.0) { ec->dec = -90.0; }
	else if (ec->dec > 90.0) { ec->dec = 90.0; }
}
