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

// ID_VW.C

#include "id_heads.h"

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

#define VIEWWIDTH		40
#define PIXTOBLOCK		4		// 16 pixels to an update block

/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

unsigned	fontnumber;		// 0 based font number for drawing

boolean		screenfaded;

pictabletype	_seg *pictable;
pictabletype	_seg *picmtable;
spritetabletype _seg *spritetable;

int			bordercolor;

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

void	VWL_MeasureString (char far *string, word *width, word *height,
		fontstruct _seg *font);
void 	VWL_DrawCursor (void);
void 	VWL_EraseCursor (void);
void 	VWL_DBSetup (void);
void	VWL_UpdateScreenBlocks (void);


int			bordercolor;
int			cursorvisible;
int			cursornumber,cursorwidth,cursorheight,cursorx,cursory;
memptr		cursorsave;
unsigned	cursorspot;

//===========================================================================


/*
=======================
=
= VW_Startup
=
=======================
*/

void	VW_Startup (void)
{
	cursorvisible = 0;
}

//===========================================================================


/*
=============================================================================

							SCREEN FADES

=============================================================================
*/

char colors[7][17]=
{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
 {0,0,0,0,0,0,0,0,0,1,2,3,4,5,6,7,0},
 {0,0,0,0,0,0,0,0,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0},
 {0,1,2,3,4,5,6,7,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0},
 {0,1,2,3,4,5,6,7,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0},
 {0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f}};


void VW_SetPalette(byte *palette)
{
	byte	p;
	word	i;

	for (i = 0;i < 15;i++)
	{
		p = palette[i];
		colors[0][i] = 0;
		colors[1][i] = (p > 0x10)? (p & 0x0f) : 0;
		colors[2][i] = (p > 0x10)? p : 0;
		colors[3][i] = p;
		colors[4][i] = (p > 0x10)? 0x1f : p;
		colors[5][i] = 0x1f;
	}
}


void VW_FadeOut(void)
{
/*
#if GRMODE == EGAGR
	int i;

	for (i=3;i>=0;i--)
	{
	  colors[i][16] = bordercolor;
	  _ES=FP_SEG(&colors[i]);
	  _DX=FP_OFF(&colors[i]);
	  _AX=0x1002;
	  geninterrupt(0x10);
	  VW_WaitVBL(6);
	}
	screenfaded = true;
#endif
*/
}


void VW_FadeIn(void)
{
/*
#if GRMODE == EGAGR
	int i;

	for (i=0;i<4;i++)
	{
	  colors[i][16] = bordercolor;
	  _ES=FP_SEG(&colors[i]);
	  _DX=FP_OFF(&colors[i]);
	  _AX=0x1002;
	  geninterrupt(0x10);
	  VW_WaitVBL(6);
	}
	screenfaded = false;
#endif
*/
}

void VW_FadeUp(void)
{
/*
#if GRMODE == EGAGR
	int i;

	for (i=3;i<6;i++)
	{
	  colors[i][16] = bordercolor;
	  _ES=FP_SEG(&colors[i]);
	  _DX=FP_OFF(&colors[i]);
	  _AX=0x1002;
	  geninterrupt(0x10);
	  VW_WaitVBL(6);
	}
	screenfaded = true;
#endif
*/
}

void VW_FadeDown(void)
{
/*
#if GRMODE == EGAGR
	int i;

	for (i=5;i>2;i--)
	{
	  colors[i][16] = bordercolor;
	  _ES=FP_SEG(&colors[i]);
	  _DX=FP_OFF(&colors[i]);
	  _AX=0x1002;
	  geninterrupt(0x10);
	  VW_WaitVBL(6);
	}
	screenfaded = false;
#endif
*/
}


//===========================================================================

/*
====================
=
= VW_SetSplitScreen
=
====================
*/

void VW_SetSplitScreen (int linenum)
{
}

//===========================================================================


//===========================================================================

#if NUMPICS>0

/*
====================
=
= VW_DrawPic
=
= X in bytes, y in pixels, chunknum is the #defined picnum
=
====================
*/

void VW_DrawPic(unsigned x, unsigned y, unsigned chunknum)
{
	int	picnum = chunknum - STARTPICS;
	memptr source;
	unsigned width,height;

	source = grsegs[chunknum];
	width = pictable[picnum].width;
	height = pictable[picnum].height;

	VW_MemToScreen(source,8*x,y,width,height);
}


#endif

#if NUMPICM>0

/*
====================
=
= VW_DrawMPic
=
= X in bytes, y in pixels, chunknum is the #defined picnum
=
====================
*/

void VW_DrawMPic(unsigned x, unsigned y, unsigned chunknum)
{
//	NYI("VW_DrawMPic");

	int	picnum = chunknum - STARTPICM;
	memptr source;
	unsigned dest,width,height;

	source = grsegs[chunknum];
	width = picmtable[picnum].width;
	height = picmtable[picnum].height;

	VW_MaskBlock(source,8*x,y,8*width,height,width*height);
}

void VW_ClipDrawMPic(unsigned x, int y, unsigned chunknum)
{
	NYI("VW_ClipDrawMPic");
/*
	int	picnum = chunknum - STARTPICM;
	memptr source;
	unsigned dest,width,ofs,plane;
	int		height;

	source = grsegs[chunknum];
	width = picmtable[picnum].width;
	height = picmtable[picnum].height;
	plane = width*height;

	ofs = 0;
	if (y<0)
	{
		ofs= -y*width;
		height+=y;
		y=0;
	}
	else if (y+height>216)
	{
		height-=(y-216);
	}
	dest = ylookup[y]+x+bufferofs;
	if (height<1)
		return;

	VW_MaskBlock(source,ofs,dest,width,height,plane);
*/
}


#endif

//==========================================================================

/*
==================
=
= VW_MeasureString
=
==================
*/

#if NUMFONT+NUMFONTM>0
void
VWL_MeasureString (char far *string, word *width, word *height, fontstruct _seg *font)
{
	*height = font->height;
	for (*width = 0;*string;string++)
		*width += font->width[*((byte far *)string)];	// proportional width
}

void	VW_MeasurePropString (char far *string, word *width, word *height)
{
	VWL_MeasureString(string,width,height,(fontstruct _seg *)grsegs[STARTFONT+fontnumber]);
}

void	VW_MeasureMPropString  (char far *string, word *width, word *height)
{
	VWL_MeasureString(string,width,height,(fontstruct _seg *)grsegs[STARTFONTM+fontnumber]);
}


#endif


/*
=============================================================================

					   CURSOR ROUTINES

These only work in the context of the double buffered update routines

=============================================================================
*/

/*
====================
=
= VWL_DrawCursor
=
= Background saves, then draws the cursor at cursorspot
=
====================
*/

void VWL_DrawCursor (void)
{
	NYI("VWL_DrawCursor");

//	VW_ScreenToMem(cursorx,cursory,cursorsave,cursorwidth,cursorheight);
	VWB_DrawSprite(cursorx,cursory,cursornumber);

}


//==========================================================================


/*
====================
=
= VWL_EraseCursor
=
====================
*/

void VWL_EraseCursor (void)
{
	VW_MemToScreen(cursorsave,8*cursorx,cursory,cursorwidth,cursorheight);
}


//==========================================================================


/*
====================
=
= VW_ShowCursor
=
====================
*/

void VW_ShowCursor (void)
{
	cursorvisible++;
}


//==========================================================================

/*
====================
=
= VW_HideCursor
=
====================
*/

void VW_HideCursor (void)
{
	cursorvisible--;
}

//==========================================================================

/*
====================
=
= VW_MoveCursor
=
====================
*/
#define MAXCURSORX	(319-24)
#define MAXCURSORY	(199-24)

void VW_MoveCursor (int x, int y)
{
	if (x>MAXCURSORX)
		x=MAXCURSORX;
	if (y>MAXCURSORY)
		y=MAXCURSORY;			// catacombs hack to keep cursor on screen

	cursorx = x;
	cursory = y;
}

//==========================================================================

/*
====================
=
= VW_SetCursor
=
= Load in a sprite to be used as a cursor, and allocate background save space
=
====================
*/

void VW_SetCursor (int spritenum)
{
	VW_FreeCursor ();

	cursornumber = spritenum;

	CA_CacheGrChunk (spritenum);
	MM_SetLock (&grsegs[spritenum],true);

	cursorwidth = spritetable[spritenum-STARTSPRITES].width+1;
	cursorheight = spritetable[spritenum-STARTSPRITES].height;

	MM_GetPtr (&cursorsave,cursorwidth*cursorheight*5);
	MM_SetLock (&cursorsave,true);
}


/*
====================
=
= VW_FreeCursor
=
= Frees the memory used by the cursor and its background save
=
====================
*/

void VW_FreeCursor (void)
{
	if (cursornumber)
	{
		MM_SetLock (&grsegs[cursornumber],false);
		MM_SetPurge (&grsegs[cursornumber],3);
		MM_SetLock (&cursorsave,false);
		MM_FreePtr (&cursorsave);
		cursornumber = 0;
	}
}


/*
=============================================================================

				Double buffer management routines

=============================================================================
*/



/*
===========================
=
= VW_UpdateScreen
=
= Updates any changed areas of the double buffer and displays the cursor
=
===========================
*/

void VW_UpdateScreen (void)
{
	if (cursorvisible>0)
		VWL_DrawCursor();

	BE_FlipBuffer();

	if (cursorvisible>0)
		VWL_EraseCursor();
}


void VWB_DrawTile8 (int x, int y, int tile)
{
	VW_DrawTile8 (x/SCREENXDIV,y,tile);
}

void VWB_DrawTile8M (int x, int y, int tile)
{
	int xb;

	xb = x/SCREENXDIV; 			// use intermediate because VW_DT8M is macro
	VW_DrawTile8M (xb,y,tile);
}

void VWB_DrawTile16 (int x, int y, int tile)
{
	VW_DrawTile16 (x,y,tile);
}

void VWB_DrawTile16M (int x, int y, int tile)
{
	int xb;

	xb = x/SCREENXDIV;		// use intermediate because VW_DT16M is macro
	VW_DrawTile16M (xb,y,tile);
}

#if NUMPICS
void VWB_DrawPic (int x, int y, int chunknum)
{
// mostly copied from drawpic
	int	picnum = chunknum - STARTPICS;
	memptr source;
	unsigned dest,width,height;

	x/= SCREENXDIV;


	source = grsegs[chunknum];
	width = pictable[picnum].width;
	height = pictable[picnum].height;

	VW_MemToScreen(source,8*x,y,width,height);
}
#endif

#if NUMPICM>0
void VWB_DrawMPic(int x, int y, int chunknum)
{
// mostly copied from drawmpic
	int	picnum = chunknum - STARTPICM;
	memptr source;
	unsigned dest,width,height;

	x/=SCREENXDIV;

	source = grsegs[chunknum];
	width = picmtable[picnum].width;
	height = picmtable[picnum].height;

	VW_MaskBlock(source,8*x,y,width,height,width*height);
}
#endif


void VWB_Bar (int x, int y, int width, int height, int color)
{
	VW_Bar (x,y,width,height,color);
}


#if NUMFONT
void VWB_DrawPropString	 (char far *string)
{
	int x,y;
	x = px;
	y = py;
	VW_DrawPropString (string);
}
#endif


#if NUMFONTM
void VWB_DrawMPropString (char far *string)
{
	int x,y;
	x = px;
	y = py;
	VW_DrawMPropString (string);
}
#endif

#if NUMSPRITES
void VWB_DrawSprite(int x, int y, int chunknum)
{
	spritetabletype far *spr;
	spritetype _seg	*block;
	unsigned	shift,width,height;

	spr = &spritetable[chunknum-STARTSPRITES];
	block = (spritetype _seg *)grsegs[chunknum];

	y+=spr->orgy>>G_P_SHIFT;
	x+=spr->orgx>>G_P_SHIFT;



	shift = 0;
	width = block->width[shift];
	height = spr->height;

	VW_MaskBlock ((byte*)block+block->sourceoffset[shift],(x&0xFFF8)+((x&7)/2*2),y,width,height,block->planesize[shift]);
}
#endif

void VWB_Plot (int x, int y, int color)
{
	VW_Plot(x,y,color);
}

void VWB_Hlin (int x1, int x2, int y, int color)
{
	VW_Hlin(x1,x2,y,color);
}

void VWB_Vlin (int y1, int y2, int x, int color)
{
	VW_Vlin(y1,y2,x,color);
}


//===========================================================================
