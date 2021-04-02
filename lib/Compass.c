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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "proteus_internal.h"

#include "proteus/Compass.h"
#include "ErrLog.h"

#define ERRLOG_ID "proteus_Compass"


#define MAG_GRID_X (360) // -180 to 179 - in 1 degree increments
#define MAG_GRID_Y (181) // -90 to 90 - in 1 degree increments

// Current data for years 2020-2025.
#define MAG_DATA_YEAR_START (2020)
#define MAG_DATA_YEARS (6)
#define MAG_DATA_SEC_AT_START (1577836800)
#define MAG_DATA_SEC_IN_YEAR (31557600)


typedef struct
{
	float dec[MAG_DATA_YEARS]; // Magnetic declination (degrees) for each year
} MagGridPoint;

static MagGridPoint* _magGrid = 0;

static void initMagGrid(const char* magGridDataPath);
static int readMagGridPoint(char* s, float* x, float* y, int* year, float* magDec);

static void insertMagGridPoint(MagGridPoint* magGrid, float lon, float lat, int year, float magDec);

static int getXYIndex(int x, int y);

static double getYearOffsetForTime(time_t t);


PROTEUS_API double proteus_Compass_diff(double a, double b)
{
	double c = b - a;

	if (c < 0.0)
	{
		c += 360.0;
	}

	return ((c > 180.0) ? (c - 360.0) : c);
}

PROTEUS_API int proteus_Compass_init(const char* magDecFile)
{
	if (!magDecFile)
	{
		return -2;
	}

	initMagGrid(magDecFile);

	return ((_magGrid != 0) ? 0 : -1);
}

PROTEUS_API double proteus_Compass_magdec(const proteus_GeoPos* pos, time_t t)
{
	// NOTE: Constants below will require modification if MAG_GRID_X or MAG_GRID_Y values change.
	int ilon = ((int) floor(pos->lon)) + 180;
	int ilat = ((int) floor(pos->lat)) + 90;

	if (ilat < 0 || ilat >= (MAG_GRID_Y - 1))
	{
		return 0.0;
	}

	if (ilon == MAG_GRID_X)
	{
		ilon = 0;
	}

	const MagGridPoint* magGridPtA = _magGrid + getXYIndex(ilon, ilat);
	const MagGridPoint* magGridPtB = _magGrid + getXYIndex(ilon + 1, ilat);
	const MagGridPoint* magGridPtC = _magGrid + getXYIndex(ilon, ilat + 1);
	const MagGridPoint* magGridPtD = _magGrid + getXYIndex(ilon + 1, ilat + 1);

	if (ilon == MAG_GRID_X - 1)
	{
		magGridPtA = _magGrid + getXYIndex(ilon, ilat);
		magGridPtB = _magGrid + getXYIndex(0, ilat);
		magGridPtC = _magGrid + getXYIndex(ilon, ilat + 1);
		magGridPtD = _magGrid + getXYIndex(0, ilat + 1);
	}


	const double xFrac = (ilon == 0 && pos->lon == 180.0) ? 0.0 : pos->lon - ((double) (ilon - 180));
	const double yFrac = pos->lat - ((double) (ilat - 90));


	int t0;
	int t1;
	double tFrac;

	const double y = getYearOffsetForTime(t);
	if (y <= 0.0)
	{
		t0 = 0;
		t1 = 0;

		tFrac = 0.0;
	}
	else if (y >= ((double)(MAG_DATA_YEARS - 1)))
	{
		t0 = MAG_DATA_YEARS - 1;
		t1 = MAG_DATA_YEARS - 1;

		tFrac = 0.0;
	}
	else
	{
		t0 = (int) floor(y);
		t1 = t0 + 1;

		tFrac = y - floor(y);
	}


	const double magDec0_0 = (magGridPtA->dec[t0] * (1.0 - xFrac)) + (magGridPtB->dec[t0] * xFrac);
	const double magDec1_0 = (magGridPtC->dec[t0] * (1.0 - xFrac)) + (magGridPtD->dec[t0] * xFrac);
	const double magDec_0 = (magDec0_0 * (1.0 - yFrac)) + (magDec1_0 * yFrac);

	const double magDec0_1 = (magGridPtA->dec[t1] * (1.0 - xFrac)) + (magGridPtB->dec[t1] * xFrac);
	const double magDec1_1 = (magGridPtC->dec[t1] * (1.0 - xFrac)) + (magGridPtD->dec[t1] * xFrac);
	const double magDec_1 = (magDec0_1 * (1.0 - yFrac)) + (magDec1_1 * yFrac);

	double magDec = (magDec_0 * (1.0 - tFrac)) + (magDec_1 * tFrac);


	while (magDec <= -180.0)
	{
		magDec += 360.0;
	}

	while (magDec > 180.0)
	{
		magDec -= 360.0;
	}


	return magDec;
}


#define MAG_GRID_PARSE_BUF_SIZE (256)

static void initMagGrid(const char* magGridDataPath)
{
	MagGridPoint* magGrid = (MagGridPoint*) malloc(MAG_GRID_X * MAG_GRID_Y * sizeof(MagGridPoint));
	memset(magGrid, 0x00, MAG_GRID_X * MAG_GRID_Y * sizeof(MagGridPoint));

	FILE* fp;
	char buf[MAG_GRID_PARSE_BUF_SIZE];

	float x, y, magDec;
	int year;


	fp = fopen(magGridDataPath, "r");
	if (fp == 0)
	{
		goto fail;
	}

	while (fgets(buf, MAG_GRID_PARSE_BUF_SIZE, fp) == buf)
	{
		if (readMagGridPoint(buf, &x, &y, &year, &magDec) != 0)
		{
			goto fail;
		}

		insertMagGridPoint(magGrid, x, y, year, magDec);
	}
	fclose(fp);


	_magGrid = magGrid;
	ERRLOG1("Initialized mag grid (from %s).", magGridDataPath);

	return;

fail:
	ERRLOG("Failed to init mag grid!");
	free(magGrid);
}

static int readMagGridPoint(char* s, float* x, float* y, int* year, float* magDec)
{
	char* t;
	char* w;

	if ((w = strtok_r(s, ",", &t)) == 0)
	{
		return -1;
	}
	*y = strtof(w, 0);

	if ((w = strtok_r(0, ",", &t)) == 0)
	{
		return -2;
	}
	*x = strtof(w, 0);

	if ((w = strtok_r(0, ",", &t)) == 0)
	{
		return -3;
	}
	*year = strtol(w, 0, 10);

	if ((w = strtok_r(0, ",", &t)) == 0)
	{
		return -4;
	}
	*magDec = strtof(w, 0);

	return 0;
}

static void insertMagGridPoint(MagGridPoint* magGrid, float lon, float lat, int year, float magDec)
{
	year = year - MAG_DATA_YEAR_START;
	if (year < 0 || year >= MAG_DATA_YEARS)
	{
		return;
	}

	if (lon >= 180.0)
	{
		lon -= 360.0;
	}

	int ilon = ((int) roundf(lon)) + 180;
	int ilat = ((int) roundf(lat)) + 90;

	if (ilat < 0 || ilat >= MAG_GRID_Y)
	{
		ERRLOG4("Failed to insert mag grid point at %f,%f (%d, %d).", lon, lat, ilon, ilat);
		return;
	}

	if (ilon == MAG_GRID_X)
	{
		ilon = 0;
	}

	MagGridPoint* p = magGrid + getXYIndex(ilon, ilat);

	p->dec[year] = magDec;
}

static int getXYIndex(int x, int y)
{
	return y * MAG_GRID_X + x;
}

static double getYearOffsetForTime(time_t t)
{
	return ((double)(t - MAG_DATA_SEC_AT_START) / (double)MAG_DATA_SEC_IN_YEAR);
}
