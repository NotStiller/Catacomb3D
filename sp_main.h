/* Catacomb 3-D SDL Port
 * Copyright (C) 1993-2014 Flat Rock Software
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef SP_MAIN_H
#define SP_MAIN_H

void SP_Exit(void);
int SP_GameActive(void);
void SP_GameEnter(void);
void SP_GameLeave(void);
int  SP_StrafeOn(void);
void SP_PollEvents(void);
int SP_LastScan(void);
char SP_LastASCII(void);
int SP_Keyboard(int Key);
void SP_SetTimeCount(long Ticks);
long SP_TimeCount(void);


#endif

