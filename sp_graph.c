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

#include <SDL/SDL.h>
#include "id_heads.h"


// Change this to get a bigger window at start.
static int screenScaleUp=2; 		// 1 yields 320x200, 2 yields 640x400 window etc.
int winWidth=0, winHeight=0;

int bufferWidth=0, bufferHeight=0;
uint8_t *buffer = NULL;

/*// Some constants, don't change them !
static const int virtualWidth=320, virtualHeight=200;*/


// other stuff used by the backend
BufferSetup guiBuffer;
static int borderColor=0;
static SDL_Surface* screen = NULL;
static SDL_Surface* screen2 = NULL;
int screenfaded;

RenderSetup renderSetup;
RenderOutput renderOutput;

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
	SPG_ResizeNow();
}


static void putPixel(BufferSetup *Setup, unsigned x, unsigned y, unsigned color) {
	if (x < 0 || y < 0 || x >= Setup->Width || y >= Setup->Height) {
		return;
	}
	y *= Setup->Scale;
	x *= Setup->Scale;
	x += Setup->ScreenX;
	y += Setup->ScreenY;
	int i;
	for (i = 0; i < Setup->Scale; i++) {
		memset(buffer+y++*bufferWidth+x, color, Setup->Scale);
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

static void spg_UpdateRect(SDL_Surface *Dest, SDL_Surface *Source, int X, int Y, int Scale) {
	SDL_Rect rect;
	rect.x = Scale*X;
	rect.y = Scale*Y;
	rect.w = Scale;
	rect.h = Scale;
	SDL_BlitSurface(Source, &rect, Dest, &rect);
}

static void spg_Fizzle (SDL_Surface *Dest, SDL_Surface *Source, int PixelSize) {
	const int width = 320, height = 200;
	int x,y;
	int start,frame;
	uint32_t rndval;
	int done;

	done = 0;
	rndval = 1;
	frame=0;
	start = SP_TimeCount();
	do { // while (!done)
		if (SPI_GetLastKey() != sc_None) {
			done = true;
			break;
		}


		const int PIXPERFRAME = 1600;
		int p;
		for (p=0;p<PIXPERFRAME && !done; p++) {
			y = (rndval&0xFF)-1;
			x = (rndval>>8)&0x1FF;
			if (rndval&1) {
				rndval = 
				rndval = (rndval^0x24000)/2;
			} else {
				rndval = rndval/2;
			}

			if (x>=width || y>=height) {
				continue;
			}
			spg_UpdateRect(Dest,Source,x,y,PixelSize);

			if (rndval == 1) { // entire sequence has been completed
				done = true;
				break;
			};
		}
		if (!done) {
			assert(SDL_Flip(Dest) == 0);
			frame+=1;
			while (SP_TimeCount()<start+frame) { }  // don't go too fast
		}
	} while (!done);
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = bufferWidth;
	rect.h = bufferHeight;
	SDL_BlitSurface(Source, &rect, Dest, &rect);
	SPI_GetMouseDelta(NULL, NULL);
}


void spg_BlitBuffer(SDL_Surface *Screen) {
	SDL_LockSurface(Screen);
	assert(Screen->format->BytesPerPixel == 1);
	int width = bufferWidth;
	int height = bufferHeight;
	if (winWidth < width) {
		width = winWidth;
	}
	if (winHeight < height) {
		height = winHeight;
	}
	if (width <= 0 || height <= 0) {
		return;
	}

// write out the buffer, including renderer window and gui
	int y;
	for (y=0; y < height; y++) {
		memcpy((uint8_t*)Screen->pixels+y*Screen->pitch, &buffer[y*bufferWidth], width);
	}
	SDL_UnlockSurface(Screen);
}

void SPG_ClearBuffer(int Color) {
	if (Color < 0) {
		Color = borderColor;
	}
	memset(buffer, Color, bufferWidth*bufferHeight);
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
	X += Setup->ScreenX;
	Y += Setup->ScreenY;
	Width *= Setup->Scale;
	Height *= Setup->Scale;

	int i;
	for (i = 0; i < Height; i++) {
		memset(&buffer[(Y+i)*bufferWidth+X], Color, Width);
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


const	unsigned	screenbwide = 40;

void SPG_DrawScaleShape (BufferSetup *Target, RenderOutput *Walls, int XCenter, int Height, uint8_t *Pic, int ColorKey)
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
		} else if (x >= Target->Width) { // this column is wholly right off the scrren
			break;
		} else if (nextX > Target->Width) { // this column is partly right off the screen
			nextX = Target->Width;
		}

		while (Walls->WallSpans[x].Depth > fracscale && x < nextX) { // this column is hidden behind a wall
			x++;
		}		
		if (x >= nextX) {
			continue;
		}

		long stepy = ((long)Height<<16) / 64;
		long topy = (Target->Height-Height)/2;

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

			if (starty >= Target->Height) {
				break;
			} else if (endy < 0) {
				continue;
			}
			if (starty < 0) {
				starty = 0;
			}
			if (endy > Target->Height) {
				endy = Target->Height;
			}
			int y;
			for (y = starty; y < endy; y++) {
				memset(buffer+y*bufferWidth+x, color, nextX-x);
			}
		}
	}
}

void SPG_FlipBuffer() {
	spg_BlitBuffer(screen);
	assert(SDL_Flip(screen) == 0);
	SP_PollEvents();
}

void SPG_FizzleFadeBuffer() {
	spg_BlitBuffer(screen2);

	int scale = (bufferWidth+319)/320;
	if ((bufferHeight+199)/200 > scale) {
		scale = (bufferHeight+199)/200;
	}
	if (scale < 1) {
		scale = 1;
	}
	spg_Fizzle(screen, screen2, scale);

	assert(SDL_Flip(screen) == 0);
	SP_PollEvents();
}


void SPG_ScaleWalls (BufferSetup *Target, RenderOutput *Walls) {
	int j, k, x, w;
	w = 1;
	for (x = 0; x < Target->Width; x += w) {
		WallSpan *span = &Walls->WallSpans[x];
		w = span->Width;
		if (w <= 0) {
			w = 2;
			continue;
		}
		int h = span->Height;
		byte *wallsrc = span->TexData;
		assert(h >= 0);
		long step = (h<<17) / 64;
		long toppix = ((int)Target->Height-2*h)/2;
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
			} else if (end > Target->Height) {
				end = Target->Height;
			}
			start += toppix;
			if (start >= (signed int)Target->Height) {
				break;
			} else if (start < 0) {
				start = 0;
			}
			int y;
			for (y = start; y < end; y++) {
				memset(buffer+y*bufferWidth+x, col, w);
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
	SDL_SetPalette(screen2, SDL_LOGPAL|SDL_PHYSPAL, cols, 0, 16);
}

int SPG_ResizeNow() {
	if (bufferWidth == winWidth && bufferHeight == winHeight) {
		return 0;
	}

	printf("Resize now !\n");
	if (buffer != NULL) {
		free(buffer);
		buffer = NULL;
	}

	if (screen != NULL) {
		SDL_FreeSurface(screen);
		screen = NULL;
		SDL_FreeSurface(screen2);
		screen2 = NULL;
	}

	bufferWidth = winWidth;
	if (bufferWidth < 320) {
		bufferWidth = 320;
	}
	bufferHeight = winHeight;
	if (bufferHeight < 200) {
		bufferHeight = 200;
	}
	buffer = malloc(bufferWidth*bufferHeight);
	SPG_ClearBuffer(0);

	screen = SDL_SetVideoMode(bufferWidth, bufferHeight, 8, SDL_SWSURFACE|SDL_RESIZABLE);
	assert(screen != NULL);
	screen2 = SDL_CreateRGBSurface(SDL_SWSURFACE,bufferWidth,bufferHeight,8,0,0,0,0);
	assert(screen2 != NULL);

	SPG_SetColors(NULL);

	GameWindowResizeHook(bufferWidth, bufferHeight);

	return 1;
}

void SPG_SetWindowSize(int Width, int Height) {
	winWidth = Width;
	winHeight = Height;
}

void SPG_SetupRenderer(BufferSetup *Buffer) {
#warning check that this is correct
	assert(!(Buffer->Width&1));
	assert(!(Buffer->Height&1));
	renderSetup.Width = Buffer->Width;
	renderSetup.CenterX = Buffer->ScreenX + Buffer->Width/2-1;

	if (renderOutput.WallSpans != NULL) {
		free(renderOutput.WallSpans);
		renderOutput.WallSpans = NULL;
	}

	int widthplus = Buffer->Width+100;//2*renderSetup.Width+100;
	renderOutput.WallSpans = (WallSpan*)malloc(widthplus*sizeof(WallSpan));
	BuildTables();
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

