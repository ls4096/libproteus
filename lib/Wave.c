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

#include "proteus/Wave.h"
#include "proteus/ScalarConv.h"
#include "Constants.h"
#include "ErrLog.h"

#define ERRLOG_ID "proteus_Wave"
#define UPDATER_THREAD_NAME "proteus_Wave"


// NOTE: This module currently makes some fixed assumptions about the grid dimensions
//       and time between forecast points (for blending).

#define WAVE_GRID_X (360) // -180 to 179 - in 1 degree increments
#define WAVE_GRID_Y (181) // -90 to 90 - in 1 degree increments

// 11 hours, 58 minutes
#define WAVE_DATA_PHASE_IN_SECONDS (11 * (60 * 60) + (58 * 60))


typedef struct
{
	float waveHeight; // m
} WaveGridPoint;

static const char* _f1File = 0;
static const char* _f2File = 0;

static WaveGridPoint* _waveGrid0 = 0;
static WaveGridPoint* _waveGrid1 = 0;
static pthread_rwlock_t _waveGridLock;
static time_t _waveGridPhaseTime = 0;

static pthread_t _waveUpdaterThread;
static void* waveUpdaterMain(void* arg);


static void updateWaveGrid(int grid, const char* waveDataPath);
static int readWavePoint(char* s, float* x, float* y, float* waveHeight);

static void insertWaveGridPoint(WaveGridPoint* waveGrid, float lon, float lat, float waveHeight);

static int getXYIndex(int x, int y);


PROTEUS_API int proteus_Wave_init(const char* f1File, const char* f2File)
{
	if (!f1File || !f2File)
	{
		return -3;
	}

	_f1File = strdup(f1File);
	_f2File = strdup(f2File);

	if (0 != pthread_rwlock_init(&_waveGridLock, 0))
	{
		ERRLOG("Failed to init rwlock!");
		return -4;
	}

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
		updateWaveGrid(0, _f1File);
		updateWaveGrid(1, _f1File);

		// Actual phase time doesn't matter when both grids are identical.
		_waveGridPhaseTime = curTime;
	}
	else
	{
		updateWaveGrid(0, _f1File);
		updateWaveGrid(1, _f2File);

		// Next phase time at 0600Z + phase interval.
		_waveGridPhaseTime = curTime - (3600 * hour) - (60 * min) + (3600 * 6) + WAVE_DATA_PHASE_IN_SECONDS;
	}

	ERRLOG2("Wave grid phase time: %lu (%ld seconds from now).", _waveGridPhaseTime, (_waveGridPhaseTime - curTime));

	if (0 != pthread_create(&_waveUpdaterThread, 0, &waveUpdaterMain, 0))
	{
		return -2;
	}

#if defined(_GNU_SOURCE) && defined(__GLIBC__)
	if (0 != pthread_setname_np(_waveUpdaterThread, UPDATER_THREAD_NAME))
	{
		ERRLOG1("Couldn't set thread name to %s. Continuing anyway.", UPDATER_THREAD_NAME);
	}
#endif

	return ((_waveGrid0 != 0 && _waveGrid1 != 0) ? 0 : -1);
}

PROTEUS_API bool proteus_Wave_get(const proteus_GeoPos* pos, proteus_WaveData* wd)
{
	// NOTE: Constants below will require modification if WAVE_GRID_X or WAVE_GRID_Y values change.
	int ilon = ((int) floor(pos->lon)) + 180;
	int ilat = ((int) floor(pos->lat)) + 90;

	if (ilat < 0 || ilat >= (WAVE_GRID_Y - 1))
	{
		return false;
	}

	if (ilon == WAVE_GRID_X)
	{
		ilon = 0;
	}

	bool ret = false;
	if (0 != pthread_rwlock_rdlock(&_waveGridLock))
	{
		ERRLOG("get: Failed to lock for read!");
		return false;
	}

	const WaveGridPoint* waveGridPtA0 = _waveGrid0 + getXYIndex(ilon, ilat);
	const WaveGridPoint* waveGridPtB0 = _waveGrid0 + getXYIndex(ilon + 1, ilat);
	const WaveGridPoint* waveGridPtC0 = _waveGrid0 + getXYIndex(ilon, ilat + 1);
	const WaveGridPoint* waveGridPtD0 = _waveGrid0 + getXYIndex(ilon + 1, ilat + 1);

	const WaveGridPoint* waveGridPtA1 = _waveGrid1 + getXYIndex(ilon, ilat);
	const WaveGridPoint* waveGridPtB1 = _waveGrid1 + getXYIndex(ilon + 1, ilat);
	const WaveGridPoint* waveGridPtC1 = _waveGrid1 + getXYIndex(ilon, ilat + 1);
	const WaveGridPoint* waveGridPtD1 = _waveGrid1 + getXYIndex(ilon + 1, ilat + 1);

	if (ilon == WAVE_GRID_X - 1)
	{
		waveGridPtA0 = _waveGrid0 + getXYIndex(ilon, ilat);
		waveGridPtB0 = _waveGrid0 + getXYIndex(0, ilat);
		waveGridPtC0 = _waveGrid0 + getXYIndex(ilon, ilat + 1);
		waveGridPtD0 = _waveGrid0 + getXYIndex(0, ilat + 1);

		waveGridPtA1 = _waveGrid1 + getXYIndex(ilon, ilat);
		waveGridPtB1 = _waveGrid1 + getXYIndex(0, ilat);
		waveGridPtC1 = _waveGrid1 + getXYIndex(ilon, ilat + 1);
		waveGridPtD1 = _waveGrid1 + getXYIndex(0, ilat + 1);
	}


	uint8_t valid = 0;

	if (waveGridPtA0->waveHeight >= 0.0f)
	{
		valid |= 0x01;
	}

	if (waveGridPtB0->waveHeight >= 0.0f)
	{
		valid |= 0x02;
	}

	if (waveGridPtC0->waveHeight >= 0.0f)
	{
		valid |= 0x04;
	}

	if (waveGridPtD0->waveHeight >= 0.0f)
	{
		valid |= 0x08;
	}

	if (valid == 0)
	{
		goto done;
	}

	int count = 0;
	WaveGridPoint avgPt0;

	if (valid != 0x0f)
	{
		avgPt0.waveHeight = 0;

		if ((valid & 0x01) == 0x01)
		{
			avgPt0.waveHeight += waveGridPtA0->waveHeight;
			count++;
		}
		else
		{
			waveGridPtA0 = &avgPt0;
		}

		if ((valid & 0x02) == 0x02)
		{
			avgPt0.waveHeight += waveGridPtB0->waveHeight;
			count++;
		}
		else
		{
			waveGridPtB0 = &avgPt0;
		}

		if ((valid & 0x04) == 0x04)
		{
			avgPt0.waveHeight += waveGridPtC0->waveHeight;
			count++;
		}
		else
		{
			waveGridPtC0 = &avgPt0;
		}

		if ((valid & 0x08) == 0x08)
		{
			avgPt0.waveHeight += waveGridPtD0->waveHeight;
			count++;
		}
		else
		{
			waveGridPtD0 = &avgPt0;
		}

		avgPt0.waveHeight /= count;
	}


	valid = 0;

	if (waveGridPtA1->waveHeight >= 0.0f)
	{
		valid |= 0x01;
	}

	if (waveGridPtB1->waveHeight >= 0.0f)
	{
		valid |= 0x02;
	}

	if (waveGridPtC1->waveHeight >= 0.0f)
	{
		valid |= 0x04;
	}

	if (waveGridPtD1->waveHeight >= 0.0f)
	{
		valid |= 0x08;
	}

	if (valid == 0)
	{
		goto done;
	}

	count = 0;
	WaveGridPoint avgPt1;

	if (valid != 0x0f)
	{
		avgPt1.waveHeight = 0;

		if ((valid & 0x01) == 0x01)
		{
			avgPt1.waveHeight += waveGridPtA1->waveHeight;
			count++;
		}
		else
		{
			waveGridPtA1 = &avgPt1;
		}

		if ((valid & 0x02) == 0x02)
		{
			avgPt1.waveHeight += waveGridPtB1->waveHeight;
			count++;
		}
		else
		{
			waveGridPtB1 = &avgPt1;
		}

		if ((valid & 0x04) == 0x04)
		{
			avgPt1.waveHeight += waveGridPtC1->waveHeight;
			count++;
		}
		else
		{
			waveGridPtC1 = &avgPt1;
		}

		if ((valid & 0x08) == 0x08)
		{
			avgPt1.waveHeight += waveGridPtD1->waveHeight;
			count++;
		}
		else
		{
			waveGridPtD1 = &avgPt1;
		}

		avgPt1.waveHeight /= count;
	}


	const double xFrac = (ilon == 0 && pos->lon == 180.0) ? 0.0 : pos->lon - ((double) (ilon - 180));
	const double yFrac = pos->lat - ((double) (ilat - 90));

	const long tDiff = _waveGridPhaseTime - time(0);
	double tFrac = 1.0 - (((double) tDiff) / ((double) WAVE_DATA_PHASE_IN_SECONDS));
	if (tFrac < 0.0)
	{
		tFrac = 0.0;
	}
	else if (tFrac > 1.0)
	{
		tFrac = 1.0;
	}


	const double height0_0 = (waveGridPtA0->waveHeight * (1.0 - xFrac)) + (waveGridPtB0->waveHeight * xFrac);
	const double height1_0 = (waveGridPtC0->waveHeight * (1.0 - xFrac)) + (waveGridPtD0->waveHeight * xFrac);
	const double height_0 = (height0_0 * (1.0 - yFrac)) + (height1_0 * yFrac);

	const double height0_1 = (waveGridPtA1->waveHeight * (1.0 - xFrac)) + (waveGridPtB1->waveHeight * xFrac);
	const double height1_1 = (waveGridPtC1->waveHeight * (1.0 - xFrac)) + (waveGridPtD1->waveHeight * xFrac);
	const double height_1 = (height0_1 * (1.0 - yFrac)) + (height1_1 * yFrac);

	wd->waveHeight = (height_0 * (1.0 - tFrac)) + (height_1 * tFrac);


	ret = true;

done:
	if (0 != pthread_rwlock_unlock(&_waveGridLock))
	{
		ERRLOG("get: Failed to unlock rwlock!");
	}

	return ret;
}

#define WAVE_GRID_PARSE_BUF_SIZE (256)

static void updateWaveGrid(int grid, const char* waveDataPath)
{
	WaveGridPoint* waveGrid = malloc(WAVE_GRID_X * WAVE_GRID_Y * sizeof(WaveGridPoint));
	if (!waveGrid)
	{
		ERRLOG("updateWxGrid: Alloc failed for waveGrid!");
		goto fail;
	}

	if (grid == -1)
	{
		// Updating wave data grids, so copy previous grid to start with sane values (in case new values are unavailable for some reason).
		memcpy(waveGrid, _waveGrid1, WAVE_GRID_X * WAVE_GRID_Y * sizeof(WaveGridPoint));
	}
	else
	{
		// Setting up wave data grid for the first time, so initialize the grid all negative float values (indicating invalid data).
		memset(waveGrid, 0xf0, WAVE_GRID_X * WAVE_GRID_Y * sizeof(WaveGridPoint));
	}

	FILE* fp;
	char buf[WAVE_GRID_PARSE_BUF_SIZE];

	float x, y;
	float waveHeight;


	fp = fopen(waveDataPath, "r");
	if (fp == 0)
	{
		goto fail;
	}

	while (fgets(buf, WAVE_GRID_PARSE_BUF_SIZE, fp) == buf)
	{
		if (readWavePoint(buf, &x, &y, &waveHeight) != 0)
		{
			goto fail;
		}

		insertWaveGridPoint(waveGrid, x, y, waveHeight);
	}
	fclose(fp);


	if (grid != -1)
	{
		if (grid == 0)
		{
			_waveGrid0 = waveGrid;
		}
		else
		{
			_waveGrid1 = waveGrid;
		}

		ERRLOG2("Initialized wave grid %d (from %s).", grid, waveDataPath);
	}
	else
	{
		if (0 != pthread_rwlock_wrlock(&_waveGridLock))
		{
			ERRLOG("updateWaveGrid: Failed to lock for write!");
			goto fail;
		}

		// Update, so free grid 0 data, grid 0 gets grid 1 data, and grid 1 gets latest data.
		free(_waveGrid0);
		_waveGrid0 = _waveGrid1;
		_waveGrid1 = waveGrid;

		_waveGridPhaseTime = time(0) + WAVE_DATA_PHASE_IN_SECONDS;

		if (0 != pthread_rwlock_unlock(&_waveGridLock))
		{
			ERRLOG("updateWaveGrid: Failed to unlock rwlock!");
		}

		ERRLOG2("Updated wave grids (latest from %s). Grid phase time: %lu", waveDataPath, _waveGridPhaseTime);
	}

	return;

fail:
	ERRLOG("Failed to update wave grid!");
	free(waveGrid);
}

static int readWavePoint(char* s, float* x, float* y, float* waveHeight)
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
	*waveHeight = strtof(w, 0);

	return 0;
}

static void insertWaveGridPoint(WaveGridPoint* waveGrid, float lon, float lat, float waveHeight)
{
	if (lon >= 180.0)
	{
		lon -= 360.0;
	}

	int ilon = ((int) roundf(lon)) + 180;
	int ilat = ((int) roundf(lat)) + 90;

	if (ilat < 0 || ilat >= WAVE_GRID_Y)
	{
		ERRLOG4("Failed to insert wave grid point at %f,%f (%d, %d).", lon, lat, ilon, ilat);
		return;
	}

	if (ilon == WAVE_GRID_X)
	{
		ilon = 0;
	}

	WaveGridPoint* p = waveGrid + getXYIndex(ilon, ilat);

	p->waveHeight = waveHeight;
}

static int getXYIndex(int x, int y)
{
	return y * WAVE_GRID_X + x;
}


static void* waveUpdaterMain(void* arg)
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
			const char* waveDataPath = ((hour == 18) ? _f1File : _f2File);
			updateWaveGrid(-1, waveDataPath);
			update = false;
		}

		// TODO: Sleep until we need to update the grids. Polling with a short sleep here is lazy and causes pointless frequent wakeups.
		sleep(60);
	}

	return 0;
}
