/**
 * This file contains a list of some simple but useful scalar value conversion functions.
 */

#ifndef _proteus_ScalarConv_h_
#define _proteus_ScalarConv_h_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


// Converts metres to nautical miles.
double proteus_ScalarConv_m2nm(double m);

// Converts nautical miles to metres.
double proteus_ScalarConv_nm2m(double m);

// Converts metres to geographical degrees of latitude, at the provided latitude.
double proteus_ScalarConv_m2dlat(double m, double lat);

// Converts metres to geographical degrees of longitude, at the provided latitude.
double proteus_ScalarConv_m2dlon(double m, double lat);

// Converts degrees to radians.
double proteus_ScalarConv_deg2rad(double deg);

// Converts radians to degrees.
double proteus_ScalarConv_rad2deg(double rad);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _proteus_ScalarConv_h_
