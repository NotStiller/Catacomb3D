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

pictabletype	*pictable;
pictabletype	*picmtable;
spritetabletype *spritetable;

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

void	VWL_MeasureString (char *string, word *width, word *height,
		fontstruct *font);

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
}

//===========================================================================

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
	printf("NYI: %s in %s:%i\n", "VW_ClipDrawMPic", __FILE__, __LINE__);
	assert(0);
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
VWL_MeasureString (char *string, word *width, word *height, fontstruct *font)
{
	byte *f = (byte*)font;

	*height = *(int16_t*)f;
	for (*width = 0;*string;string++)
		*width += *(byte*)(f+2+512+*(byte *)string);

}

void	VW_MeasurePropString (char *string, word *width, word *height)
{
	VWL_MeasureString(string,width,height,(fontstruct *)grsegs[STARTFONT+fontnumber]);
}

void	VW_MeasureMPropString  (char *string, word *width, word *height)
{
	VWL_MeasureString(string,width,height,(fontstruct *)grsegs[STARTFONTM+fontnumber]);
}


#endif



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
	SP_FlipBuffer();
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


void VWB_Bar (int x, int y, int width, int height, int color)
{
	VW_Bar (x,y,width,height,color);
}


void VWB_DrawPropString	 (char *string)
{
	int x,y;
	x = px;
	y = py;
	VW_DrawPropString (string);
}

void VWB_DrawSprite(int x, int y, int chunknum)
{
	spritetabletype *spr;
	spritetype	*block;
	unsigned	shift,width,height;

	spr = &spritetable[chunknum-STARTSPRITES];
	block = (spritetype *)grsegs[chunknum];

	y+=spr->orgy>>G_P_SHIFT;
	x+=spr->orgx>>G_P_SHIFT;



	shift = 0;
	width = block->width[shift];
	height = spr->height;

	VW_MaskBlock ((byte*)block+block->sourceoffset[shift],(x&0xFFF8)+((x&7)/2*2),y,width,height,block->planesize[shift]);
}


//===========================================================================
