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

#ifndef __ID_MM__
#include "id_mm.h"
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


#define SC_INDEX	0x3C4
#define SC_RESET	0
#define SC_CLOCK	1
#define SC_MAPMASK	2
#define SC_CHARMAP	3
#define SC_MEMMODE	4

#define CRTC_INDEX	0x3D4
#define CRTC_H_TOTAL	0
#define CRTC_H_DISPEND	1
#define CRTC_H_BLANK	2
#define CRTC_H_ENDBLANK	3
#define CRTC_H_RETRACE	4
#define CRTC_H_ENDRETRACE 5
#define CRTC_V_TOTAL	6
#define CRTC_OVERFLOW	7
#define CRTC_ROWSCAN	8
#define CRTC_MAXSCANLINE 9
#define CRTC_CURSORSTART 10
#define CRTC_CURSOREND	11
#define CRTC_STARTHIGH	12
#define CRTC_STARTLOW	13
#define CRTC_CURSORHIGH	14
#define CRTC_CURSORLOW	15
#define CRTC_V_RETRACE	16
#define CRTC_V_ENDRETRACE 17
#define CRTC_V_DISPEND	18
#define CRTC_OFFSET	19
#define CRTC_UNDERLINE	20
#define CRTC_V_BLANK	21
#define CRTC_V_ENDBLANK	22
#define CRTC_MODE	23
#define CRTC_LINECOMPARE 24


#define GC_INDEX	0x3CE
#define GC_SETRESET	0
#define GC_ENABLESETRESET 1
#define GC_COLORCOMPARE	2
#define GC_DATAROTATE	3
#define GC_READMAP	4
#define GC_MODE		5
#define GC_MISCELLANEOUS 6
#define GC_COLORDONTCARE 7
#define GC_BITMASK	8

#define ATR_INDEX	0x3c0
#define ATR_MODE	16
#define ATR_OVERSCAN	17
#define ATR_COLORPLANEENABLE 18
#define ATR_PELPAN	19
#define ATR_COLORSELECT	20

#define	STATUS_REGISTER_1    0x3da

//===========================================================================

typedef struct
{
  short int	width,
	height,
	orgx,orgy,
	xl,yl,xh,yh,
	shifts;
} spritetabletype;

typedef	struct
{
	unsigned short	sourceoffset[MAXSHIFTS];
	unsigned short	planesize[MAXSHIFTS];
	unsigned short	width[MAXSHIFTS];
	byte		data[];
} spritetype;		// the memptr for each sprite points to this

typedef struct
{
	short int width,height;
} pictabletype;


typedef struct
{
	short int height;
	short int location[256];
	char width[256];
} fontstruct;


//===========================================================================

extern	boolean		screenfaded;
extern	char 		colors[7][17];	// pallets for fades

extern	pictabletype	_seg *pictable;
extern	pictabletype	_seg *picmtable;
extern	spritetabletype _seg *spritetable;

extern	unsigned	fontnumber;		// 0 based font number for drawing
extern	int			px,py;
extern	byte		pdrawmode,fontcolor;

extern	int			bordercolor;

//
// asm globals
//

extern	unsigned	*shifttabletable[8];
extern	unsigned	bufferwidth,bufferheight,screenspot;	// used by font drawing stuff



//===========================================================================


void	VW_Startup (void);
void	VW_Shutdown (void);

void 	VW_SetLineWidth(int width);
void 	VW_SetSplitScreen(int width);
void 	VW_SetScreen (unsigned CRTC, unsigned pelpan);

void	VW_SetScreenMode (int grmode);
void	VW_ClearVideo (int color);
void	VW_WaitVBL (int number);

void	VW_ColorBorder (int color);
void 	VW_SetPalette(byte *palette);
void	VW_SetDefaultColors(void);
void	VW_FadeOut(void);
void	VW_FadeIn(void);
void	VW_FadeUp(void);
void	VW_FadeDown(void);

void	VW_SetAtrReg (int reg, int value);

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
#define VW_DrawTile16(x,y,t) \
	VW_MemToScreen(grsegs[STARTTILE16+t],8*x,y,2,16)
#define VW_DrawTile16M(x,y,t) \
	VW_MaskBlock((byte*)grsegs[STARTTILE16M]+(t)*160,8*x,y,2,16,32)

void VW_DrawPic(unsigned x, unsigned y, unsigned chunknum);
void VW_DrawMPic(unsigned x, unsigned y, unsigned chunknum);
void VW_ClipDrawMPic(unsigned x, int y, unsigned chunknum);

//
// pixel addressable routines
//
void	VW_MeasurePropString (char far *string, word *width, word *height);
void	VW_MeasureMPropString  (char far *string, word *width, word *height);

void VW_DrawPropString (char far *string);
void VW_DrawMPropString (char far *string);
void VW_Plot(unsigned x, unsigned y, unsigned color);
void VW_Hlin(unsigned xl, unsigned xh, unsigned y, unsigned color);
void VW_Vlin(unsigned yl, unsigned yh, unsigned x, unsigned color);
void VW_Bar (unsigned x, unsigned y, unsigned width, unsigned height,
	unsigned color);

//===========================================================================

void VW_UpdateScreen (void);

//
// cursor
//

void VW_ShowCursor (void);
void VW_HideCursor (void);
void VW_MoveCursor (int x, int y);
void VW_SetCursor (int spritenum);
void VW_FreeCursor (void);

//
// mode independant routines
// coordinates in pixels, rounded to best screen res
// regions marked in double buffer
//

void VWB_DrawTile8 (int x, int y, int tile);
void VWB_DrawTile8M (int x, int y, int tile);
void VWB_DrawTile16 (int x, int y, int tile);
void VWB_DrawTile16M (int x, int y, int tile);
void VWB_DrawPic (int x, int y, int chunknum);
void VWB_DrawMPic(int x, int y, int chunknum);
void VWB_Bar (int x, int y, int width, int height, int color);

void VWB_DrawPropString	 (char far *string);
void VWB_DrawMPropString (char far *string);
void VWB_DrawSprite (int x, int y, int chunknum);
void VWB_Plot (int x, int y, int color);
void VWB_Hlin (int x1, int x2, int y, int color);
void VWB_Vlin (int y1, int y2, int x, int color);

//===========================================================================
