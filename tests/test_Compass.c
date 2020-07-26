#include "tests.h"
#include "tests_assert.h"

#include "proteus/Compass.h"

int test_Compass_run()
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
