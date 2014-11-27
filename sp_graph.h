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

#ifndef SP_GRAPH_H
#define SP_GRAPH_H

typedef struct {
	int Width, Height, Scale;
	int ScreenX, ScreenY;
} BufferSetup;

typedef struct {
	int Height, Width;
	uint8_t *TexData;
	fixed Depth;
} WallSpan;

typedef struct {
	WallSpan *WallSpans;
	int NumSpans;
} RenderOutput;

extern BufferSetup guiBuffer;

void SPG_ClearBuffer(int Color); // Color < 0 -> BorderColor
void SPG_Init(void);
void SPG_SetWindowSize(int Width, int Height);
void SPG_SetupRenderer(BufferSetup *Buffer);
void SPG_SetBorderColor(int Color);

void SPG_ScaleWalls(BufferSetup *Target, RenderOutput *Walls);
void SPG_DrawScaleShape (BufferSetup *Target, RenderOutput *Walls, int XCenter, int Height, uint8_t *Pic, int ColorKey);
void SPG_FlipBuffer(void);
void SPG_FizzleFadeBuffer(void);
int SPG_ResizeNow (void);

void SPG_Bar(BufferSetup *Setup, int X, int Y, int Width, int Height, int Color);
void SPG_DrawPic(BufferSetup *Setup, uint8_t *Source, int ScrX, int ScrY);
void SPG_DrawPicSkip(BufferSetup *Setup, uint8_t *Source, int ScrX, int ScrY, int LowerV, int UpperV);
void SPG_MeasureString (char *String, int FontNumber, int *Width, int *Height);
int SPG_DrawString(BufferSetup *Setup, int PX, int PY, char *String, int FontNum, int FontColor);

#endif

