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
#include "id_heads.h"


// Change this to get a bigger window at start.
static int screenScaleUp=2; 		// 1 means 320x200, 2 means 640x400 window etc.
int winWidth=1, winHeight=1;

/*// Some constants, don't change them !
static const int virtualWidth=320, virtualHeight=200;*/


// other stuff used by the backend
BufferSetup guiBuffer;
static int redrawNeeded=0;
static int borderColor=0;
static SDL_Surface* screen = NULL;
int screenfaded;

RenderSetup3D renderSetup;

unsigned short *wallheight = NULL;
unsigned short *wallwidth = NULL;
uint8_t **wallpointer = NULL;
fixed *zbuffer = NULL;	

/* Source for the following is
http://commons.wikimedia.org/w/index.php?title=File:EGA_Table.PNG&oldid=39767054
*/
static uint32_t EGAPalette[64] = {
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

int CatacombColors[7][17]=
{{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
 {0,0,0,0,0,0,0,0,0,1,2,3,4,5,6,7,0},
 {0,0,0,0,0,0,0,0,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0},
 {0,1,2,3,4,5,6,7,0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0},
 {0,1,2,3,4,5,6,7,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0},
 {0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f,0x1f}};

static int EGADefaultColors[16] = {0, 1, 2, 3, 4, 5, 20, 7, 56, 57, 58, 59, 60, 61, 62, 63};
static int *curColors = EGADefaultColors;

void SPG_Init() {
	SPG_SetWindowSize(screenScaleUp*320+50, screenScaleUp*200+50);
	redrawNeeded = 0;
}


static void putPixel(BufferSetup *Setup, unsigned x, unsigned y, unsigned color) {
	if (x < 0 || y < 0 || x >= Setup->Width || y >= Setup->Height) {
		return;
	}
	y *= Setup->Scale;
	x *= Setup->Scale;
	int i;
	for (i = 0; i < Setup->Scale; i++) {
		memset(Setup->Buffer+y++*Setup->Pitch+x, color, Setup->Scale);
	}
}

static void putPixelXOR(BufferSetup *Setup, unsigned x, unsigned y, unsigned color) {
	if (x < 0 || y < 0 || x >= Setup->Width || y >= Setup->Height) {
		return;
	}
	y *= Setup->Scale;
	x *= Setup->Scale;
	int i, j;
	for (i = 0; i < Setup->Scale; i++) {
		for (j = 0; j < Setup->Scale; j++) {
			Setup->Buffer[(y+i)*Setup->Pitch+x+j] ^= color;
		}
	}
}


//static int drawPropChar(BufferSetup *Setup, int X, int Y, int FontNumber, int Char) {
static int drawPropChar(BufferSetup *Setup, int X, int Y, fontstruct *Font, int FontColor, int Char) {
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

void spg_Fizzle (int Scale)
{
	unsigned        x,y,p,frame;
	long            rndval;
	boolean abortable = true;
	int width = 320;
	int height = 200;
	rndval = 1;
	y = 0;
	frame=0;
	int start = SP_TimeCount();
	do      // while (1)
	{
		SP_PollEvents(false);
		if (abortable)
		{
extern	ControlInfo	control;
			IN_ReadControl(0,&control);
			if (control.button0 || control.button1 || SP_Keyboard(sc_Space) || SP_Keyboard(sc_Enter))
			{
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

			
			SDL_UpdateRect(screen,Scale*x,Scale*y,Scale,Scale);

			if (rndval == 1)                // entire sequence has been completed
			{
				return;
			};
		}
		frame+=1;
		while (SP_TimeCount()<start+frame) {        // don't go too fast
		}
	} while (1);

}



void spg_Blit(SDL_Surface *Screen, BufferSetup *RenderBuffers[]) {
	SDL_LockSurface(Screen);
	assert(Screen->format->BytesPerPixel == 1);
	int i;
	for (i = 0; RenderBuffers[i] != NULL; i++) {
		BufferSetup *buf = RenderBuffers[i];
		int width = buf->Width;
		int height = buf->Height;
		if (winWidth < buf->ScreenX+width) {
			width = winWidth-buf->ScreenX;
		}
		if (winHeight < buf->ScreenY+height) {
			height = winHeight-buf->ScreenY;
		}
		if (width <= 0 || height <= 0) {
			continue;
		}

		// write out the buffer, including renderer window and gui
		int y;
		for (y=0; y < height; y++) {
			memcpy((uint8_t*)Screen->pixels+(y+buf->ScreenY)*Screen->pitch+buf->ScreenX, &buf->Buffer[y*buf->Pitch], width);
		}
	}
	SDL_UnlockSurface(Screen);
}



void SPG_ClearBlitAndFlip(int ClearColor, BufferSetup *RenderBuffers[]) {
	if (ClearColor < 0) {
		ClearColor = borderColor;
	}
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = winWidth;
	rect.h = winHeight;
	assert(SDL_FillRect(screen, &rect, ClearColor) == 0);

	spg_Blit(screen, RenderBuffers);

	assert(SDL_Flip(screen) == 0);
	SP_PollEvents(false);
}


void SPG_ClearBlitAndFizzle(int ClearColor, BufferSetup *RenderBuffers[]) {
	if (ClearColor < 0) {
		ClearColor = borderColor;
	}
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = winWidth;
	rect.h = winHeight;
	assert(SDL_FillRect(screen, &rect, ClearColor) == 0);

	spg_Blit(screen, RenderBuffers);

	int scale = (winWidth+319)/320;
	if ((winHeight+199)/200 > scale) {
		scale = (winHeight+199)/200;
	}
	if (scale < 1) {
		scale = 1;
	}
	spg_Fizzle(scale);
	assert(SDL_Flip(screen) == 0);
	SP_PollEvents(false);
}

void spg_Scale(SDL_Surface *Screen, BufferSetup *RenderBuffer) {
	int scale = winWidth/RenderBuffer->Width, scaleV = winHeight/RenderBuffer->Height;
	if (scaleV < scale) {
		scale = scaleV;
	}
	if (scale < 1) {
		scale = 1;
	}
	int width = scale*RenderBuffer->Width, height = scale*RenderBuffer->Height;
	int offsX = (winWidth-width)/2;
	int offsY = (winHeight-height)/2;
	if (winWidth < width) {
		offsX = 0;
		width = winWidth;
	}
	if (winHeight < height) {
		offsY = 0;
		height = winHeight;
	}

	SDL_LockSurface(Screen);
	assert(Screen->format->BytesPerPixel == 1);

	int x,y;
	for (y=0; y < height; y++) {
		uint8_t *src = RenderBuffer->Buffer+(y/scale)*RenderBuffer->Pitch;
		uint8_t *dst = Screen->pixels+(y+offsY)*Screen->pitch+offsX;
		for (x=0; x < width; x++) {
			dst[x] = src[x/scale];
		}
	}

	SDL_UnlockSurface(Screen);
}

void SPG_ClearScaleAndFlip(int ClearColor, BufferSetup *RenderBuffer) {
	if (ClearColor < 0) {
		ClearColor = borderColor;
	}
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = winWidth;
	rect.h = winHeight;
	assert(SDL_FillRect(screen, &rect, ClearColor) == 0);

	spg_Scale(screen, RenderBuffer);

	assert(SDL_Flip(screen) == 0);
	SP_PollEvents(false);
}

void SPG_ClearScaleAndFizzle(int ClearColor, BufferSetup *RenderBuffer) {
	if (ClearColor < 0) {
		ClearColor = borderColor;
	}
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = winWidth;
	rect.h = winHeight;
	assert(SDL_FillRect(screen, &rect, ClearColor) == 0);

	spg_Scale(screen, RenderBuffer);

	int scale = (winWidth+319)/320;
	if ((winHeight+199)/200 > scale) {
		scale = (winHeight+199)/200;
	}
	if (scale < 1) {
		scale = 1;
	}
	spg_Fizzle(scale);
	assert(SDL_Flip(screen) == 0);
	SP_PollEvents(false);
}

void SPG_FlipBuffer(void) {
	FlipBuffer();
}

void SPG_Bar(BufferSetup *Setup, int X, int Y, int Width, int Height, int Color)
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
	X *= Setup->Scale;
	Y *= Setup->Scale;
	Width *= Setup->Scale;
	Height *= Setup->Scale;

	int i;
	for (i = 0; i < Height; i++) {
		memset(&Setup->Buffer[(Y+i)*Setup->Pitch+X], Color, Width);
	}
}


void SPG_MeasureString (char *String, int FontNumber, int *Width, int *Height) {
	fontstruct *font = (fontstruct *)grsegs[StartFont+FontNumber];
	byte *f = (byte*)font;

	if (Height != NULL) {
		*Height = font->height;
	}

	if (Width != NULL) {
		for (*Width = 0;*String;String++)
			*Width += font->width[*(byte*)String];
	}
}

int SPG_DrawString(BufferSetup *Setup, int PX, int PY, char *String, int FontNum, int FontColor) {
	fontstruct *font = (fontstruct*)grsegs[StartFont+FontNum];
	while (*String) {
		int w = drawPropChar(Setup, PX, PY, font, FontColor, *(unsigned char*)String++);
		PX += w;
	}
	return PX;
}


void SPG_DrawTile8(BufferSetup *Setup, unsigned X, unsigned Y, uint8_t *Tile) {
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

void SPG_DrawPic(BufferSetup *Setup, uint8_t *Source, int ScrX, int ScrY) {
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

void SPG_DrawPicSkip(BufferSetup *Setup, uint8_t *Source, int ScrX, int ScrY, int LowerV, int UpperV) {
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


void SPG_DrawFloors (int Floor, int Ceiling)
{
	int y = 0;
	for (; y < renderSetup.CenterY+1; y++) {
		memset(renderSetup.BufferStart+y*renderSetup.Pitch, Ceiling, renderSetup.Width);
	}
	for (; y < renderSetup.Height; y++) {
		memset(renderSetup.BufferStart+y*renderSetup.Pitch, Floor, renderSetup.Width);
	}
}


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

void SPG_SetColors(int *Colors) {
	if (Colors != NULL) {
		curColors = Colors;
	}
	SDL_Color cols[256];
	int i;
	for (i = 0; i < 16; i++) {
		unsigned long hash = EGAPalette[curColors[i]];
		cols[i].r = (hash>>16)&0xFF;
		cols[i].g = (hash>>8)&0xFF;
		cols[i].b = hash&0xFF;
	}
	SDL_SetPalette(screen, SDL_LOGPAL|SDL_PHYSPAL, cols, 0, 16);
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

	SPG_SetColors(NULL);

	GameWindowResizeHook(Width, Height);
}

void SPG_SetupRenderer(int Width, int Height, char *Buffer, int BufferPitch) {
	renderSetup.Width = Width/2*2;
	renderSetup.Height = Height/2*2;
	renderSetup.Pitch = BufferPitch;
	renderSetup.BufferStart = Buffer;
	renderSetup.CenterX = renderSetup.Width/2-1;
	renderSetup.CenterY = renderSetup.Height/2-1;

	if (wallheight != NULL) {
		free(wallheight);
		wallheight = NULL;
		free(wallwidth);
		wallwidth = NULL;
		free(wallpointer);
		wallpointer = NULL;
		free(zbuffer);
		zbuffer = NULL;
	}

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
		putPixel(&guiBuffer, x, yl, color);
	}
}


void VW_DrawPropString(char *String) {
	px = SPG_DrawString(&guiBuffer, px, py, String, fontnumber, fontcolor);
/*	while (*String) {
		int w = drawPropChar(&guiBuffer, px, py, fontnumber, *(unsigned char*)String++);
		px += w;
	}*/
}


void VW_Hlin(unsigned xl, unsigned xh, unsigned y, unsigned color) {
	if (xh < 0 || xl >= 320 || xl > xh || y < 0 || y >= 200) {
		return;
	}
	if (xl < 0) {
		xl = 0;
	}
	if (xh >= 320) {
		xh = 319;
	}
	for (; xl <= xh; xl++) {
		putPixel(&guiBuffer, xl,y,color);
	}
}


void	VW_MeasurePropString (char *string, word *width, word *height)
{
	fontstruct *font = (fontstruct *)grsegs[StartFont+fontnumber];
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
	} else if (x >= 320) {
		width += 320-1-x;
		x = 320-1;
	}
	if (y < 0) {
		height += y;
		y = 0;
	} else if (y >= 200) {
		height += 200-1-y;
		y = 200-1;
	}
	if (x+width > 320) {
		width = 320-x;
	}
	if (y+height > 200) {
		height = 200-y;
	}
	assert(x >= 0);
	assert(y >= 0);
	assert(width >= 0);
	assert(height >= 0);

	int i;
	for (i = 0; i < height; i++) {
		VW_Hlin(x,x+width-1,y+i,color);
	}
}

void VW_DrawTile8(unsigned X, unsigned Y, unsigned Tile) {
	SPG_DrawTile8(&guiBuffer, 8*X,Y,(byte*)grsegs[StartTile8]+64*Tile);
}

void VW_DrawTile8M(unsigned X, unsigned Y, unsigned Tile) {
	SPG_DrawTile8(&guiBuffer, 8*X,Y,(byte*)grsegs[StartTile8M]+64*Tile);
}

void VW_WaitVBL(int Ticks) {
	int start = SDL_GetTicks();
	while (70*(SDL_GetTicks()-start) < 1000*Ticks) {
	}
}



void VW_SetDefaultColors(void)
{
extern int screenfaded;
	SPG_SetColors(CatacombColors[3]);
	screenfaded = false;
}

void VW_FadeOut(void)
{
extern int screenfaded;
	int i;
	for (i=3;i>=0;i--)
	{
		SPG_SetColors(CatacombColors[i]);
		VW_WaitVBL(6);
	}
	screenfaded = true;
}


void VW_FadeIn(void)
{
extern int screenfaded;
	int i;

	for (i=0;i<4;i++)
	{
		SPG_SetColors(CatacombColors[i]);
		VW_WaitVBL(6);
	}
	screenfaded = false;
}



#if 0
static void flipPixel(unsigned x, unsigned y) { // this is used by FizzleFade
return;
#if 0
	SDL_Rect rect;
	x = screenOffsetX+screenScaleUp*x;
	y = screenOffsetY+screenScaleUp*y;
	rect.x = x;
	rect.y = y;
	rect.w = screenScaleUp;
	rect.h = screenScaleUp;
	assert(SDL_FillRect(screen, &rect, screenBuffer[x+y*screenWidth]) == 0);
#endif
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
	int start = SP_TimeCount();
	do      // while (1)
	{
		SP_PollEvents(false);
		if (abortable)
		{
extern	ControlInfo	c;
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
		while (SP_TimeCount()<start+frame) {        // don't go too fast
		}
	} while (1);

}
#endif

