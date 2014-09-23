/* Catacomb 3-D SDL Port
 * Copyright (C) 2014 twitter.com/NotStiller
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

#include "srcport.h"
#include <SDL/SDL.h>
#include "c3_def.h"

// Change this to get a bigger window.
static const int screenScaleUp=2; 		// 1 means 320x200, 2 means 640x400 window etc.

// Some constants, don't change them !
static const int screenWidth = 320;		// don't change these 2, otherwise the game will crash !
static const int screenHeight = 200;

// other stuff used by the backend
static SDL_Surface* screen = NULL;
static unsigned char *screenBuffer = NULL;

/* Source for the following is
http://commons.wikimedia.org/w/index.php?title=File:EGA_Table.PNG&oldid=39767054
*/
static unsigned long EGAPalette[64] = {
	0x000000, 0x0000AA, 0x00AA00, 0x00AAAA, //  0- 3
	0xAA0000, 0xAA00AA, 0xAAAA00, 0xAAAAAA, //  4- 7
	0x000055, 0x0000FF, 0x00AA55, 0x00AAFF, //  8-11
	0xAA0055, 0xAA00FF, 0xAAAA55, 0xAAAAFF, // 12-15

	0x005500, 0x0055AA, 0x00FF00, 0x00FFAA, // 16-19
	0xAA5500, 0xAA55AA, 0xAAFF00, 0xAAFFAA, // 20-23
	0x005555, 0x0055FF, 0x00FF55, 0x00FFFF, // 24-27
	0xAA5555, 0xAA55FF, 0xAAFF55, 0xAAFFFF, // 28-31

	0x550000, 0x5500AA, 0x55AA00, 0x55AAAA, // 32-35
	0xFF0000, 0xFF00AA, 0xFFAA00, 0xFFAAAA, // 36-39
	0x550055, 0x5500FF, 0x55AA55, 0x55AAFF, // 40-43
	0xFF0055, 0xFF00FF, 0xFFAA55, 0xFFAAFF, // 44-47

	0x555500, 0x5555AA, 0x55FF00, 0x55FFAA, // 48-51
	0xFF5500, 0xFF55AA, 0xFFFF00, 0xFFFFAA, // 52-55
	0x555555, 0x5555FF, 0x55FF55, 0x55FFFF, // 56-59
	0xFF5555, 0xFF55FF, 0xFFFF55, 0xFFFFFF, // 60-63
};

static int EGADefaultColors[] = {0, 1, 2, 3, 4, 5, 20, 7, 56, 57, 58, 59, 60, 61, 62, 63};

void SPG_Init() {
	screen = SDL_SetVideoMode(screenScaleUp*screenWidth, screenScaleUp*screenHeight, 8, SDL_SWSURFACE);
	assert(screen != NULL);

// clear screen
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = screenScaleUp*screenWidth;
	rect.h = screenScaleUp*screenHeight;
	assert(SDL_FillRect(screen, &rect, 0) == 0);

// initialize the SDL palette to the original EGA palette
	SDL_Color cols[256];
	int i;
	for (i = 0; i < 16; i++) {
		unsigned long hash = EGAPalette[EGADefaultColors[i]];
		cols[i].r = (hash>>16)&0xFF;
		cols[i].g = (hash>>8)&0xFF;
		cols[i].b = hash&0xFF;
	}
	SDL_SetPalette(screen, SDL_LOGPAL|SDL_PHYSPAL, cols, 0, 16);

// allocate and clear buffer
	screenBuffer = malloc(320*200);
	memset(screenBuffer, 0, 320*200);
}


static void flipPixel(unsigned x, unsigned y) { // this is a mean hack to make FizzleFade work, but whatever...
	SDL_Rect rect;
	rect.y = screenScaleUp*y;
	rect.x = screenScaleUp*x;
	rect.w = screenScaleUp;
	rect.h = screenScaleUp;
	assert(SDL_FillRect(screen, &rect, screenBuffer[x+y*screenWidth]) == 0);
}

static void putPixel(unsigned x, unsigned y, unsigned color) {
	if (x < 0 || y < 0 || x >= screenWidth || y >= screenHeight) {
		return;
	}
	screenBuffer[y*screenWidth+x] = color;
}

static void putPixelXOR(unsigned x, unsigned y, unsigned color) {
	if (x < 0 || y < 0 || x >= screenWidth || y >= screenHeight) {
		return;
	}
	screenBuffer[y*screenWidth+x] ^= color;
}

int drawPropChar(int X, int Y, int FontNumber, int Char) {
	fontstruct *font = (fontstruct*)grsegs[STARTFONT+FontNumber];
	int width = font->width[Char];
	uint8_t *c = (uint8_t*)font+font->location[Char];
	int x, y, i;
	for (y = 0; y < font->height; y++) {
		for (x = 0; x < width; x += 8) {
			uint8_t b = *c++;
			int i;
			for (i = 0; i < 8; i++) {
				if (b&1) {
					putPixelXOR(X+x+7-i, Y+y, fontcolor);
				}
				b >>= 1;
			}
		}
	}
	return width;
}



void SPG_FlipBuffer (void) {
	SDL_LockSurface(screen);
	assert(screen->h == screenScaleUp*screenHeight);
	assert(screen->w == screenScaleUp*screenWidth);
	assert(screen->format->BytesPerPixel == 1);
	int y;
	for (y=0; y < screen->h; y++) {
		if (screenScaleUp == 1) {
			memcpy(screen->pixels+y*screen->pitch, screenBuffer+screenWidth*y, screenWidth);
		} else {
			int i, x, Y=y/screenScaleUp;
			unsigned char *dest = screen->pixels+y*screen->pitch;
			unsigned char *src = &screenBuffer[Y*screenWidth];
			for (x=0; x < screenWidth; x++) {
				for (i=0; i < screenScaleUp; i++) {
					*dest++ = *src;
				}
				src++;
			}
		}
	}	
	SDL_UnlockSurface(screen);
	assert(SDL_Flip(screen) == 0);
	SP_PollEvents(false);
}



// id_vw*

void VW_DrawPropString(char *String) {
	while (*String) {
		int w = drawPropChar(px, py, fontnumber, *(unsigned char*)String++);
		px += w;
	}
}


void VW_Hlin(unsigned xl, unsigned xh, unsigned y, unsigned color) {
	if (xh < 0 || xl >= screenWidth || xl > xh || y < 0 || y >= screenHeight) {
		return;
	}
	if (xl < 0) {
		xl = 0;
	}
	if (xh >= screenWidth) {
		xh = screenWidth-1;
	}
	memset(&screenBuffer[y*screenWidth+xl], color, xh+1-xl);
}


void VW_Bar (unsigned x, unsigned y, unsigned width, unsigned height,
	unsigned color)
{
	if (x < 0) {
		width += x;
		x = 0;
	} else if (x >= screenWidth) {
		width += screenWidth-1-x;
		x = screenWidth-1;
	}
	if (y < 0) {
		height += y;
		y = 0;
	} else if (y >= screenHeight) {
		height += screenHeight-1-y;
		y = screenHeight-1;
	}
	if (x+width > screenWidth) {
		width = screenWidth-x;
	}
	if (y+height > screenHeight) {
		height = screenHeight-y;
	}
	assert(x >= 0);
	assert(y >= 0);
	assert(width >= 0);
	assert(height >= 0);
	int i;
	for (i = 0; i < height; i++) {
		memset(&screenBuffer[(y+i)*screenWidth+x], color, width);
	}
}


void	VW_ClearVideo (int color)
{
	memset(screenBuffer, color, screenWidth*screenHeight);
}


void FizzleFade (unsigned width, unsigned height, boolean abortable)
{
	unsigned        x,y,p,frame;
	long            rndval;

	rndval = 1;
	y = 0;
	SDL_Rect rect;

	rect.w = 1;
	rect.h = 1;
	frame=0;
	SP_SetTimeCount(0);
	do      // while (1)
	{
		SP_PollEvents(false);
		if (abortable)
		{
			IN_ReadControl(0,&c);
			if (c.button0 || c.button1 || SP_Keyboard(sc_Space) || SP_Keyboard(sc_Enter))
			{
				SPG_FlipBuffer();
				return;
			}
		}


//#define PIXPERFRAME     1600
#define PIXPERFRAME     1600
		for (p=0;p<PIXPERFRAME;p++)
		{
			y = (rndval&0xFF)-1;
			x = (rndval>>8)&0x1FF;
			if (rndval&1) {
				rndval = 
				rndval = (rndval^0x24000)/2;
			} else {
				rndval = rndval/2;
			}

			assert(x >= 0);
			assert(y >= 0);
			if (x>=width || y>=height)
				continue;

			
			flipPixel(x, y);

			if (rndval == 1)                // entire sequence has been completed
			{
				SDL_Flip(screen);
				return;
			};
		}
		SDL_Flip(screen);
		frame+=1;
		while (SP_TimeCount()<frame) {        // don't go too fast
		}
	} while (1);

}


void VW_DrawTile8(unsigned X, unsigned Y, unsigned Tile) {
	byte *t = (byte*)grsegs[STARTTILE8]+64*Tile;
	int x,y;
	X*=8;
	for (y = 0; y < 8; y++) {
		for (x = 0; x < 8; x++) {
			putPixel(X+x,Y+y, *t++);
		}
	}
}

void VW_DrawTile8M(unsigned X, unsigned Y, unsigned Tile) {
	uint8_t *t = (byte*)grsegs[STARTTILE8M]+64*Tile;
	int x,y;
	X*=8;
	for (y = 0; y < 8; y++) {
		for (x = 0; x < 8; x++) {
			uint8_t c = *t++;
			if (c&0xF0) {
				continue;
			}
			putPixel(X+x, Y+y, c);
		}
	}
}

void SPG_DrawMaskedPic(uint8_t *Source, int ScrX, int ScrY) {
	int width = *(uint32_t*)Source;
	int height = *(1+(uint32_t*)Source);
	byte *planes = Source+2*sizeof(uint32_t);
	int x,y;
	uint8_t c;
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			c = *planes++;
			if (c&0xF0) {
				continue;
			}
			putPixel(ScrX+x, ScrY+y, c);
		}
	}
}

void SPG_DrawMaskedPicSkip(uint8_t *Source, int ScrX, int ScrY, int LowerV, int UpperV) {
	int width = *(uint32_t*)Source;
	byte *planes = Source+2*sizeof(uint32_t)+LowerV*width;
	int x,y;
	uint8_t c;
	for (y = LowerV; y < UpperV; y++) {
		for (x = 0; x < width; x++) {
			c = *planes++;
			if (c&0xF0) {
				continue;
			}
			putPixel(ScrX+x, ScrY+y, c);
		}
	}
}

void SPG_DrawPic(uint8_t *Source, int ScrX, int ScrY) {
	int width = *(uint32_t*)Source;
	int height = *(1+(uint32_t*)Source);
	byte *planes = Source+2*sizeof(uint32_t);
	int x,y;
	for (y = 0; y < height; y++) {
		for (x = 0; x < width; x++) {
			putPixel(ScrX+x, ScrY+y, *planes++);
		}
	}
}

void SPG_DrawPicSkip(uint8_t *Source, int ScrX, int ScrY, int LowerV, int UpperV) {
	int width = *(uint32_t*)Source;
	byte *planes = Source+2*sizeof(uint32_t)+LowerV*width;
	int x,y;
	for (y = LowerV; y < UpperV; y++) {
		for (x = 0; x < width; x++) {
			putPixel(ScrX+x, ScrY+y, *planes++);
		}
	}
}


// c3_asm.asm

unsigned short wallheight[VIEWWIDTH];
unsigned short wallwidth[VIEWWIDTH];
memptr wallseg[VIEWWIDTH];
int wallofs[VIEWWIDTH];

//long wallscalecall[65*6];


void ScaleWalls (void) {
	int j, k, x, w;
	w = 1;
	for (x = 0; x < VIEWWIDTH; x += w) {
		w = wallwidth[x];
		if (w <= 0) {
			w = 2;
			continue;
		}
		int h = wallheight[x];
		assert(h >= 0);
		assert(h < MAXSCALE+1);
		t_compscale *scale = scaledirectory[h];
		byte *wallsrc = wallseg[x]+wallofs[x];

		int srcy;
		for (srcy = 0; srcy < 64; srcy++) {
			int col = *wallsrc++;
			int sy = scale->starty[srcy];
			int ey = scale->endy[srcy];
			if (sy < 0) {
				sy = 0;
			}
			if (sy < ey) {
				VW_Bar(x, sy, w, ey-sy, col);
			}
		}
	}
}


// id_vw_a.asm

void	VW_WaitVBL (int number) {
	long tics = SP_TimeCount()+number;
	SPG_FlipBuffer();
	while (SP_TimeCount() < tics) {
	}
}


// id_vw_ae.asm

extern	unsigned	bufferwidth,bufferheight,screenspot;	// used by font drawing stuff
int			px,py;
byte		pdrawmode,fontcolor;

void VW_Plot(unsigned x, unsigned y, unsigned color) {
	if (x < 0 || y < 0 || x >= screenWidth || y >= screenHeight) {
		return;
	}
	putPixel(x, y, color);
}


void VW_Vlin(unsigned yl, unsigned yh, unsigned x, unsigned color) {
	for (; yl <= yh; yl++) {
		VW_Plot(x, yl, color);
	}
}

