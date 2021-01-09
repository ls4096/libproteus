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
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "proteus_internal.h"

#include "proteus/Ocean.h"
#include "proteus/ScalarConv.h"
#include "Constants.h"
#include "ErrLog.h"

#define ERRLOG_ID "proteus_Ocean"
#define UPDATER_THREAD_NAME "proteus_Ocean"


// NOTE: This module currently makes some fixed assumptions about the grid dimensions
//       and time between forecast points (for blending).

#define OCEAN_GRID_X (900) // 0.0 (-180) to 359.6 (179.6) - in 0.4 degree increments
#define OCEAN_GRID_Y (397) // -78.4 to 80.0 - in 0.4 degree increments

// 11 hours, 58 minutes
#define OCEAN_DATA_PHASE_IN_SECONDS (11 * (60 * 60) + (58 * 60))


typedef struct
{
	float currentU; // m/s
	float currentV; // m/s
	float surfaceTemp; // deg C
	float salinity;

	bool valid;
} OceanGridPoint;

static const char* _f1File = 0;
static const char* _f2File = 0;

static OceanGridPoint* _oceanGrid0 = 0;
static OceanGridPoint* _oceanGrid1 = 0;
static pthread_mutex_t _oceanGridLock;
static time_t _oceanGridPhaseTime = 0;

static pthread_t _oceanUpdaterThread;
static void* oceanUpdaterMain(void* arg);


static void updateOceanGrid(int grid, const char* oceanDataPath);
static int readOceanPoint(char* s, float* x, float* y, float* temp, float* u, float* v, float* salinity);

static void insertOceanGridPoint(OceanGridPoint* oceanGrid, float lon, float lat, float u, float v, float temp, float salinity);

static int getXYIndex(int x, int y);

static void computeOceanDataIce(proteus_OceanData* od);


int proteus_Ocean_init(const char* f1File, const char* f2File)
{
	if (!f1File || !f2File)
	{
		return -3;
	}

	_f1File = strdup(f1File);
	_f2File = strdup(f2File);

	pthread_mutex_init(&_oceanGridLock, 0);

	time_t curTime = time(0);
	struct tm tres;
	if (&tres != gmtime_r(&curTime, &tres))
	{
		return -2;
	}

	const int hour = tres.tm_hour;
	const int min = tres.tm_min;

	if (hour >= 17 || hour < 6)
	{
		// In this situation, it's expected that the "f2" data would be "older" than the "f1" data,
		// so just init both grids to the same data for now.
		updateOceanGrid(0, _f1File);
		updateOceanGrid(1, _f1File);

		// Actual phase time doesn't matter when both grids are identical.
		_oceanGridPhaseTime = curTime;
	}
	else
	{
		updateOceanGrid(0, _f1File);
		updateOceanGrid(1, _f2File);

		// Next phase time at 0600Z + phase interval.
		_oceanGridPhaseTime = curTime - (3600 * hour) - (60 * min) + (3600 * 6) + OCEAN_DATA_PHASE_IN_SECONDS;
	}

	ERRLOG2("Ocean grid phase time: %lu (%ld seconds from now).", _oceanGridPhaseTime, (_oceanGridPhaseTime - curTime));

	if (0 != pthread_create(&_oceanUpdaterThread, 0, &oceanUpdaterMain, 0))
	{
		return -2;
	}

#if defined(_GNU_SOURCE) && defined(__GLIBC__)
	if (0 != pthread_setname_np(_oceanUpdaterThread, UPDATER_THREAD_NAME))
	{
		ERRLOG1("Couldn't set thread name to %s. Continuing anyway.", UPDATER_THREAD_NAME);
	}
#endif

	return ((_oceanGrid0 != 0 && _oceanGrid1 != 0) ? 0 : -1);
}

bool proteus_Ocean_get(const proteus_GeoPos* pos, proteus_OceanData* od)
{
	// NOTE: Constants below will require modification if OCEAN_GRID_X or OCEAN_GRID_Y values change.
	int ilon = ((int) floor(pos->lon * 2.5)) + 450;
	int ilat = ((int) floor(pos->lat * 2.5)) + 196;

	if (ilat < 0 || ilat >= (OCEAN_GRID_Y - 1))
	{
		return false;
	}

	if (ilon == OCEAN_GRID_X)
	{
		ilon = 0;
	}

	bool ret = false;
	pthread_mutex_lock(&_oceanGridLock);

	const OceanGridPoint* oceanGridPtA0 = _oceanGrid0 + getXYIndex(ilon, ilat);
	const OceanGridPoint* oceanGridPtB0 = _oceanGrid0 + getXYIndex(ilon + 1, ilat);
	const OceanGridPoint* oceanGridPtC0 = _oceanGrid0 + getXYIndex(ilon, ilat + 1);
	const OceanGridPoint* oceanGridPtD0 = _oceanGrid0 + getXYIndex(ilon + 1, ilat + 1);

	const OceanGridPoint* oceanGridPtA1 = _oceanGrid1 + getXYIndex(ilon, ilat);
	const OceanGridPoint* oceanGridPtB1 = _oceanGrid1 + getXYIndex(ilon + 1, ilat);
	const OceanGridPoint* oceanGridPtC1 = _oceanGrid1 + getXYIndex(ilon, ilat + 1);
	const OceanGridPoint* oceanGridPtD1 = _oceanGrid1 + getXYIndex(ilon + 1, ilat + 1);

	if (ilon == OCEAN_GRID_X - 1)
	{
		oceanGridPtA0 = _oceanGrid0 + getXYIndex(ilon, ilat);
		oceanGridPtB0 = _oceanGrid0 + getXYIndex(0, ilat);
		oceanGridPtC0 = _oceanGrid0 + getXYIndex(ilon, ilat + 1);
		oceanGridPtD0 = _oceanGrid0 + getXYIndex(0, ilat + 1);

		oceanGridPtA1 = _oceanGrid1 + getXYIndex(ilon, ilat);
		oceanGridPtB1 = _oceanGrid1 + getXYIndex(0, ilat);
		oceanGridPtC1 = _oceanGrid1 + getXYIndex(ilon, ilat + 1);
		oceanGridPtD1 = _oceanGrid1 + getXYIndex(0, ilat + 1);
	}


	uint8_t valid = 0;

	if (oceanGridPtA0->valid)
	{
		valid |= 0x01;
	}

	if (oceanGridPtB0->valid)
	{
		valid |= 0x02;
	}

	if (oceanGridPtC0->valid)
	{
		valid |= 0x04;
	}

	if (oceanGridPtD0->valid)
	{
		valid |= 0x08;
	}

	if (valid == 0)
	{
		goto done;
	}

	int count = 0;
	OceanGridPoint avgPt0;

	if (valid != 0x0f)
	{
		avgPt0.currentU = 0;
		avgPt0.currentV = 0;
		avgPt0.surfaceTemp = 0;
		avgPt0.salinity = 0;

		if ((valid & 0x01) == 0x01)
		{
			avgPt0.currentU += oceanGridPtA0->currentU;
			avgPt0.currentV += oceanGridPtA0->currentV;
			avgPt0.surfaceTemp += oceanGridPtA0->surfaceTemp;
			avgPt0.salinity += oceanGridPtA0->salinity;
			count++;
		}
		else
		{
			oceanGridPtA0 = &avgPt0;
		}

		if ((valid & 0x02) == 0x02)
		{
			avgPt0.currentU += oceanGridPtB0->currentU;
			avgPt0.currentV += oceanGridPtB0->currentV;
			avgPt0.surfaceTemp += oceanGridPtB0->surfaceTemp;
			avgPt0.salinity += oceanGridPtB0->salinity;
			count++;
		}
		else
		{
			oceanGridPtB0 = &avgPt0;
		}

		if ((valid & 0x04) == 0x04)
		{
			avgPt0.currentU += oceanGridPtC0->currentU;
			avgPt0.currentV += oceanGridPtC0->currentV;
			avgPt0.surfaceTemp += oceanGridPtC0->surfaceTemp;
			avgPt0.salinity += oceanGridPtC0->salinity;
			count++;
		}
		else
		{
			oceanGridPtC0 = &avgPt0;
		}

		if ((valid & 0x08) == 0x08)
		{
			avgPt0.currentU += oceanGridPtD0->currentU;
			avgPt0.currentV += oceanGridPtD0->currentV;
			avgPt0.surfaceTemp += oceanGridPtD0->surfaceTemp;
			avgPt0.salinity += oceanGridPtD0->salinity;
			count++;
		}
		else
		{
			oceanGridPtD0 = &avgPt0;
		}

		avgPt0.currentU /= count;
		avgPt0.currentV /= count;
		avgPt0.surfaceTemp /= count;
		avgPt0.salinity /= count;
	}


	valid = 0;

	if (oceanGridPtA1->valid)
	{
		valid |= 0x01;
	}

	if (oceanGridPtB1->valid)
	{
		valid |= 0x02;
	}

	if (oceanGridPtC1->valid)
	{
		valid |= 0x04;
	}

	if (oceanGridPtD1->valid)
	{
		valid |= 0x08;
	}

	if (valid == 0)
	{
		goto done;
	}

	count = 0;
	OceanGridPoint avgPt1;

	if (valid != 0x0f)
	{
		avgPt1.currentU = 0;
		avgPt1.currentV = 0;
		avgPt1.surfaceTemp = 0;
		avgPt1.salinity = 0;

		if ((valid & 0x01) == 0x01)
		{
			avgPt1.currentU += oceanGridPtA1->currentU;
			avgPt1.currentV += oceanGridPtA1->currentV;
			avgPt1.surfaceTemp += oceanGridPtA1->surfaceTemp;
			avgPt1.salinity += oceanGridPtA1->salinity;
			count++;
		}
		else
		{
			oceanGridPtA1 = &avgPt1;
		}

		if ((valid & 0x02) == 0x02)
		{
			avgPt1.currentU += oceanGridPtB1->currentU;
			avgPt1.currentV += oceanGridPtB1->currentV;
			avgPt1.surfaceTemp += oceanGridPtB1->surfaceTemp;
			avgPt1.salinity += oceanGridPtB1->salinity;
			count++;
		}
		else
		{
			oceanGridPtB1 = &avgPt1;
		}

		if ((valid & 0x04) == 0x04)
		{
			avgPt1.currentU += oceanGridPtC1->currentU;
			avgPt1.currentV += oceanGridPtC1->currentV;
			avgPt1.surfaceTemp += oceanGridPtC1->surfaceTemp;
			avgPt1.salinity += oceanGridPtC1->salinity;
			count++;
		}
		else
		{
			oceanGridPtC1 = &avgPt1;
		}

		if ((valid & 0x08) == 0x08)
		{
			avgPt1.currentU += oceanGridPtD1->currentU;
			avgPt1.currentV += oceanGridPtD1->currentV;
			avgPt1.surfaceTemp += oceanGridPtD1->surfaceTemp;
			avgPt1.salinity += oceanGridPtD1->salinity;
			count++;
		}
		else
		{
			oceanGridPtD1 = &avgPt1;
		}

		avgPt1.currentU /= count;
		avgPt1.currentV /= count;
		avgPt1.surfaceTemp /= count;
		avgPt1.salinity /= count;
	}


	const double xFrac = (ilon == 0 && pos->lon == 180.0) ? 0.0 : (pos->lon * 2.5) - ((double) (ilon - 450));
	const double yFrac = (pos->lat * 2.5) - ((double) (ilat - 196));

	const long tDiff = _oceanGridPhaseTime - time(0);
	double tFrac = 1.0 - (((double) tDiff) / ((double) OCEAN_DATA_PHASE_IN_SECONDS));
	if (tFrac < 0.0)
	{
		tFrac = 0.0;
	}
	else if (tFrac > 1.0)
	{
		tFrac = 1.0;
	}


	const double currentU0_0 = (oceanGridPtA0->currentU * (1.0 - xFrac)) + (oceanGridPtB0->currentU * xFrac);
	const double currentU1_0 = (oceanGridPtC0->currentU * (1.0 - xFrac)) + (oceanGridPtD0->currentU * xFrac);
	const double currentV0_0 = (oceanGridPtA0->currentV * (1.0 - xFrac)) + (oceanGridPtB0->currentV * xFrac);
	const double currentV1_0 = (oceanGridPtC0->currentV * (1.0 - xFrac)) + (oceanGridPtD0->currentV * xFrac);

	const double currentU_0 = (currentU0_0 * (1.0 - yFrac)) + (currentU1_0 * yFrac);
	const double currentV_0 = (currentV0_0 * (1.0 - yFrac)) + (currentV1_0 * yFrac);

	const double currentU0_1 = (oceanGridPtA1->currentU * (1.0 - xFrac)) + (oceanGridPtB1->currentU * xFrac);
	const double currentU1_1 = (oceanGridPtC1->currentU * (1.0 - xFrac)) + (oceanGridPtD1->currentU * xFrac);
	const double currentV0_1 = (oceanGridPtA1->currentV * (1.0 - xFrac)) + (oceanGridPtB1->currentV * xFrac);
	const double currentV1_1 = (oceanGridPtC1->currentV * (1.0 - xFrac)) + (oceanGridPtD1->currentV * xFrac);

	const double currentU_1 = (currentU0_1 * (1.0 - yFrac)) + (currentU1_1 * yFrac);
	const double currentV_1 = (currentV0_1 * (1.0 - yFrac)) + (currentV1_1 * yFrac);

	const double currentU = (currentU_0 * (1.0 - tFrac)) + (currentU_1 * tFrac);
	const double currentV = (currentV_0 * (1.0 - tFrac)) + (currentV_1 * tFrac);

	if (fabs(currentV) < EPSILON)
	{
		if (currentU < -EPSILON)
		{
			od->current.angle = 270.0;
		}
		else if (currentU > EPSILON)
		{
			od->current.angle = 90.0;
		}
		else
		{
			od->current.angle = 0.0;
		}
	}
	else
	{
		od->current.angle = proteus_ScalarConv_rad2deg(atan(currentU / currentV));

		if (currentV < 0.0)
		{
			od->current.angle += 180.0;
		}
		else if (currentU < 0.0)
		{
			od->current.angle += 360.0;
		}
	}

	od->current.mag = sqrt((currentU * currentU) + (currentV * currentV));


	const double temp0_0 = (oceanGridPtA0->surfaceTemp * (1.0 - xFrac)) + (oceanGridPtB0->surfaceTemp * xFrac);
	const double temp1_0 = (oceanGridPtC0->surfaceTemp * (1.0 - xFrac)) + (oceanGridPtD0->surfaceTemp * xFrac);
	const double temp_0 = (temp0_0 * (1.0 - yFrac)) + (temp1_0 * yFrac);

	const double temp0_1 = (oceanGridPtA1->surfaceTemp * (1.0 - xFrac)) + (oceanGridPtB1->surfaceTemp * xFrac);
	const double temp1_1 = (oceanGridPtC1->surfaceTemp * (1.0 - xFrac)) + (oceanGridPtD1->surfaceTemp * xFrac);
	const double temp_1 = (temp0_1 * (1.0 - yFrac)) + (temp1_1 * yFrac);

	od->surfaceTemp = (temp_0 * (1.0 - tFrac)) + (temp_1 * tFrac);


	const double salinity0_0 = (oceanGridPtA0->salinity * (1.0 - xFrac)) + (oceanGridPtB0->salinity * xFrac);
	const double salinity1_0 = (oceanGridPtC0->salinity * (1.0 - xFrac)) + (oceanGridPtD0->salinity * xFrac);
	const double salinity_0 = (salinity0_0 * (1.0 - yFrac)) + (salinity1_0 * yFrac);

	const double salinity0_1 = (oceanGridPtA1->salinity * (1.0 - xFrac)) + (oceanGridPtB1->salinity * xFrac);
	const double salinity1_1 = (oceanGridPtC1->salinity * (1.0 - xFrac)) + (oceanGridPtD1->salinity * xFrac);
	const double salinity_1 = (salinity0_1 * (1.0 - yFrac)) + (salinity1_1 * yFrac);

	od->salinity = (salinity_0 * (1.0 - tFrac)) + (salinity_1 * tFrac);


	computeOceanDataIce(od);


	ret = true;

done:
	pthread_mutex_unlock(&_oceanGridLock);
	return ret;
}

#define OCEAN_GRID_PARSE_BUF_SIZE (256)

static void updateOceanGrid(int grid, const char* oceanDataPath)
{
	OceanGridPoint* oceanGrid = (OceanGridPoint*) malloc(OCEAN_GRID_X * OCEAN_GRID_Y * sizeof(OceanGridPoint));

	if (grid == -1)
	{
		// Updating ocean data grids, so copy previous grid to start with sane values (in case new values are unavailable for some reason).
		memcpy(oceanGrid, _oceanGrid1, OCEAN_GRID_X * OCEAN_GRID_Y * sizeof(OceanGridPoint));
	}
	else
	{
		// Setting up ocean data grid for the first time, so initialize the grid to zeros.
		memset(oceanGrid, 0, OCEAN_GRID_X * OCEAN_GRID_Y * sizeof(OceanGridPoint));
	}

	FILE* fp;
	char buf[OCEAN_GRID_PARSE_BUF_SIZE];

	float x, y;
	float u, v, temp, salinity;


	fp = fopen(oceanDataPath, "r");
	if (fp == 0)
	{
		goto fail;
	}

	while (fgets(buf, OCEAN_GRID_PARSE_BUF_SIZE, fp) == buf)
	{
		if (readOceanPoint(buf, &x, &y, &temp, &u, &v, &salinity) != 0)
		{
			goto fail;
		}

		insertOceanGridPoint(oceanGrid, x, y, u, v, temp, salinity);
	}
	fclose(fp);


	if (grid != -1)
	{
		if (grid == 0)
		{
			_oceanGrid0 = oceanGrid;
		}
		else
		{
			_oceanGrid1 = oceanGrid;
		}

		ERRLOG2("Initialized ocean grid %d (from %s).", grid, oceanDataPath);
	}
	else
	{
		pthread_mutex_lock(&_oceanGridLock);

		// Update, so free grid 0 data, grid 0 gets grid 1 data, and grid 1 gets latest data.
		free(_oceanGrid0);
		_oceanGrid0 = _oceanGrid1;
		_oceanGrid1 = oceanGrid;

		_oceanGridPhaseTime = time(0) + OCEAN_DATA_PHASE_IN_SECONDS;

		pthread_mutex_unlock(&_oceanGridLock);

		ERRLOG2("Updated ocean grids (latest from %s). Grid phase time: %lu", oceanDataPath, _oceanGridPhaseTime);
	}

	return;

fail:
	ERRLOG("Failed to update ocean grid!");
	free(oceanGrid);
}

static int readOceanPoint(char* s, float* x, float* y, float* temp, float* u, float* v, float* salinity)
{
	char* t;
	char* w;

	if ((w = strtok_r(s, ",", &t)) == 0)
	{
		return -1;
	}
	*x = strtof(w, 0);

	if ((w = strtok_r(0, ",", &t)) == 0)
	{
		return -2;
	}
	*y = strtof(w, 0);

	if ((w = strtok_r(0, ",", &t)) == 0)
	{
		return -3;
	}
	*temp = strtof(w, 0);

	if ((w = strtok_r(0, ",", &t)) == 0)
	{
		return -4;
	}
	*u = strtof(w, 0);

	if ((w = strtok_r(0, ",", &t)) == 0)
	{
		return -5;
	}
	*v = strtof(w, 0);

	if ((w = strtok_r(0, ",", &t)) == 0)
	{
		return -6;
	}
	*salinity = strtof(w, 0);

	return 0;
}

static void insertOceanGridPoint(OceanGridPoint* oceanGrid, float lon, float lat, float u, float v, float temp, float salinity)
{
	if (lon >= 180.0)
	{
		lon -= 360.0;
	}

	int ilon = ((int) roundf(lon * 2.5f)) + 450;
	int ilat = ((int) roundf(lat * 2.5f)) + 196;

	if (ilat < 0 || ilat >= OCEAN_GRID_Y)
	{
		ERRLOG4("Failed to insert ocean grid point at %f,%f (%d, %d).", lon, lat, ilon, ilat);
		return;
	}

	if (ilon == OCEAN_GRID_X)
	{
		ilon = 0;
	}

	OceanGridPoint* p = oceanGrid + getXYIndex(ilon, ilat);

	p->currentU = u;
	p->currentV = v;
	p->surfaceTemp = temp;
	p->salinity = salinity;

	p->valid = true;
}

static int getXYIndex(int x, int y)
{
	return y * OCEAN_GRID_X + x;
}

static void computeOceanDataIce(proteus_OceanData* od)
{
	if (od->surfaceTemp > 0.0f)
	{
		// Water will never be frozen above 0 degrees C with Earth's atmospheric pressures, so shortcut here.
		od->ice = 0.0f;
		return;
	}

	// Constants below determined somewhat empirically in order to provide a reasonable ice concentration estimation.
	float ice = ((-7500.0f * od->surfaceTemp) / od->salinity) - 300.0f;
	if (ice > 100.0f)
	{
		ice = 100.0f;
	}
	else if (ice < 0.0f)
	{
		ice = 0.0f;
	}

	od->ice = ice;
}


static void* oceanUpdaterMain(void* arg)
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

		// Attempt to update once every twelve hours (at 18Z and 06Z).
		if (hour == 17 || hour == 5)
		{
			update = true;
		}
		else if (update && (hour == 18 || hour == 6))
		{
			const char* oceanDataPath = ((hour == 18) ? _f1File : _f2File);
			updateOceanGrid(-1, oceanDataPath);
			update = false;
		}

		// TODO: Sleep until we need to update the grids. Polling with a short sleep here is lazy and causes pointless frequent wakeups.
		sleep(60);
	}

	return 0;
}
