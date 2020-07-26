#ifndef _proteus_GeoPos_h_
#define _proteus_GeoPos_h_

#include "proteus/GeoVec.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/**
 * A geographical position
 */
typedef struct
{
	double lat; // in decimal degrees (negative values south of Equator)
	double lon; // in decimal degrees (negative values west of Prime Meridian)
} proteus_GeoPos;

/**
 * Adds the positional difference vector v to the position p.
 *
 * Parameters
 * 	p [in/out]: the position being modified
 * 	v [in]: the vector being added to position p
 */
void proteus_GeoPos_advance(proteus_GeoPos* p, const proteus_GeoVec* v);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _proteus_GeoPos_h_
