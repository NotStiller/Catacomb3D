/* Catacomb 3-D Source Code
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

// ID_GLOB.H

#include "srcport.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __ID_GLOB__

//--------------------------------------------------------------------------

#define	EXTENSION	"C3D"

extern	unsigned char introscn[];

#include "gfxe_c3d.h"
#include "audioc3d.h"

//--------------------------------------------------------------------------

#define CAT3D

#define GREXT	"EGA"

//
//	ID Engine
//	Types.h - Generic types, #defines, etc.
//	v1.0d1
//

#ifndef	__TYPES__
#define	__TYPES__

// typedef	enum	{false,true}	boolean;
#define false 0
#define true 1
typedef	uint8_t	boolean;
typedef	uint8_t	byte;
typedef	uint16_t			word;
typedef	uint32_t		longword;
typedef	byte *					Ptr;

typedef	struct
		{
			int	x,y;
		} Point;
typedef	struct
		{
			Point	ul,lr;
		} Rect;

#define	nil	((void *)0)

#endif

#include "id_ca.h"
#include "id_vw.h"
#include "id_in.h"
#include "id_sd.h"
#include "id_us.h"


void	Quit (char *error);		// defined in user program

//
// replacing refresh manager with custom routines
//

#define	PORTTILESWIDE		21      // all drawing takes place inside a
#define	PORTTILESHIGH		14		// non displayed port of this size

#define UPDATEWIDE			(PORTTILESWIDE+1)
#define UPDATEHIGH			PORTTILESHIGH

#define	MAXTICS				6
#define DEMOTICS			3

#define	UPDATETERMINATE	0x0301

extern	unsigned	mapwidth,mapheight,tics;
extern	boolean		compatability;

extern	byte		*updateptr;
extern	unsigned	uwidthtable[UPDATEHIGH];
extern	unsigned	blockstarts[UPDATEWIDE*UPDATEHIGH];

