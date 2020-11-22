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
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#include "proteus_internal.h"

#include "proteus/GeoInfo.h"
#include "Decompress.h"
#include "ErrLog.h"

#define ERRLOG_ID "proteus_GeoInfo"


#define SQ_DEG_GRID_SIZE (450 * 3600)
#define NUM_GRIDS (360 * 181)

#define GRID_PRUNER_INTERVAL (60 * 60)
#define GRID_PRUNER_EXPIRY (6 * 60 * 60)

#define GEO_INFO_DATA_PATH_MAXLEN (4096 - 64)


typedef struct
{
	bool loaded;
	uint8_t* grid;
	time_t lastUsed;

	pthread_mutex_t lock;
} SquareDegree;

const char* _dataDir = 0;

static SquareDegree* _grids = 0;

static int getLonLatIndex(int lon, int lat);
static void loadSquareDegree(SquareDegree* sd, int ilon, int ilat);
static bool sdGridIsWater(const proteus_GeoPos* pos, const uint8_t* grid);

static pthread_t _gridPrunerThread;
static void* gridPrunerMain(void* arg);


PROTEUS_API int proteus_GeoInfo_init(const char* dataDir)
{
	if (!dataDir)
	{
		return -3;
	}

	if (strlen(dataDir) >= GEO_INFO_DATA_PATH_MAXLEN)
	{
		return -3;
	}

	_dataDir = strdup(dataDir);

	_grids = (SquareDegree*) malloc(NUM_GRIDS * sizeof(SquareDegree));

	for (int i = 0 ; i < NUM_GRIDS; i++)
	{
		SquareDegree* sd = _grids + i;

		sd->loaded = false;
		sd->grid = 0;
		sd->lastUsed = 0;

		pthread_mutex_init(&sd->lock, 0);
	}

	if (0 != pthread_create(&_gridPrunerThread, 0, &gridPrunerMain, 0))
	{
		ERRLOG("Failed to create grid pruner thread!");
		return -1;
	}

	return 0;
}

PROTEUS_API bool proteus_GeoInfo_isWater(const proteus_GeoPos* pos)
{
	const int ilon = (int) floor(pos->lon);
	const int ilat = (int) floor(pos->lat);

	const int index = getLonLatIndex(ilon, ilat);

	SquareDegree* sd = _grids + index;
	pthread_mutex_t* l = &sd->lock;
	pthread_mutex_lock(l);

	if (!sd->loaded)
	{
		loadSquareDegree(sd, ilon, ilat);
	}

	if (sd->grid == 0)
	{
		// No grid means there was no data file for this square degree.
		pthread_mutex_unlock(l);

		// Assume water if latitude >= -79.
		// Assume land or ice shelf (Antarctica) if latitude < -79.
		return (ilat >= -79);
	}

	const bool isWater = sdGridIsWater(pos, sd->grid);

	sd->lastUsed = time(0);

	pthread_mutex_unlock(l);
	return isWater;
}


static int getLonLatIndex(int lon, int lat)
{
	return (((lat + 90) * 360) + (lon + 180));
}

static void loadSquareDegree(SquareDegree* sd, int ilon, int ilat)
{
	char filename[GEO_INFO_DATA_PATH_MAXLEN + 64];

	char ns = 'N';
	char ew = 'E';

	if (ilon < 0)
	{
		ew = 'W';
		ilon = -ilon;
	}

	if (ilat < 0)
	{
		ns = 'S';
		ilat = -ilat;
	}

	snprintf(filename, GEO_INFO_DATA_PATH_MAXLEN + 64, "%s/%c%02d%c%03d.gz", _dataDir, ns, ilat, ew, ilon);

	char* fileData = 0;
	uint8_t* newGrid = 0;

	FILE* f = fopen(filename, "r");
	if (f == 0)
	{
		if (errno != ENOENT)
		{
			ERRLOG1("Failed to load square degree: %s", filename);
			goto fail;
		}
		else
		{
			ERRLOG1("Didn't find square degree file %s, so assuming all water.", filename);
			goto loaded;
		}
	}
	else
	{
		ERRLOG1("Found %s", filename);

		fseek(f, 0L, SEEK_END);
		long fileDataLen = ftell(f);
		rewind(f);

		fileData = (char*) malloc(fileDataLen);
		if (fread(fileData, 1, fileDataLen, f) != fileDataLen)
		{
			ERRLOG1("Failed to read expected file contents from file: %s", filename);
			goto fail;
		}

		newGrid = (uint8_t*) malloc(SQ_DEG_GRID_SIZE);
		int rc;
		if (0 != (rc = Decompress_inflate(newGrid, SQ_DEG_GRID_SIZE, fileData, fileDataLen)))
		{
			ERRLOG2("Failed to decompress with code %d: %s", rc, filename);
			goto fail;
		}

		sd->grid = newGrid;
	}

loaded:
	sd->loaded = true;

fail:
	if (fileData)
	{
		free(fileData);
	}

	if (!sd->loaded && newGrid)
	{
		free(newGrid);
	}

	if (f)
	{
		fclose(f);
	}
}

static bool sdGridIsWater(const proteus_GeoPos* pos, const uint8_t* grid)
{
	const double lonFrac = pos->lon - floor(pos->lon);
	const double latFrac = pos->lat - floor(pos->lat);

	const int x = (int) (lonFrac * 3600.0);
	const int y = (int) (latFrac * 3600.0);

	// Byte in grid
	const uint8_t b = grid[(3599 - y) * 450 + (x >> 3)];

	// Bit position in byte
	const int a = 7 - (x & 0x07);

	return (((b >> a) & 0x01) == 0);
}


static void* gridPrunerMain(void* arg)
{
	for (;;)
	{
		sleep(GRID_PRUNER_INTERVAL);

		ERRLOG("Grid pruner starting...");

		unsigned int loadedCount = 0;
		unsigned int griddedCount = 0;
		unsigned int retainedCount = 0;

		const time_t curTime = time(0);

		for (int i = 0; i < NUM_GRIDS; i++)
		{
			SquareDegree* sd = _grids + i;
			pthread_mutex_t* l = &sd->lock;
			pthread_mutex_lock(l);

			if (sd->loaded)
			{
				loadedCount++;

				if (sd->grid != 0)
				{
					griddedCount++;

					if (sd->lastUsed < curTime - GRID_PRUNER_EXPIRY)
					{
						free(sd->grid);
						sd->grid = 0;
						sd->loaded = false;
					}
					else
					{
						retainedCount++;
					}
				}
			}

			pthread_mutex_unlock(l);
		}

		ERRLOG3("Grid pruner done. loaded=%u, gridded=%u, retained=%u", loadedCount, griddedCount, retainedCount);
	}

	return 0;
}
