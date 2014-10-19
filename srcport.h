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

#ifndef COMPAT_H
#define COMPAT_H

#include <stdint.h>
#include <assert.h>
#include <stdio.h>

#include "sp_audio.h"
#include "sp_data.h"
#include "sp_graph.h"
#include "sp_main.h"

// the following defs are mostly accessed by the original sources
#define CASTAT(type, where) (*(type*)&(where))
#define RANDOM(max) (rand()%(max))

typedef void* memptr;

#define NUMMAPS		30
#define MAPPLANES	3
extern	void **grsegs;

typedef	struct
{
	int32_t planestart[3];
	uint16_t planelength[3];
	uint16_t width,height;
	char name[16];

	char *texts[27];
	uint16_t *rawplanes[3];
	long rawplaneslength[3];
} maptype;

extern int loadedmap;
extern maptype *curmap;
extern maptype mapheaderseg[30];


//c3_draw and c4_draw
#define PI	3.141592657
#define ANGLEQUAD	(ANGLES/4)
#define FINEANGLES	3600
#define MINRATIO	16


 
#endif

