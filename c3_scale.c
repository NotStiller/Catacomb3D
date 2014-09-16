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

// C3_SCALE.C

#include "c3_def.h"
#pragma hdrstop

//const	unsigned	viewheight = 144;
const	unsigned	screenbwide = 40;
const	byte		BACKGROUNDPIX	=   5;

unsigned short		shapesize[MAXSCALE+1];
t_compscale *scaledirectory[MAXSCALE+1];
t_compshape *shapedirectory[NUMSCALEPICS];
memptr			walldirectory[NUMSCALEWALLS];

/*
===========================
=
= DeplanePic
=
= Takes a raw bit map of width bytes by height and creates a scaleable shape
=
= Returns the length of the shape in bytes
=
= Fills in spotvis (a convenient 64*64 array) with the color values
=
===========================
*/

void DeplanePic (int picnum)
{
	byte		*plane0,*plane1,*plane2,*plane3;
	byte		by0,by1,by2,by3;
	unsigned	x,y,b,color,shift,width,height;
	byte		*dest;

//
// convert ega pixels to byte color values in a temp buffer
//
	width = pictable[picnum-STARTPICS].width;
	height = pictable[picnum-STARTPICS].height;

	if (width>64 || height!=64)
		Quit ("DePlanePic: Bad size shape");

	memset (spotvis,BACKGROUNDPIX,sizeof(spotvis));

	plane0 = (byte *)grsegs[picnum];
	plane1 = plane0 + width*height;
	plane2 = plane1 + width*height;
	plane3 = plane2 + width*height;

	for (y=0;y<height;y++)
	{
		dest = &spotvis[y][0];
		for (x=0;x<width;x++)
		{
			by0 = *plane0++;
			by1 = *plane1++;
			by2 = *plane2++;
			by3 = *plane3++;

			for (b=0;b<8;b++)
			{
				color = ((by3&1)<<3) + ((by2&1)<<2) + ((by1&1)<<1) + (by0&1);
				by3 >>= 1;
				by2 >>= 1;
				by1 >>= 1;
				by0 >>= 1;

				dest[7-b] = color;
			}	// B
			dest += 8;
		}		// X
	}			// Y
}




/*
========================
=
= BuildCompScale
=
= Builds a compiled scaler object that will scale a 64 tall object to
= the given height (centered vertically on the screen)
=
= height should be even
=
= Call with
= ---------
= DS:SI		Source for scale
= ES:DI		Dest for scale
=
= Calling the compiled scaler only destroys AL
=
========================
*/

unsigned BuildCompScale (int height, memptr *finalspot)
{
	t_compscale 	*work;

	short int			i;
	long		fix,step;
	unsigned short	src,totalscaled,totalsize;
	short int			startpix,endpix,toppix;


	MM_GetPtr ((memptr*)&work,20000);

	step = ((long)height<<16) / 64;
	toppix = (viewheight-height)/2;
	fix = 0;

	for (src=0;src<=64;src++)
	{
		startpix = fix>>16;
		fix += step;
		endpix = fix>>16;

		work->start[src] = startpix;
		if (endpix>startpix)
			work->width[src] = endpix-startpix;
		else
			work->width[src] = 0;


//
// startpix and endpix tell where to draw the vertical span that this texel covers
//
		startpix+=toppix;
		endpix+=toppix;

		if (endpix >= VIEWHEIGHT) {
			endpix = VIEWHEIGHT;
		} else if (startpix < 0) {
			startpix++;
		}
		
		if (startpix == endpix || endpix < 0 || startpix >= VIEWHEIGHT || src == 64) {
			startpix = endpix = 0;
		}

		work->starty[src] = startpix;
		work->endy[src] = endpix;
	}


	totalsize = sizeof(t_compscale);
	MM_GetPtr (finalspot,totalsize);
	memcpy ((byte *)(*finalspot),(byte*)work,totalsize);
	MM_FreePtr ((memptr*)&work);

	return totalsize;
}




/*
========================
=
= BuildCompShape
=
= typedef struct
= {
=	unsigned	width;
=	unsigned	codeofs[64];
= }	t_compshape;
=
= Width is the number of compiled line draws in the shape.  The shape
= drawing code will assume that the midpoint of the shape is in the
= middle of the width.
=
= The non background pixel data will start at codeofs[width], so codeofs
= greater than width will be invalid.
=
= Each code offset will draw one vertical line of the shape, consisting
= of 0 or more segments of scaled pixels.
=
= The scaled shapes use ss:0-4 as a scratch variable for the far call to
= the compiled scaler, so zero it back out after the shape is scaled, or
= a "null pointer assignment" will result upon termination.
=
= Setup for a call to a compiled shape
= -----------------------------------
= ax	toast
= bx	toast
= cx	toast
= dx	segment of compiled shape
= si	toast
= di	byte at top of view area to draw the line in
= bp	0
= ss:2 and ds  the segment of the compiled scaler to use
= es	screenseg
=
= Upon return, ds IS NOT SET to the data segment.  Do:
=	mov	ax,ss
=	mov	ds,ax
=
=
= GC_BITMASK	set to the pixels to be drawn in the row of bytes under DI
= GC_MODE		read mode 1, write mode 2
= GC_COLORDONTCARE  set to 0, so all reads from video memory return 0xff
=
=
= Code generated for each segment
= -------------------------------
=	mov	bx,[(segend+1)*2]
=	mov	cx,[bx]
=	mov	[BYTE PTR bx],0xc8		// far return
=	mov	ax,[segstart*2]
=	mov	[ss:0],ax				// entry point into the compiled scaler
=	mov	ds,dx                   // (mov ds,cs) the data is after the compiled code
=	mov	si,ofs data
=	call	[bp]				// scale some pixels
=	mov	ds,[bp+2]
=	mov	[bx],cx					// un patch return
=
= Code generated after all segments on a line
= -------------------------------------------
=	retf
=
========================
*/

unsigned BuildCompShape (t_compshape **finalspot)
{
	t_compshape *work;
	byte		*code;
	int			firstline,lastline,x,y;
	unsigned	firstpix,lastpix,width;
	unsigned	totalsize;
	byte 		*pixelofs;
	unsigned	buff;


	MM_GetPtr ((memptr)&work,sizeof(unsigned)+sizeof(byte*)*64+64*64+20000);

//
// find the width of the shape
//
	firstline = -1;
	x=0;
	do
	{
		for (y=0;y<64;y++)
			if (spotvis[y][x] != BACKGROUNDPIX)
			{
				firstline = x;
				break;
			}
		if (++x == 64)
			Quit ("BuildCompShape: No shape data!");
	} while (firstline == -1);

	lastline = -1;
	x=63;
	do
	{
		for (y=0;y<64;y++)
			if (spotvis[y][x] != BACKGROUNDPIX)
			{
				lastline = x;
				break;
			}
		x--;
	} while (lastline == -1);

	width = lastline-firstline+1;
	work->width = width;

	memset(work->lineofs, 0, sizeof(int)*64);
//
// copy all non background pixels to the work space
//
	boolean span=false;
	int lineofs = (char*)&work->lineofs[65] - (char*)work;
	int spanofs = 0;
	code = (byte*)work;
	for (x=firstline;x<=lastline;x++) {
		work->lineofs[x-firstline] = lineofs;
		for (y=0;y<64;y++) {
			if (spotvis[y][x] != BACKGROUNDPIX) {
				if (!span) {
					span = true;
					spanofs = lineofs;
					code[lineofs++] = 0;
					code[lineofs++] = y;
				}
				code[lineofs++] = spotvis[y][x];
			} else if (span) {
				span = false;
				assert(lineofs>spanofs+2);
				code[spanofs] = lineofs-2-spanofs;
			}
		}
		if (span) {
			span = false;
			assert(lineofs>spanofs+2);
			code[spanofs] = lineofs-2-spanofs;
		}
		code[lineofs++] = 0; // end of vertical line marker
	}

//
// copy the final shape to a properly sized buffer
//
	totalsize = lineofs;
	MM_GetPtr ((memptr *)finalspot,totalsize);
	memcpy ((byte*)(*finalspot),(byte*)work,totalsize);
	MM_FreePtr ((memptr*)&work);

	return totalsize;


	return 0;
}



/*
=======================
=
= ScaleShape
=
= Draws a compiled shape at [scale] pixels high
=
= Setup for call
= --------------
= GC_MODE			read mode 1, write mode 2
= GC_COLORDONTCARE  set to 0, so all reads from video memory return 0xff
= GC_INDEX			pointing at GC_BITMASK
=
=======================
*/

static	long		longtemp;

void ScaleShape (int xcenter, t_compshape *compshape, unsigned height)
{
	t_compscale *comptable;
	unsigned short	width,scalewidth;
	short int		x,pixel,lastpixel,pixwidth,min;
	unsigned short	*widthptr;
	int codehandle;
	unsigned short	badcodeptr;
	short int		rightclip;

	if (!compshape)
		Quit ("ScaleShape: NULL compshape ptr!");

	int scale = (height+1)/2;
	if (!scale)
		return;								// too far away
	if (scale>MAXSCALE)
		scale = MAXSCALE;
	comptable = scaledirectory[scale];

	width = compshape->width;
	scalewidth = comptable->start[width];

	pixel = xcenter - scalewidth/2;
	lastpixel = pixel+scalewidth-1;
	if (pixel >= VIEWWIDTH || lastpixel < 0)
		return;								// totally off screen

//
// scan backwards from the right edge until pixels are visable
// rightclip is the first NON VISABLE pixel
//
	if (lastpixel>=VIEWWIDTH-1)
		rightclip = VIEWWIDTH-1;
	else
		rightclip = lastpixel;

	if (zbuffer[rightclip]>scale)
	{
		if (pixel>0)
			min = pixel;
		else
			min = 0;
		do
		{
			if (--rightclip < min)
				return;							// totally covered or before 0
			if (zbuffer[rightclip]<=scale)
				break;
		} while (1);
	}
	rightclip++;

//
// scan from the left until it is on screen, leaving
// [pixel],[pixwidth],[codehandle],and [widthptr] set correctly
//

	codehandle=0;
	widthptr = &comptable->width[0];


	pixwidth = *widthptr;				// scaled width of this pixel
	while (!pixwidth)
	{
		pixwidth = *++widthptr;			// find the first visable pixel
		codehandle++;
	}

	if (pixel<0)
	{
		do
		{
			if (pixel+pixwidth>0)
			{
				pixwidth += pixel;
				pixel = 0;
				break;
			}
			do
			{
				pixwidth = *++widthptr;
				codehandle++;
			} while (!pixwidth);
			pixel+=pixwidth;
		} while (1);
	}

//
// scan until it is visable, leaving
// [pixel],[pixwidth],[codehandle],and [widthptr] set correctly
//
	do
	{
		if (zbuffer[pixel] <= scale)
			break;							// start drawing here
		pixel++;
		if (!--pixwidth)
		{
			do
			{
				pixwidth = *++widthptr;
				codehandle++;
			} while (!pixwidth);
		}
	} while (1);

	if (pixel+pixwidth>rightclip)
		pixwidth = rightclip-pixel;
//
// draw lines
//
	do		// while (1)
	{
	//
	// scale a vertical segment [pixwidth] pixels wide at [pixel]
	//

		assert(codehandle < 65);
		if (!compshape->lineofs[codehandle]) {
			continue;
		}

		long fix,step;
		short int startpix,endpix,toppix;
		unsigned short	srcy;


		step = ((long)height<<16) / 64;
		toppix = (viewheight-height)/2;
		fix = 0;
		srcy = 0;
		int spanLen;
		

		byte *code = (byte*)compshape+compshape->lineofs[codehandle];
		boolean nextX = pixwidth < 1;
		while (!nextX && (spanLen = *(code++)) != 0) {
			assert(srcy < 64);
			int y = *(code++);
			assert(srcy <= y);
			fix += step*(y-srcy);
			srcy = y;
			for (; spanLen > 0 && !nextX; spanLen--) {
				byte color = *(code++);
				startpix = fix>>16;
				fix += step;
				endpix = fix>>16;
				srcy++;
				if (endpix<=startpix) {
					continue;
				}
				startpix+=toppix;
				endpix+=toppix;

				if (startpix == endpix || endpix < 0 || startpix >= VIEWHEIGHT || srcy == 64) {
					nextX = true;
					continue;
				}

				for (;startpix<endpix;startpix++)
				{
					if (startpix >= VIEWHEIGHT)
						break;						// off the bottom of the view area
					if (startpix < 0)
						continue;					// not into the view area

					VW_Hlin(pixel, pixel+pixwidth-1, startpix, color);
				}
			}
		}


	//
	// advance to the next drawn line
	//

		if ( (pixel+=pixwidth) == rightclip )
		{
			return;							// all done!
		}

		do

		{
			pixwidth = *++widthptr;
			codehandle++;
		} while (!pixwidth);

		if (pixel+pixwidth > rightclip)
			pixwidth = rightclip-pixel;

	} while (1);
}

//
// bit mask tables for drawing scaled strips up to eight pixels wide
//

byte	bitmasks1[8][8] = {
{0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe,0xff},
{0x40,0x60,0x70,0x78,0x7c,0x7e,0x7f,0x7f},
{0x20,0x30,0x38,0x3c,0x3e,0x3f,0x3f,0x3f},
{0x10,0x18,0x1c,0x1e,0x1f,0x1f,0x1f,0x1f},
{0x8,0xc,0xe,0xf,0xf,0xf,0xf,0xf},
{0x4,0x6,0x7,0x7,0x7,0x7,0x7,0x7},
{0x2,0x3,0x3,0x3,0x3,0x3,0x3,0x3},
{0x1,0x1,0x1,0x1,0x1,0x1,0x1,0x1} };

byte	bitmasks2[8][8] = {
{0,0,0,0,0,0,0,0},
{0,0,0,0,0,0,0,0x80},
{0,0,0,0,0,0,0x80,0xc0},
{0,0,0,0,0,0x80,0xc0,0xe0},
{0,0,0,0,0x80,0xc0,0xe0,0xf0},
{0,0,0,0x80,0xc0,0xe0,0xf0,0xf8},
{0,0,0x80,0xc0,0xe0,0xf0,0xf8,0xfc},
{0,0x80,0xc0,0xe0,0xf0,0xf8,0xfc,0xfe} };






