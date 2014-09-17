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

// ID_VW.H

#ifndef __TYPES__
#include "id_types.h"
#endif

#ifndef __ID_GLOB__
#include "id_glob.h"
#endif

#define __ID_VW__

//===========================================================================

#define	G_P_SHIFT		4	// global >> ?? = pixels

#define	SCREENWIDTH		40
#define CHARWIDTH		1
#define TILEWIDTH		2
#define GRPLANES		4
#define BYTEPIXELS		8

#define VIRTUALHEIGHT	300
#define	VIRTUALWIDTH	512


#define	MAXSHIFTS		4

#define WHITE			15			// graphics mode independant colors
#define BLACK			0
#define FIRSTCOLOR		1
#define SECONDCOLOR		12
#define F_WHITE			0			// for XOR font drawing
#define F_BLACK			15
#define F_FIRSTCOLOR	14
#define F_SECONDCOLOR	3

#define SCREENXMASK		(~7)
#define SCREENXPLUS		(7)
#define SCREENXDIV		(8)

//===========================================================================

typedef struct
{
  int16_t width,
	height,
	orgx,orgy,
	xl,yl,xh,yh,
	shifts;
} spritetabletype;

typedef	struct
{ // the first three fields are filled in by software ! can be erased anyway...
	uint16_t	sourceoffset[MAXSHIFTS];
	uint16_t	planesize[MAXSHIFTS];
	uint16_t	width[MAXSHIFTS];
	byte		data[];
} spritetype;		// the memptr for each sprite points to this

typedef struct
{
	int16_t width,height;
} pictabletype;


typedef struct
{
	int16_t height;
	int16_t location[256];
	char width[256];
} fontstruct;


//===========================================================================

extern	pictabletype	*pictable;
extern	pictabletype	*picmtable;
extern	spritetabletype *spritetable;

extern	unsigned	fontnumber;		// 0 based font number for drawing
extern	int			px,py;
extern	byte		pdrawmode,fontcolor;

//===========================================================================


void	VW_Startup (void);
void	VW_Shutdown (void);

void	VW_ClearVideo (int color);
void	VW_WaitVBL (int number);

//
// block primitives
//

void VW_MaskBlock(byte *source,unsigned x, unsigned y, unsigned wide,unsigned height,unsigned planesize);
void VW_MemToScreen(byte *source,unsigned x, unsigned y,unsigned width,unsigned height);
void VW_MemToScreen2(byte *source,unsigned x, unsigned y,unsigned width,unsigned height, unsigned planesize);
void VW_ScreenToMem(unsigned x, unsigned y,memptr dest,unsigned width,unsigned height);
void VW_ScreenToScreen(unsigned sx, unsigned sy,unsigned dx, unsigned dy,unsigned width,unsigned height);


//
// block addressable routines
//

void VW_DrawTile8(unsigned x, unsigned y, unsigned tile);

#define VW_DrawTile8M(x,y,t) \
	VW_MaskBlock((byte*)grsegs[STARTTILE8M]+(t)*40,8*x,y,1,8,8)

void VW_DrawPic(unsigned x, unsigned y, unsigned chunknum);
void VW_DrawMPic(unsigned x, unsigned y, unsigned chunknum);
void VW_ClipDrawMPic(unsigned x, int y, unsigned chunknum);

//
// pixel addressable routines
//
void	VW_MeasurePropString (char *string, word *width, word *height);
void	VW_MeasureMPropString  (char *string, word *width, word *height);

void VW_DrawPropString (char *string);
void VW_DrawMPropString (char *string);
void VW_Plot(unsigned x, unsigned y, unsigned color);
void VW_Hlin(unsigned xl, unsigned xh, unsigned y, unsigned color);
void VW_Vlin(unsigned yl, unsigned yh, unsigned x, unsigned color);
void VW_Bar (unsigned x, unsigned y, unsigned width, unsigned height,
	unsigned color);

//===========================================================================

void VW_UpdateScreen (void);

//
// mode independant routines
// coordinates in pixels, rounded to best screen res
// regions marked in double buffer
//

void VWB_DrawTile8 (int x, int y, int tile);
void VWB_DrawTile8M (int x, int y, int tile);
void VWB_DrawPic (int x, int y, int chunknum);
void VWB_DrawMPic(int x, int y, int chunknum);
void VWB_Bar (int x, int y, int width, int height, int color);

void VWB_DrawPropString	 (char *string);
void VWB_DrawSprite (int x, int y, int chunknum);

//===========================================================================
