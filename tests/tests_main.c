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

#include <stdio.h>

#include <proteus/proteus.h>

#include "tests.h"

typedef int (*test_func)(void);

static const char* TEST_NAMES[] = {
	"Celestial",
	"Compass",
	"GeoInfo",
	"GeoPos",
	"GeoVec",
	"Ocean",
	"ScalarConv",
	"Wave",
	"Weather"
};

static const test_func TEST_FUNCS[] = {
	&test_Celestial_run,
	&test_Compass_run,
	&test_GeoInfo_run,
	&test_GeoPos_run,
	&test_GeoVec_run,
	&test_Ocean_run,
	&test_ScalarConv_run,
	&test_Wave_run,
	&test_Weather_run
};

int main()
{
	int testres;
	int sum = 0;

	printf("Running tests for libproteus v%s\n", proteus_getVersionString());

	for (size_t i = 0; i < (sizeof(TEST_NAMES) / sizeof(const char*)); i++)
	{
		printf("%s...\n", TEST_NAMES[i]);

		if (0 != (testres = TEST_FUNCS[i]()))
		{
			printf("\tFAILED!\n");
		}
		else
		{
			printf("\tOK\n");
		}

		sum += testres;
	}

	if (sum == 0)
	{
		printf("All tests passed!\n");
	}
	else
	{
		printf("There were test failures!\n");
	}

	return sum;
}
