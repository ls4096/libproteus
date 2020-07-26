#ifndef _proteus_GeoVec_h_
#define _proteus_GeoVec_h_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/**
 * A geographical vector, usually for describing a change in position
 * or a velocity, depending on context.
 */
typedef struct
{
	double angle; // in degrees
	double mag; // Unit depends on context (typically metres or metres/second).
} proteus_GeoVec;

/**
 * Adds the vector w to the vector v.
 *
 * Parameters
 * 	v [in/out]: the vector being modified
 * 	w [in]: the vector being added to vector v
 */
void proteus_GeoVec_add(proteus_GeoVec* v, const proteus_GeoVec* w);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _proteus_GeoVec_h_
