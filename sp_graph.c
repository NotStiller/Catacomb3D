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

#include "srcport.h"
#include <SDL/SDL.h>
#include "c3_def.h"

// Change this to get a bigger window.
static int screenScaleUp=2; 		// 1 means 320x200, 2 means 640x400 window etc.
int winWidth=1, winHeight=1;

// Some constants, don't change them !
static const int virtualWidth=320, virtualHeight=200;
static int screenWidth=0, screenHeight=0;		// don't change these 2, otherwise the game will crash !
static int screenOffsetX=0, screenOffsetY=0;
static int redrawNeeded=0;

// other stuff used by the backend
static SDL_Surface* screen = NULL;
static uint8_t *screenBuffer = NULL;
#warning make screenBuffer static or remove entirely !
static int borderColor=0;

RenderSetup3D renderSetup;
RenderSetup2D renderSetup2D;
RenderSetup2D hudSetup;
RenderSetup2D guiSetup;

unsigned short *wallheight;
unsigned short *wallwidth;
uint8_t **wallpointer;
fixed *zbuffer;	

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
	renderSetup.BufferStart = NULL;
	SPG_SetWindowSize(screenScaleUp*virtualWidth+50, screenScaleUp*virtualHeight+50);
	redrawNeeded = 0;
/*// clear screen
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = winWidth;
	rect.h = winHeight;
	assert(SDL_FillRect(screen, &rect, 0) == 0);*/
}


static void putPixel(RenderSetup2D *Setup, unsigned x, unsigned y, unsigned color) {
	if (x < 0 || y < 0 || x >= Setup->Width || y >= Setup->Height) {
		return;
	}
	y = Setup->OffsetY+Setup->Scale*y;
	x = Setup->OffsetX+Setup->Scale*x;
	int i;
	for (i = 0; i < Setup->Scale; i++) {
		memset(Setup->BufferStart+y++*Setup->Pitch+x, color, Setup->Scale);
	}
}

static void putPixelXOR(RenderSetup2D *Setup, unsigned x, unsigned y, unsigned color) {
	if (x < 0 || y < 0 || x >= Setup->Width || y >= Setup->Height) {
		return;
	}
	y = Setup->OffsetY+Setup->Scale*y;
	x = Setup->OffsetX+Setup->Scale*x;
	int i, j;
	for (i = 0; i < Setup->Scale; i++) {
		for (j = 0; j < Setup->Scale; j++) {
			Setup->BufferStart[(y+i)*Setup->Pitch+x+j] ^= color;
		}
	}
}


//static int drawPropChar(RenderSetup2D *Setup, int X, int Y, int FontNumber, int Char) {
static int drawPropChar(RenderSetup2D *Setup, int X, int Y, fontstruct *Font, int FontColor, int Char) {
	fontstruct *font = Font;
	int width = font->width[Char];
	uint8_t *c = (uint8_t*)font+font->location[Char];
	int x, y, i;
	for (y = 0; y < font->height; y++) {
		for (x = 0; x < width; x += 8) {
			uint8_t b = *c++;
			int i;
			for (i = 0; i < 8; i++) {
				if (b&1) {
//					putPixelXOR(Setup, X+x+7-i, Y+y, FontColor);
					putPixel(Setup, X+x+7-i, Y+y, FontColor);
				}
				b >>= 1;
			}
		}
	}
	return width;
}

static void flipBufferCentered(void) {
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = winWidth;
	rect.h = winHeight;
	assert(SDL_FillRect(screen, &rect, 0) == 0);

	int bufOffsX, bufOffsY, bufWidth, bufHeight;
	bufOffsX = screenWidth-screenScaleUp*virtualWidth;
	bufOffsY = screenHeight-screenScaleUp*virtualHeight;
	bufWidth = screenScaleUp*virtualWidth;
	bufHeight = screenScaleUp*virtualHeight;

	int winOffsX, winOffsY;
	winOffsX = (winWidth-screenScaleUp*virtualWidth)/2;
	winOffsY = (winHeight-screenScaleUp*virtualHeight)/2;

	// the buffer will never be scaled down, so window can be too small !
	if (winWidth < screenScaleUp*virtualWidth) {
		winOffsX = 0;
		bufWidth = winWidth;
	}
	if (winHeight < screenScaleUp*virtualHeight) {
		winOffsY = 0;
		bufHeight = winHeight;
	}

	
	SDL_LockSurface(screen);
	assert(screen->format->BytesPerPixel == 1);

	int y;
	for (y=0; y < bufHeight; y++) {
		memcpy(screen->pixels+(y+winOffsY)*screen->pitch+winOffsX, screenBuffer+(y+bufOffsY)*screenWidth+bufOffsX, bufWidth);
	}
	SDL_UnlockSurface(screen);
	assert(SDL_Flip(screen) == 0);
	SP_PollEvents(false);
}

static void flipBufferOffset(void) {
	SDL_LockSurface(screen);
	assert(screen->format->BytesPerPixel == 1);
	int drawWidth, drawHeight;
	drawWidth = screenWidth;
	drawHeight = screenHeight;
	if (winWidth < screenWidth) {
		drawWidth = winWidth;
	}
	if (winHeight < screenHeight) {
		drawHeight = winHeight;
	}
	assert(screen->pitch >= drawWidth);
	int y;
	// write out the buffer, including renderer window and gui
	for (y=0; y < drawHeight; y++) {
		memcpy(screen->pixels+y*screen->pitch, screenBuffer+y*screenWidth, drawWidth);
	}
	if (winHeight == screenHeight && winWidth == screenWidth) {
		// write background color at top right (if there is space)
		for (y=0; y < screenOffsetY; y++) {
			memset(screen->pixels+y*screen->pitch+renderSetup.Width, borderColor, screenWidth-renderSetup.Width);
		}
		// write background color at lower left (if there is space)
		for (y=renderSetup.Height; y < screenHeight; y++) {
			memset(screen->pixels+y*screen->pitch, borderColor, screenOffsetX);
		}
	}
	SDL_UnlockSurface(screen);
	assert(SDL_Flip(screen) == 0);
	SP_PollEvents(false);
}

void SPG_FlipBuffer (void) {
	if (!SP_GameActive()) {
		flipBufferCentered();
	} else {
		flipBufferOffset();
	}
}

void SPG_Bar(RenderSetup2D *Setup, int X, int Y, int Width, int Height, int Color)
{
	if (X < 0) {
		Width += X;
		X = 0;
	}
	if (Y < 0) {
		Height += Y;
		Y = 0;
	}
	if (X+Width > Setup->Width) {
		Width = Setup->Width-X;
	}
	if (Y+Height > Setup->Height) {
		Height = Setup->Height-Y;
	}
	assert(X >= 0);
	assert(Y >= 0);
	assert(Width >= 0);
	assert(Height >= 0);
	X = Setup->OffsetX+Setup->Scale*X;
	Y = Setup->OffsetY+Setup->Scale*Y;
	Width *= Setup->Scale;
	Height *= Setup->Scale;

	int i;
	for (i = 0; i < Height; i++) {
		memset(&screenBuffer[(Y+i)*Setup->Pitch+X], Color, Width);
	}
}


void SPG_MeasureString (char *String, int FontNumber, int *Width, int *Height) {
	fontstruct *font = (fontstruct *)grsegs[STARTFONT+FontNumber];
	byte *f = (byte*)font;

	if (Height != NULL) {
		*Height = font->height;
	}

	if (Width != NULL) {
		for (*Width = 0;*String;String++)
			*Width += font->width[*(byte*)String];
	}
}

int SPG_DrawString(RenderSetup2D *Setup, int PX, int PY, char *String, int FontNum, int FontColor) {
	fontstruct *font = (fontstruct*)grsegs[STARTFONT+FontNum];
	while (*String) {
		int w = drawPropChar(Setup, PX, PY, font, FontColor, *(unsigned char*)String++);
		PX += w;
	}
	return PX;
}


static void flipPixel(unsigned x, unsigned y) { // this is used by FizzleFade
	SDL_Rect rect;
	x = screenOffsetX+screenScaleUp*x;
	y = screenOffsetY+screenScaleUp*y;
	rect.x = x;
	rect.y = y;
	rect.w = screenScaleUp;
	rect.h = screenScaleUp;
	assert(SDL_FillRect(screen, &rect, screenBuffer[x+y*screenWidth]) == 0);
#warning this function is bullshit
}

void FizzleFade (unsigned width, unsigned height, boolean abortable)
{
	SPG_FlipBuffer();
printf("Reimplement fizzle fade\n");
//SPG_FlipBuffer();
return;
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

void SPG_DrawTile8(RenderSetup2D *Setup, unsigned X, unsigned Y, uint8_t *Tile) {
	uint8_t *t = Tile;
	int x,y;
	for (y = 0; y < 8; y++) {
		for (x = 0; x < 8; x++) {
			uint8_t c = *t++;
			if (c&0xF0) {
				continue;
			}
			putPixel(Setup, X+x, Y+y, c);
		}
	}
}

void SPG_DrawPic(RenderSetup2D *Setup, uint8_t *Source, int ScrX, int ScrY) {
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
			putPixel(Setup, ScrX+x, ScrY+y, c);
		}
	}
}

void SPG_DrawPicSkip(RenderSetup2D *Setup, uint8_t *Source, int ScrX, int ScrY, int LowerV, int UpperV) {
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
			putPixel(Setup, ScrX+x, ScrY+y, c);
		}
	}
}


// c_draw.c

void SPG_ClearScreen (void)
{
	int y = 0;
	for (; y < renderSetup.CenterY+1; y++) {
		memset(renderSetup.BufferStart+y*renderSetup.Pitch, 0, renderSetup.Width);
	}
	for (; y < renderSetup.Height; y++) {
		memset(renderSetup.BufferStart+y*renderSetup.Pitch, 8, renderSetup.Width);
	}
}



// c_scale.c


const	unsigned	screenbwide = 40;

void SPG_DrawScaleShape (int XCenter, int Height, uint8_t *Pic, int ColorKey)
{
	long width = ((uint32_t*)Pic)[0];
	uint8_t *picdata = Pic+2*sizeof(uint32_t);

	short int		x,pixel,pixwidth,min;
	unsigned short	*widthptr;
	int codehandle;

	unsigned short	badcodeptr;
	short int		rightclip;

	int scale = (Height+1)/2;
	if (!scale) {
		return;
	}
	fixed fracscale = scale<<16;
	long stepx = (2*fracscale) / 64;
	int scalewidth = (width*stepx)>>16;
	int nextX=XCenter - scalewidth/2;
	int u;
	for (u = 0; u < width; u++) {
		long x1 = (u*stepx)>>16;
		long x2 = ((u+1)*stepx)>>16;
		if (x1 == x2) {
			continue;
		}
		int x = nextX;
		nextX += x2-x1;
		if (nextX <= 0) { // this column is wholly left off the screen
			continue;
		} else if (x < 0) { // this column is partly left off and partly on the screen
			x = 0;
		} else if (x >= renderSetup.Width) { // this column is wholly right off the scrren
			break;
		} else if (nextX > renderSetup.Width) { // this column is partly right off the screen
			nextX = renderSetup.Width;
		}

		while (zbuffer[x] > fracscale && x < nextX) { // this column is hidden behind a wall
			x++;
		}		
		if (x >= nextX) {
			continue;
		}

		long stepy = ((long)Height<<16) / 64;
		long topy = (renderSetup.Height-Height)/2;

		int v=0;
		for (v = 0; v < 64; v++) {
			long starty = (stepy*v)>>16;
			long endy = (stepy*(v+1))>>16;
			int color = picdata[v*width+u];
			if (color == ColorKey) {
				continue;
			}
			
			if (endy <= starty) {
				continue;
			}
			starty += topy;
			endy += topy;

			if (starty >= renderSetup.Height) {
				break;
			} else if (endy < 0) {
				continue;
			}
			if (starty < 0) {
				starty = 0;
			}
			if (endy > renderSetup.Height) {
				endy = renderSetup.Height;
			}
			int y;
			for (y = starty; y < endy; y++) {
				memset(renderSetup.BufferStart+y*renderSetup.Pitch+x, color, nextX-x);
			}
		}
	}
}




// c3_asm.asm


void ScaleWalls (void) {
	int j, k, x, w;
	w = 1;
	for (x = 0; x < renderSetup.Width; x += w) {
		w = wallwidth[x];
		if (w <= 0) {
			w = 2;
			continue;
		}
		int h = wallheight[x];
		byte *wallsrc = wallpointer[x];
		assert(h >= 0);
		long step = (h<<17) / 64;
		long toppix = ((int)renderSetup.Height-2*h)/2;
		int srcy;
		for (srcy=0; srcy < 64; srcy++) {
//			int col = *wallsrc++;
			int col = wallsrc[srcy*64];
			long start = (srcy*step)>>16;
			long end = ((srcy+1)*step)>>16;
			if (start == end) {
				continue;
			}
			end += toppix;
			if (end < 0) {
				continue;
			} else if (end > renderSetup.Height) {
				end = renderSetup.Height;
			}
			start += toppix;
			if (start >= (signed int)renderSetup.Height) {
				break;
			} else if (start < 0) {
				start = 0;
			}
			int y;
			for (y = start; y < end; y++) {
				memset(renderSetup.BufferStart+y*renderSetup.Pitch+x, col, w);
			}
		}
	}
}

void SPG_SetBorderColor(int Color) {
	borderColor = Color;
}

void SPG_SetWindowSize(int Width, int Height) {
	winWidth = Width;
	winHeight = Height;

	if (screen != NULL) {
		SDL_FreeSurface(screen);
		screen = NULL;
	}
	screen = SDL_SetVideoMode(winWidth, winHeight, 8, SDL_SWSURFACE|SDL_RESIZABLE);
	assert(screen != NULL);

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

	int scale = Width/virtualWidth, scaleH = Height/virtualHeight;
	screenWidth = Width;
	screenHeight = Height;
	if (scaleH < scale) {
		scale = scaleH;
	}
	screenScaleUp = scale;
	if (scale < 1) {
		screenScaleUp = scale = 1;
		screenWidth = virtualWidth;
		screenHeight = virtualHeight;
	}
	screenOffsetX = screenWidth-screenScaleUp*virtualWidth;
	screenOffsetY = screenHeight-screenScaleUp*virtualHeight;
	
	if (screenBuffer != NULL) {
		free(screenBuffer);
		screenBuffer = NULL;
		free(wallheight);
		wallheight = NULL;
		free(wallwidth);
		wallwidth = NULL;
		free(wallpointer);
		wallpointer = NULL;
		free(zbuffer);
		zbuffer = NULL;
	}
	screenBuffer = malloc(screenWidth*screenHeight);

	hudSetup.BufferStart = screenBuffer;	
	hudSetup.Pitch = screenWidth;
	hudSetup.Width = virtualWidth;
	hudSetup.Height = virtualHeight;
	hudSetup.OffsetX = screenWidth-screenScaleUp*virtualWidth;
	hudSetup.OffsetY = screenHeight-screenScaleUp*virtualHeight;
	hudSetup.Scale = screenScaleUp;

	guiSetup.BufferStart = screenBuffer;	
	guiSetup.Pitch = screenWidth;
	guiSetup.Width = virtualWidth;
	guiSetup.Height = virtualHeight;
	guiSetup.OffsetX = 0;
	guiSetup.OffsetY = 0;
	guiSetup.Scale = screenScaleUp;
	guiSetup = hudSetup;

	renderSetup.Width = (screenWidth-screenScaleUp*7*8)/2*2;
	renderSetup.Height = (screenHeight-screenScaleUp*7*8)/2*2;
	renderSetup.Pitch = screenWidth;
	renderSetup.BufferStart = screenBuffer;
	renderSetup.CenterX = renderSetup.Width/2-1;
	renderSetup.CenterY = renderSetup.Height/2-1;
	renderSetup.EnterPlaqueX = winWidth/2/screenScaleUp-screenOffsetX;
	renderSetup.EnterPlaqueY = winHeight/2/screenScaleUp-screenOffsetY;

	renderSetup2D.BufferStart = screenBuffer;	
	renderSetup2D.Pitch = screenWidth;
	renderSetup2D.Width = renderSetup.Width/screenScaleUp;
	renderSetup2D.Height = renderSetup.Height/screenScaleUp;
	renderSetup2D.Scale = screenScaleUp;
	renderSetup2D.OffsetX = renderSetup.Width-screenScaleUp*renderSetup2D.Width;
	renderSetup2D.OffsetY = renderSetup.Height-screenScaleUp*renderSetup2D.Height;


	int widthplus = 2*renderSetup.Width+100;
	wallheight = malloc(sizeof(unsigned short)*widthplus);
	wallwidth = malloc(sizeof(unsigned short)*widthplus);
	wallpointer = malloc(sizeof(uint8_t*)*widthplus);
	zbuffer = malloc(sizeof(int)*widthplus);
	BuildTables();
	redrawNeeded = 1;
}

int SPG_PollRedraw (void) {
	if (redrawNeeded) {
		redrawNeeded = 0;
		return 1;
	}
	return 0;
}



void VW_Vlin(unsigned yl, unsigned yh, unsigned x, unsigned color) {
	for (; yl <= yh; yl++) {
		putPixel(&guiSetup, x, yl, color);
	}
}


void VW_DrawPropString(char *String) {
	px = SPG_DrawString(&guiSetup, px, py, String, fontnumber, fontcolor);
/*	while (*String) {
		int w = drawPropChar(guiSetup, px, py, fontnumber, *(unsigned char*)String++);
		px += w;
	}*/
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
	y = screenOffsetY+screenScaleUp*y;
	int width = screenScaleUp*(xh+1-xl);
	xl = screenOffsetX+screenScaleUp*xl;
	int i;
	for (i = 0; i < screenScaleUp; i++) {
		memset(&screenBuffer[(y+i)*screenWidth+xl], color, width);
	}
}


void	VW_MeasurePropString (char *string, word *width, word *height)
{
	fontstruct *font = (fontstruct *)grsegs[STARTFONT+fontnumber];
	byte *f = (byte*)font;

	*height = font->height;
	for (*width = 0;*string;string++)
		*width += font->width[*(byte*)string];
}

void VW_Bar (unsigned x, unsigned y, unsigned width, unsigned height, unsigned color)
{
	if (x < 0) {
		width += x;
		x = 0;
	} else if (x >= virtualWidth) {
		width += virtualWidth-1-x;
		x = virtualWidth-1;
	}
	if (y < 0) {
		height += y;
		y = 0;
	} else if (y >= virtualHeight) {
		height += virtualHeight-1-y;
		y = virtualHeight-1;
	}
	if (x+width > virtualWidth) {
		width = virtualWidth-x;
	}
	if (y+height > virtualHeight) {
		height = virtualHeight-y;
	}
	assert(x >= 0);
	assert(y >= 0);
	assert(width >= 0);
	assert(height >= 0);
	x = screenOffsetX+screenScaleUp*x;
	y = screenOffsetY+screenScaleUp*y;
	width *= screenScaleUp;
	height *= screenScaleUp;

	int i;
	for (i = 0; i < height; i++) {
		memset(&screenBuffer[(y+i)*screenWidth+x], color, width);
	}
}

void VW_DrawTile8(unsigned X, unsigned Y, unsigned Tile) {
	SPG_DrawTile8(&guiSetup, 8*X,Y,(byte*)grsegs[STARTTILE8]+64*Tile);
}

void VW_DrawTile8M(unsigned X, unsigned Y, unsigned Tile) {
	SPG_DrawTile8(&guiSetup, 8*X,Y,(byte*)grsegs[STARTTILE8M]+64*Tile);
}


