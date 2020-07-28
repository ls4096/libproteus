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

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "proteus_internal.h"

#include "proteus/Weather.h"
#include "proteus/ScalarConv.h"
#include "Constants.h"
#include "ErrLog.h"


// NOTE: This module currently makes some fixed assumptions about the grid dimensions
//       and time between forecast points (for blending).

#define WX_GRID_X (360)
#define WX_GRID_Y (181)

// 2 hours, 58 minutes
#define WX_DATA_PHASE_IN_SECONDS (2 * (60 * 60) + (58 * 60))

#define ERRLOG_ID "proteus_Weather"


typedef struct
{
	float windU; // m/s
	float windV; // m/s

	float temp; // K
	float dewpoint; // K
	float pressure; // Pa
	float cloud; // %
	float visibility; // m
	float prate; // kg/m^2/s

	uint8_t cond;
} WxGridPoint;

static const char* _f1Dir = 0;
static const char* _f2Dir = 0;

static WxGridPoint* _wxGrid0 = 0;
static WxGridPoint* _wxGrid1 = 0;
static pthread_mutex_t _wxGridLock;
static time_t _wxGridPhaseTime = 0;

static pthread_t _wxUpdaterThread;
static void* wxUpdaterMain(void* arg);


static void updateWxGrid(int grid, const char* wxDataDirPath);
static int readWxPointF(char* s, int* x, int* y, float* f);
static int readWxPointI(char* s, int* x, int* y, int* n);

static void insertWxGridUgrd(WxGridPoint* wxGrid, int lon, int lat, float ugrd);
static void insertWxGridVgrd(WxGridPoint* wxGrid, int lon, int lat, float vgrd);
static void insertWxGridTmp(WxGridPoint* wxGrid, int lon, int lat, float tmp);
static void insertWxGridDpt(WxGridPoint* wxGrid, int lon, int lat, float dpt);
static void insertWxGridPres(WxGridPoint* wxGrid, int lon, int lat, float pres);
static void insertWxGridCld(WxGridPoint* wxGrid, int lon, int lat, float cld);
static void insertWxGridVis(WxGridPoint* wxGrid, int lon, int lat, float vis);
static void insertWxGridPrate(WxGridPoint* wxGrid, int lon, int lat, float prate);
static void insertWxGridRain(WxGridPoint* wxGrid, int lon, int lat, int rain);
static void insertWxGridSnow(WxGridPoint* wxGrid, int lon, int lat, int snow);
static void insertWxGridIcep(WxGridPoint* wxGrid, int lon, int lat, int icep);
static void insertWxGridFrzr(WxGridPoint* wxGrid, int lon, int lat, int frzr);

static int getLonLatIndex(int lon, int lat);


PROTEUS_API int proteus_Weather_init(const char* f1Dir, const char* f2Dir)
{
	if (!f1Dir || !f2Dir)
	{
		return -3;
	}

	_f1Dir = strdup(f1Dir);
	_f2Dir = strdup(f2Dir);

	pthread_mutex_init(&_wxGridLock, 0);

	time_t curTime = time(0);
	struct tm tres;
	if (&tres != gmtime_r(&curTime, &tres))
	{
		return -2;
	}

	const int hour = tres.tm_hour;
	const int min = tres.tm_min;

	if (((hour + 2) % 6) < 3)
	{
		// In this situation, it's expected that the "f2" data would be "older" than the "f1" data,
		// so just init both grids to the same data for now.
		updateWxGrid(0, _f1Dir);
		updateWxGrid(1, _f1Dir);

		// Actual phase time doesn't matter when both grids are identical.
		_wxGridPhaseTime = curTime;
	}
	else
	{
		updateWxGrid(0, _f1Dir);
		updateWxGrid(1, _f2Dir);

		// Next phase time at {0115Z, 0715Z, 1315Z, 1915Z} + WX_DATA_PHASE_IN_SECONDS.
		_wxGridPhaseTime = curTime - (3600 * ((hour - 1) % 3)) - (60 * min) + (60 * 15) + WX_DATA_PHASE_IN_SECONDS;
	}

	ERRLOG2("Weather grid phase time: %lu (%ld seconds from now).", _wxGridPhaseTime, (_wxGridPhaseTime - curTime));

	if (0 != pthread_create(&_wxUpdaterThread, 0, &wxUpdaterMain, 0))
	{
		return -2;
	}

	return ((_wxGrid0 != 0 && _wxGrid1 != 0) ? 0 : -1);
}

PROTEUS_API void proteus_Weather_get(const proteus_GeoPos* pos, proteus_Weather* wx, bool windOnly)
{
	const int ilon = (int) floor(pos->lon);
	const int ilat = (int) floor(pos->lat);

	pthread_mutex_lock(&_wxGridLock);

	const WxGridPoint* wxGridPtA0 = _wxGrid0 + getLonLatIndex(ilon, ilat);
	const WxGridPoint* wxGridPtB0 = _wxGrid0 + getLonLatIndex(ilon + 1, ilat);
	const WxGridPoint* wxGridPtC0 = _wxGrid0 + getLonLatIndex(ilon, ilat + 1);
	const WxGridPoint* wxGridPtD0 = _wxGrid0 + getLonLatIndex(ilon + 1, ilat + 1);

	const WxGridPoint* wxGridPtA1 = _wxGrid1 + getLonLatIndex(ilon, ilat);
	const WxGridPoint* wxGridPtB1 = _wxGrid1 + getLonLatIndex(ilon + 1, ilat);
	const WxGridPoint* wxGridPtC1 = _wxGrid1 + getLonLatIndex(ilon, ilat + 1);
	const WxGridPoint* wxGridPtD1 = _wxGrid1 + getLonLatIndex(ilon + 1, ilat + 1);

	if (ilon == -180 || ilon == 180)
	{
		wxGridPtA0 = _wxGrid0 + getLonLatIndex(180, ilat);
		wxGridPtB0 = _wxGrid0 + getLonLatIndex(-179, ilat);
		wxGridPtC0 = _wxGrid0 + getLonLatIndex(180, ilat + 1);
		wxGridPtD0 = _wxGrid0 + getLonLatIndex(-179, ilat + 1);

		wxGridPtA1 = _wxGrid1 + getLonLatIndex(180, ilat);
		wxGridPtB1 = _wxGrid1 + getLonLatIndex(-179, ilat);
		wxGridPtC1 = _wxGrid1 + getLonLatIndex(180, ilat + 1);
		wxGridPtD1 = _wxGrid1 + getLonLatIndex(-179, ilat + 1);
	}

	if (ilat == 90)
	{
		wxGridPtC0 = wxGridPtA0;
		wxGridPtD0 = wxGridPtB0;

		wxGridPtC1 = wxGridPtA1;
		wxGridPtD1 = wxGridPtB1;
	}

	const double xFrac = pos->lon - ((double) ilon);
	const double yFrac = pos->lat - ((double) ilat);

	const long tDiff = _wxGridPhaseTime - time(0);
	double tFrac = 1.0 - (((double) tDiff) / ((double) WX_DATA_PHASE_IN_SECONDS));
	if (tFrac < 0.0)
	{
		tFrac = 0.0;
	}
	else if (tFrac > 1.0)
	{
		tFrac = 1.0;
	}


	const double windU0_0 = (wxGridPtA0->windU * (1.0 - xFrac)) + (wxGridPtB0->windU * xFrac);
	const double windU1_0 = (wxGridPtC0->windU * (1.0 - xFrac)) + (wxGridPtD0->windU * xFrac);
	const double windV0_0 = (wxGridPtA0->windV * (1.0 - xFrac)) + (wxGridPtB0->windV * xFrac);
	const double windV1_0 = (wxGridPtC0->windV * (1.0 - xFrac)) + (wxGridPtD0->windV * xFrac);

	const double windU_0 = -1.0 * ((windU0_0 * (1.0 - yFrac)) + (windU1_0 * yFrac));
	const double windV_0 = -1.0 * ((windV0_0 * (1.0 - yFrac)) + (windV1_0 * yFrac));

	const double windU0_1 = (wxGridPtA1->windU * (1.0 - xFrac)) + (wxGridPtB1->windU * xFrac);
	const double windU1_1 = (wxGridPtC1->windU * (1.0 - xFrac)) + (wxGridPtD1->windU * xFrac);
	const double windV0_1 = (wxGridPtA1->windV * (1.0 - xFrac)) + (wxGridPtB1->windV * xFrac);
	const double windV1_1 = (wxGridPtC1->windV * (1.0 - xFrac)) + (wxGridPtD1->windV * xFrac);

	const double windU_1 = -1.0 * ((windU0_1 * (1.0 - yFrac)) + (windU1_1 * yFrac));
	const double windV_1 = -1.0 * ((windV0_1 * (1.0 - yFrac)) + (windV1_1 * yFrac));

	const double windU = (windU_0 * (1.0 - tFrac)) + (windU_1 * tFrac);
	const double windV = (windV_0 * (1.0 - tFrac)) + (windV_1 * tFrac);

	if (fabs(windV) < EPSILON)
	{
		if (windU < -EPSILON)
		{
			wx->wind.angle = 270.0;
		}
		else if (windU > EPSILON)
		{
			wx->wind.angle = 90.0;
		}
		else
		{
			wx->wind.angle = 0.0;
		}
	}
	else
	{
		wx->wind.angle = proteus_ScalarConv_rad2deg(atan(windU / windV));

		if (windV < 0.0)
		{
			wx->wind.angle += 180.0;
		}
		else if (windU < 0.0)
		{
			wx->wind.angle += 360.0;
		}
	}

	wx->wind.mag = sqrt((windU * windU) + (windV * windV));

	if (windOnly)
	{
		goto done;
	}


	const double temp0_0 = (wxGridPtA0->temp * (1.0 - xFrac)) + (wxGridPtB0->temp * xFrac);
	const double temp1_0 = (wxGridPtC0->temp * (1.0 - xFrac)) + (wxGridPtD0->temp * xFrac);
	const double temp_0 = (temp0_0 * (1.0 - yFrac)) + (temp1_0 * yFrac);

	const double temp0_1 = (wxGridPtA1->temp * (1.0 - xFrac)) + (wxGridPtB1->temp * xFrac);
	const double temp1_1 = (wxGridPtC1->temp * (1.0 - xFrac)) + (wxGridPtD1->temp * xFrac);
	const double temp_1 = (temp0_1 * (1.0 - yFrac)) + (temp1_1 * yFrac);

	wx->temp = (temp_0 * (1.0 - tFrac)) + (temp_1 * tFrac) - 273.15;


	const double dewpoint0_0 = (wxGridPtA0->dewpoint * (1.0 - xFrac)) + (wxGridPtB0->dewpoint * xFrac);
	const double dewpoint1_0 = (wxGridPtC0->dewpoint * (1.0 - xFrac)) + (wxGridPtD0->dewpoint * xFrac);
	const double dewpoint_0 = (dewpoint0_0 * (1.0 - yFrac)) + (dewpoint1_0 * yFrac);

	const double dewpoint0_1 = (wxGridPtA1->dewpoint * (1.0 - xFrac)) + (wxGridPtB1->dewpoint * xFrac);
	const double dewpoint1_1 = (wxGridPtC1->dewpoint * (1.0 - xFrac)) + (wxGridPtD1->dewpoint * xFrac);
	const double dewpoint_1 = (dewpoint0_1 * (1.0 - yFrac)) + (dewpoint1_1 * yFrac);

	wx->dewpoint = (dewpoint_0 * (1.0 - tFrac)) + (dewpoint_1 * tFrac) - 273.15;


	const double pressure0_0 = (wxGridPtA0->pressure * (1.0 - xFrac)) + (wxGridPtB0->pressure * xFrac);
	const double pressure1_0 = (wxGridPtC0->pressure * (1.0 - xFrac)) + (wxGridPtD0->pressure * xFrac);
	const double pressure_0 = (pressure0_0 * (1.0 - yFrac)) + (pressure1_0 * yFrac);

	const double pressure0_1 = (wxGridPtA1->pressure * (1.0 - xFrac)) + (wxGridPtB1->pressure * xFrac);
	const double pressure1_1 = (wxGridPtC1->pressure * (1.0 - xFrac)) + (wxGridPtD1->pressure * xFrac);
	const double pressure_1 = (pressure0_1 * (1.0 - yFrac)) + (pressure1_1 * yFrac);

	wx->pressure = ((pressure_0 * (1.0 - tFrac)) + (pressure_1 * tFrac)) / 100.0;


	const double cloud0_0 = (wxGridPtA0->cloud * (1.0 - xFrac)) + (wxGridPtB0->cloud * xFrac);
	const double cloud1_0 = (wxGridPtC0->cloud * (1.0 - xFrac)) + (wxGridPtD0->cloud * xFrac);
	const double cloud_0 = (cloud0_0 * (1.0 - yFrac)) + (cloud1_0 * yFrac);

	const double cloud0_1 = (wxGridPtA1->cloud * (1.0 - xFrac)) + (wxGridPtB1->cloud * xFrac);
	const double cloud1_1 = (wxGridPtC1->cloud * (1.0 - xFrac)) + (wxGridPtD1->cloud * xFrac);
	const double cloud_1 = (cloud0_1 * (1.0 - yFrac)) + (cloud1_1 * yFrac);

	wx->cloud = (cloud_0 * (1.0 - tFrac)) + (cloud_1 * tFrac);


	const double visibility0_0 = (wxGridPtA0->visibility * (1.0 - xFrac)) + (wxGridPtB0->visibility * xFrac);
	const double visibility1_0 = (wxGridPtC0->visibility * (1.0 - xFrac)) + (wxGridPtD0->visibility * xFrac);
	const double visibility_0 = (visibility0_0 * (1.0 - yFrac)) + (visibility1_0 * yFrac);

	const double visibility0_1 = (wxGridPtA1->visibility * (1.0 - xFrac)) + (wxGridPtB1->visibility * xFrac);
	const double visibility1_1 = (wxGridPtC1->visibility * (1.0 - xFrac)) + (wxGridPtD1->visibility * xFrac);
	const double visibility_1 = (visibility0_1 * (1.0 - yFrac)) + (visibility1_1 * yFrac);

	wx->visibility = (visibility_0 * (1.0 - tFrac)) + (visibility_1 * tFrac);


	const double prate0_0 = (wxGridPtA0->prate * (1.0 - xFrac)) + (wxGridPtB0->prate * xFrac);
	const double prate1_0 = (wxGridPtC0->prate * (1.0 - xFrac)) + (wxGridPtD0->prate * xFrac);
	const double prate_0 = (prate0_0 * (1.0 - yFrac)) + (prate1_0 * yFrac);

	const double prate0_1 = (wxGridPtA1->prate * (1.0 - xFrac)) + (wxGridPtB1->prate * xFrac);
	const double prate1_1 = (wxGridPtC1->prate * (1.0 - xFrac)) + (wxGridPtD1->prate * xFrac);
	const double prate_1 = (prate0_1 * (1.0 - yFrac)) + (prate1_1 * yFrac);

	wx->prate = ((prate_0 * (1.0 - tFrac)) + (prate_1 * tFrac)) * 3600.0;


	if (xFrac < 0.5 && yFrac < 0.5)
	{
		wx->cond = (tFrac < 0.5) ? wxGridPtA0->cond : wxGridPtA1->cond;
	}
	else if (xFrac > 0.5 && yFrac < 0.5)
	{
		wx->cond = (tFrac < 0.5) ? wxGridPtB0->cond : wxGridPtB1->cond;
	}
	else if (xFrac < 0.5 && yFrac > 0.5)
	{
		wx->cond = (tFrac < 0.5) ? wxGridPtC0->cond : wxGridPtC1->cond;
	}
	else
	{
		wx->cond = (tFrac < 0.5) ? wxGridPtD0->cond : wxGridPtD1->cond;
	}


done:
	pthread_mutex_unlock(&_wxGridLock);
}


static void updateWxGrid(int grid, const char* wxDataDirPath)
{
	WxGridPoint* wxGrid = (WxGridPoint*) malloc(WX_GRID_X * WX_GRID_Y * sizeof(WxGridPoint));

	if (grid == -1)
	{
		// Updating weather grids, so copy previous grid to start with sane values (in case new values are unavailable for some reason).
		memcpy(wxGrid, _wxGrid1, WX_GRID_X * WX_GRID_Y * sizeof(WxGridPoint));
	}
	else
	{
		// Setting up weather grid for the first time, so initialize the grid to zeros.
		memset(wxGrid, 0, WX_GRID_X * WX_GRID_Y * sizeof(WxGridPoint));
	}

	FILE* fp;
	char buf[256];
	char filePath[1024];

	int x, y, n;
	float f;


	// ugrd
	sprintf(filePath, "%s/%s", wxDataDirPath, "ugrd.csv");
	fp = fopen(filePath, "r");
	if (fp == 0)
	{
		goto fail;
	}

	while (fgets(buf, 256, fp) == buf)
	{
		if (readWxPointF(buf, &x, &y, &f) != 0)
		{
			goto fail;
		}

		insertWxGridUgrd(wxGrid, x, y, f);
	}
	fclose(fp);

	// vgrd
	sprintf(filePath, "%s/%s", wxDataDirPath, "vgrd.csv");
	fp = fopen(filePath, "r");
	if (fp == 0)
	{
		goto fail;
	}

	while (fgets(buf, 256, fp) == buf)
	{
		if (readWxPointF(buf, &x, &y, &f) != 0)
		{
			goto fail;
		}

		insertWxGridVgrd(wxGrid, x, y, f);
	}
	fclose(fp);

	// tmp
	sprintf(filePath, "%s/%s", wxDataDirPath, "tmp.csv");
	fp = fopen(filePath, "r");
	if (fp == 0)
	{
		goto fail;
	}

	while (fgets(buf, 256, fp) == buf)
	{
		if (readWxPointF(buf, &x, &y, &f) != 0)
		{
			goto fail;
		}

		insertWxGridTmp(wxGrid, x, y, f);
	}
	fclose(fp);

	// dpt
	sprintf(filePath, "%s/%s", wxDataDirPath, "dpt.csv");
	fp = fopen(filePath, "r");
	if (fp == 0)
	{
		goto fail;
	}

	while (fgets(buf, 256, fp) == buf)
	{
		if (readWxPointF(buf, &x, &y, &f) != 0)
		{
			goto fail;
		}

		insertWxGridDpt(wxGrid, x, y, f);
	}
	fclose(fp);

	// pres
	sprintf(filePath, "%s/%s", wxDataDirPath, "pres.csv");
	fp = fopen(filePath, "r");
	if (fp == 0)
	{
		goto fail;
	}

	while (fgets(buf, 256, fp) == buf)
	{
		if (readWxPointF(buf, &x, &y, &f) != 0)
		{
			goto fail;
		}

		insertWxGridPres(wxGrid, x, y, f);
	}
	fclose(fp);

	// cld
	sprintf(filePath, "%s/%s", wxDataDirPath, "cld.csv");
	fp = fopen(filePath, "r");
	if (fp == 0)
	{
		goto fail;
	}

	while (fgets(buf, 256, fp) == buf)
	{
		if (readWxPointI(buf, &x, &y, &n) != 0)
		{
			goto fail;
		}

		insertWxGridCld(wxGrid, x, y, (float)n);
	}
	fclose(fp);

	// vis
	sprintf(filePath, "%s/%s", wxDataDirPath, "vis.csv");
	fp = fopen(filePath, "r");
	if (fp == 0)
	{
		goto fail;
	}

	while (fgets(buf, 256, fp) == buf)
	{
		if (readWxPointF(buf, &x, &y, &f) != 0)
		{
			goto fail;
		}

		insertWxGridVis(wxGrid, x, y, f);
	}
	fclose(fp);

	// prate
	sprintf(filePath, "%s/%s", wxDataDirPath, "prate.csv");
	fp = fopen(filePath, "r");
	if (fp == 0)
	{
		goto fail;
	}

	while (fgets(buf, 256, fp) == buf)
	{
		if (readWxPointF(buf, &x, &y, &f) != 0)
		{
			goto fail;
		}

		insertWxGridPrate(wxGrid, x, y, f);
	}
	fclose(fp);

	// rain
	sprintf(filePath, "%s/%s", wxDataDirPath, "rain.csv");
	fp = fopen(filePath, "r");
	if (fp == 0)
	{
		goto fail;
	}

	while (fgets(buf, 256, fp) == buf)
	{
		if (readWxPointI(buf, &x, &y, &n) != 0)
		{
			goto fail;
		}

		insertWxGridRain(wxGrid, x, y, n);
	}
	fclose(fp);

	// snow
	sprintf(filePath, "%s/%s", wxDataDirPath, "snow.csv");
	fp = fopen(filePath, "r");
	if (fp == 0)
	{
		goto fail;
	}

	while (fgets(buf, 256, fp) == buf)
	{
		if (readWxPointI(buf, &x, &y, &n) != 0)
		{
			goto fail;
		}

		insertWxGridSnow(wxGrid, x, y, n);
	}
	fclose(fp);

	// icep
	sprintf(filePath, "%s/%s", wxDataDirPath, "icep.csv");
	fp = fopen(filePath, "r");
	if (fp == 0)
	{
		goto fail;
	}

	while (fgets(buf, 256, fp) == buf)
	{
		if (readWxPointI(buf, &x, &y, &n) != 0)
		{
			goto fail;
		}

		insertWxGridIcep(wxGrid, x, y, n);
	}
	fclose(fp);

	// frzr
	sprintf(filePath, "%s/%s", wxDataDirPath, "frzr.csv");
	fp = fopen(filePath, "r");
	if (fp == 0)
	{
		goto fail;
	}

	while (fgets(buf, 256, fp) == buf)
	{
		if (readWxPointI(buf, &x, &y, &n) != 0)
		{
			goto fail;
		}

		insertWxGridFrzr(wxGrid, x, y, n);
	}
	fclose(fp);


	if (grid != -1)
	{
		if (grid == 0)
		{
			_wxGrid0 = wxGrid;
		}
		else
		{
			_wxGrid1 = wxGrid;
		}

		ERRLOG2("Initialized weather grid %d (from %s).", grid, wxDataDirPath);
	}
	else
	{
		pthread_mutex_lock(&_wxGridLock);

		// Update, so free grid 0 data, grid 0 gets grid 1 data, and grid 1 gets latest data.
		free(_wxGrid0);
		_wxGrid0 = _wxGrid1;
		_wxGrid1 = wxGrid;

		_wxGridPhaseTime = time(0) + WX_DATA_PHASE_IN_SECONDS;

		pthread_mutex_unlock(&_wxGridLock);

		ERRLOG2("Updated weather grids (latest from %s). Grid phase time: %lu", wxDataDirPath, _wxGridPhaseTime);
	}

	return;

fail:
	ERRLOG("Failed to update WX grid!");
	free(wxGrid);
}

static int readWxPointF(char* s, int* x, int* y, float* f)
{
	char* t;
	char* u;

	if ((u = strtok_r(s, ",", &t)) == 0)
	{
		return -1;
	}
	*x = strtol(u, 0, 10);

	if ((u = strtok_r(0, ",", &t)) == 0)
	{
		return -2;
	}
	*y = strtol(u, 0, 10);

	if ((u = strtok_r(0, ",", &t)) == 0)
	{
		return -3;
	}
	*f = strtof(u, 0);

	return 0;
}

static int readWxPointI(char* s, int* x, int* y, int* n)
{
	char* t;
	char* u;

	if ((u = strtok_r(s, ",", &t)) == 0)
	{
		return -1;
	}
	*x = strtol(u, 0, 10);

	if ((u = strtok_r(0, ",", &t)) == 0)
	{
		return -2;
	}
	*y = strtol(u, 0, 10);

	if ((u = strtok_r(0, ",", &t)) == 0)
	{
		return -3;
	}
	*n = strtol(u, 0, 10);

	return 0;
}

static void insertWxGridUgrd(WxGridPoint* wxGrid, int lon, int lat, float ugrd)
{
	wxGrid[getLonLatIndex(lon, lat)].windU = ugrd;
}

static void insertWxGridVgrd(WxGridPoint* wxGrid, int lon, int lat, float vgrd)
{
	wxGrid[getLonLatIndex(lon, lat)].windV = vgrd;
}

static void insertWxGridTmp(WxGridPoint* wxGrid, int lon, int lat, float tmp)
{
	wxGrid[getLonLatIndex(lon, lat)].temp = tmp;
}

static void insertWxGridDpt(WxGridPoint* wxGrid, int lon, int lat, float dpt)
{
	wxGrid[getLonLatIndex(lon, lat)].dewpoint = dpt;
}

static void insertWxGridPres(WxGridPoint* wxGrid, int lon, int lat, float pres)
{
	wxGrid[getLonLatIndex(lon, lat)].pressure = pres;
}

static void insertWxGridCld(WxGridPoint* wxGrid, int lon, int lat, float cld)
{
	wxGrid[getLonLatIndex(lon, lat)].cloud = cld;
}

static void insertWxGridVis(WxGridPoint* wxGrid, int lon, int lat, float vis)
{
	wxGrid[getLonLatIndex(lon, lat)].visibility = vis;
}

static void insertWxGridPrate(WxGridPoint* wxGrid, int lon, int lat, float prate)
{
	wxGrid[getLonLatIndex(lon, lat)].prate = prate;
}

static void insertWxGridRain(WxGridPoint* wxGrid, int lon, int lat, int rain)
{
	if (rain == 1)
	{
		wxGrid[getLonLatIndex(lon, lat)].cond |= PROTEUS_WX_COND_RAIN;
	}
}

static void insertWxGridSnow(WxGridPoint* wxGrid, int lon, int lat, int snow)
{
	if (snow == 1)
	{
		wxGrid[getLonLatIndex(lon, lat)].cond |= PROTEUS_WX_COND_SNOW;
	}
}

static void insertWxGridIcep(WxGridPoint* wxGrid, int lon, int lat, int icep)
{
	if (icep == 1)
	{
		wxGrid[getLonLatIndex(lon, lat)].cond |= PROTEUS_WX_COND_ICEP;
	}
}

static void insertWxGridFrzr(WxGridPoint* wxGrid, int lon, int lat, int frzr)
{
	if (frzr == 1)
	{
		wxGrid[getLonLatIndex(lon, lat)].cond |= PROTEUS_WX_COND_FRZR;
	}
}

static int getLonLatIndex(int lon, int lat)
{
	// NOTE: Constants here also need updating if WX_GRID_X or WX_GRID_Y ever change.
	return (((lat + 90) * WX_GRID_X) + (lon + 179));
}


static void* wxUpdaterMain(void* arg)
{
	bool update = false;

	for (;;)
	{
		time_t curTime = time(0);
		struct tm tres;
		if (&tres != gmtime_r(&curTime, &tres))
		{
			ERRLOG("gmtime_r failed!");
			sleep(5);

			continue;
		}

		const int hour = tres.tm_hour;
		const int min = tres.tm_min;

		// Attempt to update once every three hours (at 01Z, 04Z, 07Z, 10Z, 13Z, 16Z, 19Z and 22Z), at 15 minutes past the hour.
		if ((hour % 3 == 1) && (min < 15))
		{
			update = true;
		}
		else if (min >= 15 && update)
		{
			const char* wxDataDir = ((hour % 6 == 4) ? _f1Dir : _f2Dir);
			updateWxGrid(-1, wxDataDir);
			update = false;
		}

		// TODO: Sleep until we need to update the grids. Polling with a short sleep here is lazy and causes pointless frequent wakeups.
		sleep(60);
	}

	return 0;
}
