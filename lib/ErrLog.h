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

#ifndef _ErrLog_h_
#define _ErrLog_h_

#define ERRLOG(msg) proteus_internal_ErrLog_log(ERRLOG_ID, msg)
#define ERRLOG1(msg, a1) proteus_internal_ErrLog_log(ERRLOG_ID, msg, a1)
#define ERRLOG2(msg, a1, a2) proteus_internal_ErrLog_log(ERRLOG_ID, msg, a1, a2)
#define ERRLOG3(msg, a1, a2, a3) proteus_internal_ErrLog_log(ERRLOG_ID, msg, a1, a2, a3)
#define ERRLOG4(msg, a1, a2, a3, a4) proteus_internal_ErrLog_log(ERRLOG_ID, msg, a1, a2, a3, a4)
#define ERRLOG5(msg, a1, a2, a3, a4, a5) proteus_internal_ErrLog_log(ERRLOG_ID, msg, a1, a2, a3, a4, a5)

void proteus_internal_ErrLog_log(const char* id, const char* msg, ...);

#endif // _ErrLog_h_
