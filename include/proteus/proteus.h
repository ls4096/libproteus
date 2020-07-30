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

#ifndef _proteus_proteus_h_
#define _proteus_proteus_h_

#ifndef PROTEUS_API
#define PROTEUS_API
#endif // PROTEUS_API

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/**
 * Returns the library major version number, as an integer.
 */
PROTEUS_API int proteus_getVersionMajor();

/**
 * Returns the library minor version number, as an integer.
 */
PROTEUS_API int proteus_getVersionMinor();

/**
 * Returns the library patch version number, as an integer.
 */
PROTEUS_API int proteus_getVersionPatch();

/**
 * Returns the complete library version number, as a string.
 */
PROTEUS_API const char* proteus_getVersionString();


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _proteus_proteus_h_
