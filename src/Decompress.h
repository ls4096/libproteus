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
