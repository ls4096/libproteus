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

#include "proteus_internal.h"

#include "proteus/proteus.h"

#define PROTEUS_VERSION_MAJOR 0
#define PROTEUS_VERSION_MINOR 4
#define PROTEUS_VERSION_PATCH 3

#define STRX(x) #x
#define STR(x) STRX(x)
#define PROTEUS_VERSION_STRING STR(PROTEUS_VERSION_MAJOR) "." STR(PROTEUS_VERSION_MINOR) "." STR(PROTEUS_VERSION_PATCH)

PROTEUS_API int proteus_getVersionMajor()
{
	return PROTEUS_VERSION_MAJOR;
}

PROTEUS_API int proteus_getVersionMinor()
{
	return PROTEUS_VERSION_MINOR;
}

PROTEUS_API int proteus_getVersionPatch()
{
	return PROTEUS_VERSION_PATCH;
}

PROTEUS_API const char* proteus_getVersionString()
{
	return PROTEUS_VERSION_STRING;
}
