/**
 * Copyright (C) 2020-2025 ls4096 <ls4096@8bitbyte.ca>
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

#include "proteus/Compass.h"

#define MAG_DATA_FILE "./test_data/compass_mag/mag_dec.csv"

static int testCompassDiff();
static int testCompassMag();

int test_Compass_run()
{
	if (0 != testCompassDiff())
	{
		return 1;
	}

	if (0 != testCompassMag())
	{
		return 1;
	}

	return 0;
}

static int testCompassDiff()
{
	for (double a = 0.0; a <= 360.01; a += 0.1)
	{
		// Diff between two identical angles should always be zero.
		EQUALS_DBL(0.0, proteus_Compass_diff(a, a));
	}

	EQUALS_DBL(1.0, proteus_Compass_diff(0.0, 1.0));
	EQUALS_DBL(1.0, proteus_Compass_diff(360.0, 1.0));

	EQUALS_DBL(-1.0, proteus_Compass_diff(1.0, 0.0));
	EQUALS_DBL(-1.0, proteus_Compass_diff(1.0, 360.0));

	EQUALS_DBL(2.0, proteus_Compass_diff(359.0, 1.0));
	EQUALS_DBL(-2.0, proteus_Compass_diff(1.0, 359.0));

	EQUALS_DBL(180.0, proteus_Compass_diff(180.0, 360.0));
	EQUALS_DBL(180.0, proteus_Compass_diff(360.0, 180.0));

	EQUALS_DBL(180.0, proteus_Compass_diff(180.0, 0.0));
	EQUALS_DBL(180.0, proteus_Compass_diff(0.0, 180.0));

	EQUALS_DBL(180.0, proteus_Compass_diff(90.0, 270.0));
	EQUALS_DBL(180.0, proteus_Compass_diff(270.0, 90.0));

	EQUALS_DBL(179.0, proteus_Compass_diff(359.0, 178.0));
	EQUALS_DBL(-179.0, proteus_Compass_diff(178.0, 359.0));

	EQUALS_DBL(90.0, proteus_Compass_diff(45.0, 135.0));
	EQUALS_DBL(-90.0, proteus_Compass_diff(135.0, 45.0));

	EQUALS_DBL(90.0, proteus_Compass_diff(315.0, 45.0));
	EQUALS_DBL(-90.0, proteus_Compass_diff(45.0, 315.0));

	return 0;
}


static int test_spatial_interpolation();
static int test_spatial_interpolation_180();
static int test_temporal_interpolation();

#define MAG_DATA_SEC_IN_YEAR (31557600)
#define MAG_DATA_SEC_2020 (1577836800)

static const time_t T_2019 = MAG_DATA_SEC_2020 - MAG_DATA_SEC_IN_YEAR;
static const time_t T_2020 = MAG_DATA_SEC_2020;
static const time_t T_2021 = MAG_DATA_SEC_2020 + (1 * MAG_DATA_SEC_IN_YEAR);
static const time_t T_2022 = MAG_DATA_SEC_2020 + (2 * MAG_DATA_SEC_IN_YEAR);
static const time_t T_2023 = MAG_DATA_SEC_2020 + (3 * MAG_DATA_SEC_IN_YEAR);
static const time_t T_2024 = MAG_DATA_SEC_2020 + (4 * MAG_DATA_SEC_IN_YEAR);
static const time_t T_2025 = MAG_DATA_SEC_2020 + (5 * MAG_DATA_SEC_IN_YEAR);
static const time_t T_2026 = MAG_DATA_SEC_2020 + (6 * MAG_DATA_SEC_IN_YEAR);
static const time_t T_2030 = MAG_DATA_SEC_2020 + (10 * MAG_DATA_SEC_IN_YEAR);
static const time_t T_2031 = MAG_DATA_SEC_2020 + (11 * MAG_DATA_SEC_IN_YEAR);

static int testCompassMag()
{
	if (0 != proteus_Compass_init(MAG_DATA_FILE))
	{
		return 1;
	}

	proteus_GeoPos p;


	p.lat = 55.0;
	p.lon = -100.0;
	EQUALS_FLT(5.15, proteus_Compass_magdec(&p, T_2019));
	EQUALS_FLT(5.15, proteus_Compass_magdec(&p, T_2020));
	EQUALS_FLT(5.12, proteus_Compass_magdec(&p, T_2021));
	EQUALS_FLT(5.09, proteus_Compass_magdec(&p, T_2022));
	EQUALS_FLT(5.06, proteus_Compass_magdec(&p, T_2023));
	EQUALS_FLT(5.04, proteus_Compass_magdec(&p, T_2024));
	EQUALS_FLT(5.01, proteus_Compass_magdec(&p, T_2025));
	EQUALS_FLT(4.92, proteus_Compass_magdec(&p, T_2026));
	EQUALS_FLT(4.76, proteus_Compass_magdec(&p, T_2030));
	EQUALS_FLT(4.76, proteus_Compass_magdec(&p, T_2031));

	p.lat = -80.0;
	p.lon = 50.0;
	EQUALS_FLT(-66.99, proteus_Compass_magdec(&p, T_2019));
	EQUALS_FLT(-66.99, proteus_Compass_magdec(&p, T_2020));
	EQUALS_FLT(-67.15, proteus_Compass_magdec(&p, T_2021));
	EQUALS_FLT(-67.32, proteus_Compass_magdec(&p, T_2022));
	EQUALS_FLT(-67.49, proteus_Compass_magdec(&p, T_2023));
	EQUALS_FLT(-67.65, proteus_Compass_magdec(&p, T_2024));
	EQUALS_FLT(-67.82, proteus_Compass_magdec(&p, T_2025));
	EQUALS_FLT(-67.95, proteus_Compass_magdec(&p, T_2026));
	EQUALS_FLT(-68.58, proteus_Compass_magdec(&p, T_2030));
	EQUALS_FLT(-68.58, proteus_Compass_magdec(&p, T_2031));

	p.lat = 40.0;
	p.lon = -60.0;
	EQUALS_FLT(-16.30, proteus_Compass_magdec(&p, T_2019));
	EQUALS_FLT(-16.30, proteus_Compass_magdec(&p, T_2020));
	EQUALS_FLT(-16.18, proteus_Compass_magdec(&p, T_2021));
	EQUALS_FLT(-16.06, proteus_Compass_magdec(&p, T_2022));
	EQUALS_FLT(-15.94, proteus_Compass_magdec(&p, T_2023));
	EQUALS_FLT(-15.82, proteus_Compass_magdec(&p, T_2024));
	EQUALS_FLT(-15.70, proteus_Compass_magdec(&p, T_2025));
	EQUALS_FLT(-15.60, proteus_Compass_magdec(&p, T_2026));
	EQUALS_FLT(-15.15, proteus_Compass_magdec(&p, T_2030));
	EQUALS_FLT(-15.15, proteus_Compass_magdec(&p, T_2031));

	p.lat = -65.0;
	p.lon = 70.0;
	EQUALS_FLT(-72.17, proteus_Compass_magdec(&p, T_2019));
	EQUALS_FLT(-72.17, proteus_Compass_magdec(&p, T_2020));
	EQUALS_FLT(-72.36, proteus_Compass_magdec(&p, T_2021));
	EQUALS_FLT(-72.54, proteus_Compass_magdec(&p, T_2022));
	EQUALS_FLT(-72.73, proteus_Compass_magdec(&p, T_2023));
	EQUALS_FLT(-72.92, proteus_Compass_magdec(&p, T_2024));
	EQUALS_FLT(-73.10, proteus_Compass_magdec(&p, T_2025));
	EQUALS_FLT(-73.28, proteus_Compass_magdec(&p, T_2026));
	EQUALS_FLT(-74.03, proteus_Compass_magdec(&p, T_2030));
	EQUALS_FLT(-74.03, proteus_Compass_magdec(&p, T_2031));

	p.lat = -36.0;
	p.lon = 0.0;
	EQUALS_FLT(-22.68, proteus_Compass_magdec(&p, T_2019));
	EQUALS_FLT(-22.68, proteus_Compass_magdec(&p, T_2020));
	EQUALS_FLT(-22.51, proteus_Compass_magdec(&p, T_2021));
	EQUALS_FLT(-22.34, proteus_Compass_magdec(&p, T_2022));
	EQUALS_FLT(-22.16, proteus_Compass_magdec(&p, T_2023));
	EQUALS_FLT(-21.98, proteus_Compass_magdec(&p, T_2024));
	EQUALS_FLT(-21.80, proteus_Compass_magdec(&p, T_2025));
	EQUALS_FLT(-21.92, proteus_Compass_magdec(&p, T_2026));
	EQUALS_FLT(-21.41, proteus_Compass_magdec(&p, T_2030));
	EQUALS_FLT(-21.41, proteus_Compass_magdec(&p, T_2031));

	p.lat = -36.0;
	p.lon = -0.0;
	EQUALS_FLT(-22.68, proteus_Compass_magdec(&p, T_2019));
	EQUALS_FLT(-22.68, proteus_Compass_magdec(&p, T_2020));
	EQUALS_FLT(-22.51, proteus_Compass_magdec(&p, T_2021));
	EQUALS_FLT(-22.34, proteus_Compass_magdec(&p, T_2022));
	EQUALS_FLT(-22.16, proteus_Compass_magdec(&p, T_2023));
	EQUALS_FLT(-21.98, proteus_Compass_magdec(&p, T_2024));
	EQUALS_FLT(-21.80, proteus_Compass_magdec(&p, T_2025));
	EQUALS_FLT(-21.92, proteus_Compass_magdec(&p, T_2026));
	EQUALS_FLT(-21.41, proteus_Compass_magdec(&p, T_2030));
	EQUALS_FLT(-21.41, proteus_Compass_magdec(&p, T_2031));


	p.lat = -36.0;
	p.lon = -180.0;
	EQUALS_FLT(19.91, proteus_Compass_magdec(&p, T_2019));
	EQUALS_FLT(19.91, proteus_Compass_magdec(&p, T_2020));
	EQUALS_FLT(20.00, proteus_Compass_magdec(&p, T_2021));
	EQUALS_FLT(20.09, proteus_Compass_magdec(&p, T_2022));
	EQUALS_FLT(20.18, proteus_Compass_magdec(&p, T_2023));
	EQUALS_FLT(20.27, proteus_Compass_magdec(&p, T_2024));
	EQUALS_FLT(20.36, proteus_Compass_magdec(&p, T_2025));
	EQUALS_FLT(20.35, proteus_Compass_magdec(&p, T_2026));
	EQUALS_FLT(20.60, proteus_Compass_magdec(&p, T_2030));
	EQUALS_FLT(20.60, proteus_Compass_magdec(&p, T_2031));

	p.lat = -36.0;
	p.lon = 180.0;
	EQUALS_FLT(19.91, proteus_Compass_magdec(&p, T_2019));
	EQUALS_FLT(19.91, proteus_Compass_magdec(&p, T_2020));
	EQUALS_FLT(20.00, proteus_Compass_magdec(&p, T_2021));
	EQUALS_FLT(20.09, proteus_Compass_magdec(&p, T_2022));
	EQUALS_FLT(20.18, proteus_Compass_magdec(&p, T_2023));
	EQUALS_FLT(20.27, proteus_Compass_magdec(&p, T_2024));
	EQUALS_FLT(20.36, proteus_Compass_magdec(&p, T_2025));
	EQUALS_FLT(20.35, proteus_Compass_magdec(&p, T_2026));
	EQUALS_FLT(20.60, proteus_Compass_magdec(&p, T_2030));
	EQUALS_FLT(20.60, proteus_Compass_magdec(&p, T_2031));


	if (test_spatial_interpolation() != 0)
	{
		return 1;
	}

	if (test_spatial_interpolation_180() != 0)
	{
		return 1;
	}

	if (test_temporal_interpolation() != 0)
	{
		return 1;
	}


	return 0;
}

static int test_spatial_interpolation()
{
	// Some basic spatial interpolation checks

	proteus_GeoPos p;


	// A
	p.lat = -40.0;
	p.lon = 20.0;
	EQUALS_FLT(-30.76, proteus_Compass_magdec(&p, T_2021));

	// B
	p.lat = -40.0;
	p.lon = 21.0;
	EQUALS_FLT(-31.37, proteus_Compass_magdec(&p, T_2021));

	// C
	p.lat = -41.0;
	p.lon = 20.0;
	EQUALS_FLT(-31.26, proteus_Compass_magdec(&p, T_2021));

	// D
	p.lat = -41.0;
	p.lon = 21.0;
	EQUALS_FLT(-31.88, proteus_Compass_magdec(&p, T_2021));


	// 0.75A + 0.25B
	p.lat = -40.0;
	p.lon = 20.25;
	EQUALS_FLT((0.75 * -30.76) + (0.25 * -31.37), proteus_Compass_magdec(&p, T_2021));

	// 0.75A + 0.25C
	p.lat = -40.25;
	p.lon = 20.0;
	EQUALS_FLT((0.75 * -30.76) + (0.25 * -31.26), proteus_Compass_magdec(&p, T_2021));

	// 0.75C + 0.25D
	p.lat = -41.0;
	p.lon = 20.25;
	EQUALS_FLT((0.75 * -31.26) + (0.25 * -31.88), proteus_Compass_magdec(&p, T_2021));

	// 0.75 * (0.75A + 0.25B) + 0.25 * (0.75C + 0.25D)
	p.lat = -40.25;
	p.lon = 20.25;
	EQUALS_FLT(0.75 * ((0.75 * -30.76) + (0.25 * -31.37)) + 0.25 * ((0.75 * -31.26) + (0.25 * -31.88)), proteus_Compass_magdec(&p, T_2021));


	return 0;
}

static int test_spatial_interpolation_180()
{
	// Some basic spatial interpolation checks around 180 longitude.

	proteus_GeoPos p;


	// X
	p.lat = -40.0;
	p.lon = 179.0;
	EQUALS_FLT(22.49, proteus_Compass_magdec(&p, T_2022));

	// Y
	p.lat = -40.0;
	p.lon = 180.0;
	EQUALS_FLT(22.58, proteus_Compass_magdec(&p, T_2022));

	// Z
	p.lat = -40.0;
	p.lon = -179.0;
	EQUALS_FLT(22.67, proteus_Compass_magdec(&p, T_2022));


	// 0.5X + 0.5Y
	p.lat = -40.0;
	p.lon = 179.5;
	EQUALS_FLT(0.5 * 22.49 + 0.5 * 22.58, proteus_Compass_magdec(&p, T_2022));

	// 0.5Y + 0.5Z
	p.lat = -40.0;
	p.lon = -179.5;
	EQUALS_FLT(0.5 * 22.58 + 0.5 * 22.67, proteus_Compass_magdec(&p, T_2022));


	return 0;
}

static int test_temporal_interpolation()
{
	// Some basic temporal interpolation checks

	proteus_GeoPos p;


	// 2021
	p.lat = -40.0;
	p.lon = 20.0;
	EQUALS_FLT(-30.76, proteus_Compass_magdec(&p, T_2021));

	// 2022
	p.lat = -40.0;
	p.lon = 20.0;
	EQUALS_FLT(-30.91, proteus_Compass_magdec(&p, T_2022));

	// 2021.5
	p.lat = -40.0;
	p.lon = 20.0;
	EQUALS_FLT(0.5 * -30.76 + 0.5 * -30.91, proteus_Compass_magdec(&p, (T_2021 + T_2022) / 2));


	// 2024
	p.lat = -40.0;
	p.lon = 20.0;
	EQUALS_FLT(-31.23, proteus_Compass_magdec(&p, T_2024));

	// 2025
	p.lat = -40.0;
	p.lon = 20.0;
	EQUALS_FLT(-31.38, proteus_Compass_magdec(&p, T_2025));

	// 2024.75
	p.lat = -40.0;
	p.lon = 20.0;
	EQUALS_FLT(0.25 * -31.23 + 0.75 * -31.38, proteus_Compass_magdec(&p, T_2024 * 0.25 + T_2025 * 0.75));

	// 2030.75
	p.lat = -40.0;
	p.lon = 20.0;
	EQUALS_FLT(-32.78, proteus_Compass_magdec(&p, T_2030 * 0.25 + T_2031 * 0.75));


	return 0;
}
