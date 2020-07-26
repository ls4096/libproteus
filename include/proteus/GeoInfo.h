#ifndef _proteus_GeoInfo_h_
#define _proteus_GeoInfo_h_

#include <stdbool.h>

#include "proteus/GeoPos.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/**
 * Initializes the geographic information (water/land data) processing system.
 *
 * Parameters
 * 	dataDir [in]: the path to the directory with the geographic information files
 *
 * Returns
 * 	0, on success
 * 	any other value, on failure
 */
int proteus_GeoInfo_init(const char* dataDir);

/**
 * Indicates whether or not water is present at the given geographical position.
 *
 * Parameters
 * 	pos [in]: the geographical position to be queried
 *
 * Returns
 * 	true, if on water
 * 	false, if on land
 */
bool proteus_GeoInfo_isWater(const proteus_GeoPos* pos);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _proteus_GeoInfo_h_
