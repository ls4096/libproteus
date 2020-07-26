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
#include <zlib.h>

#include "Decompress.h"
#include "ErrLog.h"

#define ERRLOG_ID "Decompress"


int Decompress_inflate(uint8_t* out, size_t outlen, const char* in, size_t inlen)
{
	z_stream zs;

	zs.next_in = (unsigned char*) in;
	zs.avail_in = inlen;
	zs.next_out = out;
	zs.avail_out = outlen;

	zs.zalloc = Z_NULL;
	zs.zfree = Z_NULL;
	zs.opaque = 0;

	int rc = 0;

	int zrc = inflateInit2(&zs, 16 + MAX_WBITS);
	if (Z_OK != zrc)
	{
		ERRLOG1("Failed to inflate init! zlib rc=%d", zrc);
		rc = -1;
		goto done;
	}

	if (Z_STREAM_END != (zrc = inflate(&zs, Z_FINISH)))
	{
		ERRLOG1("Failed to inflate! zlib rc=%d", zrc);
		rc = -2;
		goto zend;
	}

	ERRLOG2("Done: in=%lu, out=%lu", zs.total_in, zs.total_out);

zend:
	if (Z_OK != (zrc = inflateEnd(&zs)))
	{
		ERRLOG1("Failed to inflate end! zlib rc=%d", zrc);
	}

done:
	return rc;
}
