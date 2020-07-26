#ifndef _proteus_Compass_h_
#define _proteus_Compass_h_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/**
 * Obtains the difference between two compass angles.
 *
 * Specifically, this provides the answer to the question:
 * How many degrees to travel from compass heading "a" to
 * compass heading "b", where negative values indicate "to
 * the left" and positive values indicate "to the right".
 *
 * Parameters
 * 	a [in]: the starting angle in degrees
 * 	b [in]: the finishing angle in degrees
 *
 * Returns the difference in degrees between the two angles,
 * in the range (-180, 180].
 */
double proteus_Compass_diff(double a, double b);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _proteus_Compass_h_
