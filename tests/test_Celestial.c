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

#include "tests.h"
#include "tests_assert.h"

#include "proteus/Celestial.h"


int test_Celestial_run()
{
	// TODO: More comprehensive tests

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
