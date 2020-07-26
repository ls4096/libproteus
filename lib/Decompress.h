#ifndef _Decompress_h_
#define _Decompress_h_

#include <stdint.h>
#include <unistd.h>

/**
 * Decompresses gzip (and possibly other types of) compressed data.
 *
 * WARNING: This function assumes that the caller knows the maximum size of the
 *          decompressed data!
 *
 * Parameters
 * 	out [out]: the buffer where the decompressed data will be stored
 * 	outlen [in]: the size of the "out" buffer available for writing
 * 	in [in]: the compressed data input buffer
 * 	inlen [in]: the size of the compressed data in the "in" buffer
 *
 * Returns
 * 	0, on success
 * 	any other value, on failure
 */
int Decompress_inflate(uint8_t* out, size_t outlen, const char* in, size_t inlen);

#endif // _Decompress_h_
