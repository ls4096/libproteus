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

#include "tests.h"
#include "tests_assert.h"

#include "proteus/Wave.h"

#define WAVE_DATA_FILE_1 "./test_data/wave/f1.csv"
#define WAVE_DATA_FILE_2 "./test_data/wave/f1.csv"

static int test_spatial_interpolation();
static int test_spatial_interpolation_180();

int test_Wave_run()
{
	if (0 != proteus_Wave_init(WAVE_DATA_FILE_1, WAVE_DATA_FILE_2))
	{
		return 1;
	}

	proteus_GeoPos p;
	proteus_WaveData wd;


	p.lat = 55.0;
	p.lon = -100.0;
	IS_FALSE(proteus_Wave_get(&p, &wd));

	p.lat = -80.0;
	p.lon = 50.0;
	IS_FALSE(proteus_Wave_get(&p, &wd));

	p.lat = 40.0;
	p.lon = -60.0;
	IS_TRUE(proteus_Wave_get(&p, &wd));
	EQUALS_FLT(1.96f, wd.waveHeight);

	p.lat = -65.0;
	p.lon = 70.0;
	IS_TRUE(proteus_Wave_get(&p, &wd));
	EQUALS_FLT(0.72f, wd.waveHeight);


	p.lat = -36.0;
	p.lon = 0.0;
	IS_TRUE(proteus_Wave_get(&p, &wd));
	EQUALS_FLT(3.57f, wd.waveHeight);

	p.lat = -36.0;
	p.lon = -0.0;
	IS_TRUE(proteus_Wave_get(&p, &wd));
	EQUALS_FLT(3.57f, wd.waveHeight);


	p.lat = -36.0;
	p.lon = -180.0;
	IS_TRUE(proteus_Wave_get(&p, &wd));
	EQUALS_FLT(2.35f, wd.waveHeight);

	p.lat = -36.0;
	p.lon = 180.0;
	IS_TRUE(proteus_Wave_get(&p, &wd));
	EQUALS_FLT(2.35f, wd.waveHeight);


	if (test_spatial_interpolation() != 0)
	{
		return 1;
	}

	if (test_spatial_interpolation_180() != 0)
	{
		return 1;
	}


	return 0;
}

static int test_spatial_interpolation()
{
	// Some basic spatial interpolation checks

	proteus_GeoPos p;
	proteus_WaveData wd;


	// A
	p.lat = -40.0;
	p.lon = 20.0;
	IS_TRUE(proteus_Wave_get(&p, &wd));
	EQUALS_FLT(3.23f, wd.waveHeight);

	// B
	p.lat = -40.0;
	p.lon = 21.0;
	IS_TRUE(proteus_Wave_get(&p, &wd));
	EQUALS_FLT(3.08f, wd.waveHeight);

	// C
	p.lat = -41.0;
	p.lon = 20.0;
	IS_TRUE(proteus_Wave_get(&p, &wd));
	EQUALS_FLT(3.54f, wd.waveHeight);

	// D
	p.lat = -41.0;
	p.lon = 21.0;
	IS_TRUE(proteus_Wave_get(&p, &wd));
	EQUALS_FLT(3.32f, wd.waveHeight);


	// 0.75A + 0.25B
	p.lat = -40.0;
	p.lon = 20.25;
	IS_TRUE(proteus_Wave_get(&p, &wd));
	EQUALS_FLT((3.23f * 0.75f) + (3.08f * 0.25f), wd.waveHeight);

	// 0.75A + 0.25C
	p.lat = -40.25;
	p.lon = 20.0;
	IS_TRUE(proteus_Wave_get(&p, &wd));
	EQUALS_FLT((3.23f * 0.75f) + (3.54f * 0.25f), wd.waveHeight);

	// 0.75C + 0.25D
	p.lat = -41.0;
	p.lon = 20.25;
	IS_TRUE(proteus_Wave_get(&p, &wd));
	EQUALS_FLT((3.54f * 0.75f) + (3.32f * 0.25f), wd.waveHeight);

	// 0.75 * (0.75A + 0.25B) + 0.25 * (0.75C + 0.25D)
	p.lat = -40.25;
	p.lon = 20.25;
	IS_TRUE(proteus_Wave_get(&p, &wd));
	EQUALS_FLT(((3.23f * 0.75f) + (3.08f * 0.25f)) * 0.75f + ((3.54f * 0.75f) + (3.32f * 0.25f)) * 0.25f, wd.waveHeight);


	return 0;
}

static int test_spatial_interpolation_180()
{
	// Some basic spatial interpolation checks around 180 longitude.

	proteus_GeoPos p;
	proteus_WaveData wd;


	// X
	p.lat = -40.0;
	p.lon = 179.0;
	IS_TRUE(proteus_Wave_get(&p, &wd));
	EQUALS_FLT(4.84f, wd.waveHeight);

	// Y
	p.lat = -40.0;
	p.lon = 180.0;
	IS_TRUE(proteus_Wave_get(&p, &wd));
	EQUALS_FLT(5.11f, wd.waveHeight);

	// Z
	p.lat = -40.0;
	p.lon = -179.0;
	IS_TRUE(proteus_Wave_get(&p, &wd));
	EQUALS_FLT(5.27f, wd.waveHeight);


	// 0.5X + 0.5Y
	p.lat = -40.0;
	p.lon = 179.5;
	IS_TRUE(proteus_Wave_get(&p, &wd));
	EQUALS_FLT((4.84f * 0.5f) + (5.11f * 0.5f), wd.waveHeight);

	// 0.5Y + 0.5Z
	p.lat = -40.0;
	p.lon = -179.5;
	IS_TRUE(proteus_Wave_get(&p, &wd));
	EQUALS_FLT((5.11f * 0.5f) + (5.27f * 0.5f), wd.waveHeight);


	return 0;
}
