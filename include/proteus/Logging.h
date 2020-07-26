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

#ifndef _proteus_Logging_h_
#define _proteus_Logging_h_

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


/**
 * Specifies if and where to write library logging output
 *
 * Parameters
 * 	logFd [in]: the file descriptor to write logging output to; set to -1 to disable logging
 */
void proteus_Logging_setOutputFd(int logFd);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _proteus_Logging_h_
