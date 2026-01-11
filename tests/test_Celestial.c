/**
 * Copyright (C) 2021-2026 ls4096 <ls4096@8bitbyte.ca>
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

#include <stdlib.h>
#include <string.h>

#include "tests.h"
#include "tests_assert.h"

#include "proteus/Celestial.h"

static int testSunBasic();
static int testEquatorialData();
static int testHorizontalData();

int test_Celestial_run()
{
	if (0 != testSunBasic())
	{
		return 1;
	}

	if (0 != testEquatorialData())
	{
		return 1;
	}

	if (0 != testHorizontalData())
	{
		return 1;
	}

	return 0;
}

static int testSunBasic()
{
	double jd;
	proteus_GeoPos pos;
	proteus_CelestialEquatorialCoord ec;
	proteus_CelestialHorizontalCoord hc;

	int rc;


	// 2021-04-01 noon at Greenwich.
	jd = 2459306.0;
	pos.lat = 51.478;
	pos.lon = 0.0;

	rc = proteus_Celestial_getEquatorialForObject(jd, PROTEUS_CELESTIAL_OBJ_SUN, &ec);
	EQUALS(rc, 0);

	// April 1, so Sun equatorial declination should be positive.
	IS_TRUE(ec.dec > 0.0 && ec.dec < 24.0);
	double prevEcDec = ec.dec;

	rc = proteus_Celestial_convertEquatorialToHorizontal(jd, &pos, &ec, false, 1010.0, 10.0, &hc);
	EQUALS(rc, 0);

	// At noon at Greenwich, so Sun should be above horizon near azimuth 180.
	IS_TRUE(hc.alt > 0.0);
	IS_TRUE(hc.alt <= 90.0); // Always at or below 90 degrees.
	IS_TRUE(hc.az >= 170.0);
	IS_TRUE(hc.az <= 190.0);


	// Advance by 12 hours.
	jd += 0.5;

	rc = proteus_Celestial_getEquatorialForObject(jd, PROTEUS_CELESTIAL_OBJ_SUN, &ec);
	EQUALS(rc, 0);

	// Sun equatorial declination should still be positive, and also greater than 12 hours ago (based on time of year).
	IS_TRUE(ec.dec > 0.0 && ec.dec < 24.0);
	IS_TRUE(ec.dec > prevEcDec);

	rc = proteus_Celestial_convertEquatorialToHorizontal(jd, &pos, &ec, false, 1010.0, 10.0, &hc);
	EQUALS(rc, 0);

	// At midnight at Greenwich, so Sun should be below horizon near azimuth 0.
	IS_TRUE(hc.alt < 0.0);
	IS_TRUE(hc.alt >= -90.0); // Always at or above -90 degrees.
	IS_TRUE(hc.az >= 350.0 || hc.az <= 10.0);
	IS_TRUE(hc.az <= 360.0 && hc.az >= 0.0);


	return 0;
}


// Test data for comparison generated using Astropy
#define EQUATORIAL_DATA_FILE "./test_data/astro/equatorial.csv"
#define EQUATORIAL_DATA_FILE_LINES 2724
#define HORIZONTAL_DATA_FILE "./test_data/astro/horizontal.csv"
#define HORIZONTAL_DATA_FILE_LINES 3888

#define READ_CSV_LINE_BUF_SIZE 1024

typedef struct {
	int obj;
	long t;
	double ra;
	double dec;
} CsvEq;

typedef struct {
	int obj;
	long t;
	double lat;
	double lon;
	double az;
	double alt;
} CsvHo;

static int readCsvEq(const char* filename, CsvEq* csveqs, const int max)
{
	FILE* fp = fopen(filename, "r");
	if (!fp)
	{
		return -1;
	}

	char buf[READ_CSV_LINE_BUF_SIZE];
	int n = 0;

	while (fgets(buf, READ_CSV_LINE_BUF_SIZE, fp) == buf)
	{
		if (n >= max)
		{
			n = -2;
			goto fail;
		}

		char* t;
		char* w;

		if ((w = strtok_r(buf, ",", &t)) == 0)
		{
			n = -3;
			goto fail;
		}
		csveqs[n].obj = strtol(w, 0, 10);

		if ((w = strtok_r(0, ",", &t)) == 0)
		{
			n = -4;
			goto fail;
		}
		csveqs[n].t = strtol(w, 0, 10);

		if ((w = strtok_r(0, ",", &t)) == 0)
		{
			n = -5;
			goto fail;
		}
		csveqs[n].ra = strtod(w, 0);

		if ((w = strtok_r(0, ",", &t)) == 0)
		{
			n = -6;
			goto fail;
		}
		csveqs[n].dec = strtod(w, 0);

		n++;
	}

fail:
	fclose(fp);
	return n;
}

static int readCsvHo(const char* filename, CsvHo* csvhos, const int max)
{
	FILE* fp = fopen(filename, "r");
	if (!fp)
	{
		return -1;
	}

	char buf[READ_CSV_LINE_BUF_SIZE];
	int n = 0;

	while (fgets(buf, READ_CSV_LINE_BUF_SIZE, fp) == buf)
	{
		if (n >= max)
		{
			n = -2;
			goto fail;
		}

		char* t;
		char* w;

		if ((w = strtok_r(buf, ",", &t)) == 0)
		{
			n = -3;
			goto fail;
		}
		csvhos[n].obj = strtol(w, 0, 10);

		if ((w = strtok_r(0, ",", &t)) == 0)
		{
			n = -4;
			goto fail;
		}
		csvhos[n].t = strtol(w, 0, 10);

		if ((w = strtok_r(0, ",", &t)) == 0)
		{
			n = -5;
			goto fail;
		}
		csvhos[n].lat = strtod(w, 0);

		if ((w = strtok_r(0, ",", &t)) == 0)
		{
			n = -6;
			goto fail;
		}
		csvhos[n].lon = strtod(w, 0);

		if ((w = strtok_r(0, ",", &t)) == 0)
		{
			n = -7;
			goto fail;
		}
		csvhos[n].az = strtod(w, 0);

		if ((w = strtok_r(0, ",", &t)) == 0)
		{
			n = -8;
			goto fail;
		}
		csvhos[n].alt = strtod(w, 0);

		n++;
	}

fail:
	fclose(fp);
	return n;
}

#define MAX_CSV_LINE_COUNT 5000

static int testEquatorialData()
{
	CsvEq eqs[MAX_CSV_LINE_COUNT];

	int n = readCsvEq(EQUATORIAL_DATA_FILE, eqs, MAX_CSV_LINE_COUNT);
	EQUALS(n, EQUATORIAL_DATA_FILE_LINES);

	for (int i = 0; i < n; i++)
	{
		proteus_CelestialEquatorialCoord ec;
		int rc = proteus_Celestial_getEquatorialForObject(
			proteus_Celestial_getJulianDayForTime(eqs[i].t),
			eqs[i].obj,
			&ec);
		EQUALS(rc, 0);

		IS_TRUE(ec.ra >= 0.0);
		IS_TRUE(ec.ra < 24.0);
		IS_TRUE(ec.dec >= -90.0);
		IS_TRUE(ec.dec <= 90.0);

		if (ec.ra > eqs[i].ra && ec.ra - eqs[i].ra > 12.0)
		{
			ec.ra -= 24.0;
		}
		else if (ec.ra < eqs[i].ra && eqs[i].ra - ec.ra > 12.0)
		{
			ec.ra += 24.0;
		}

		const double diffRa = fabs(ec.ra - eqs[i].ra);
		const double diffDec = fabs(ec.dec - eqs[i].dec);

		IS_TRUE(diffRa * 15.0 < 3.0); // within 3 deg
		IS_TRUE(diffDec * 60.0 < 1.0); // within 1 arcminute
	}

	return 0;
}

static int testHorizontalData()
{
	CsvHo hos[MAX_CSV_LINE_COUNT];

	int n = readCsvHo(HORIZONTAL_DATA_FILE, hos, MAX_CSV_LINE_COUNT);
	EQUALS(n, HORIZONTAL_DATA_FILE_LINES);

	for (int i = 0; i < n; i++)
	{
		proteus_CelestialEquatorialCoord ec;
		proteus_CelestialHorizontalCoord hc;

		int rc = proteus_Celestial_getEquatorialForObject(
			proteus_Celestial_getJulianDayForTime(hos[i].t),
			hos[i].obj,
			&ec);
		EQUALS(rc, 0);

		IS_TRUE(ec.ra >= 0.0);
		IS_TRUE(ec.ra < 24.0);
		IS_TRUE(ec.dec >= -90.0);
		IS_TRUE(ec.dec <= 90.0);

		proteus_GeoPos pos = { .lat = hos[i].lat, .lon = hos[i].lon };
		rc = proteus_Celestial_convertEquatorialToHorizontal(
			proteus_Celestial_getJulianDayForTime(hos[i].t),
			&pos,
			&ec,
			false,
			0.0,
			0.0,
			&hc);
		EQUALS(rc, 0);

		IS_TRUE(hc.az >= 0.0);
		IS_TRUE(hc.az < 360.0);
		IS_TRUE(hc.alt >= -90.0);
		IS_TRUE(hc.alt <= 90.0);

		if (hc.az > hos[i].az && hc.az - hos[i].az > 180.0)
		{
			hc.az -= 180.0;
		}
		else if (hc.az < hos[i].az && hos[i].az - hc.az > 180.0)
		{
			hc.az += 180.0;
		}

		const double diffAz = fabs(hc.az - hos[i].az);
		const double diffAlt = fabs(hc.alt - hos[i].alt);

		IS_TRUE(diffAz < 3.0); // within 3 deg
		IS_TRUE(diffAlt * 60.0 < 1.0); // within 1 arcminute
	}

	return 0;
}
