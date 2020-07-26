#include <stdio.h>

#include "tests.h"

typedef int (*test_func)(void);

static const char* TEST_NAMES[] = {
	"Compass",
	"GeoInfo",
	"GeoPos",
	"GeoVec",
	"Ocean",
	"ScalarConv",
	"Weather"
};

static const test_func TEST_FUNCS[] = {
	&test_Compass_run,
	&test_GeoInfo_run,
	&test_GeoPos_run,
	&test_GeoVec_run,
	&test_Ocean_run,
	&test_ScalarConv_run,
	&test_Weather_run
};

int main(int argc, char** argv)
{
	int testres;
	int sum = 0;

	for (int i = 0; i < (sizeof(TEST_NAMES) / sizeof(const char*)); i++)
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
