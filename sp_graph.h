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
	uint8_t *BufferStart;
	int Width, Height, Pitch;
	int OffsetX, OffsetY;
	int Scale;
} RenderSetup2D;

void SPG_Init();
void SPG_FlipBuffer();
void SPG_SetBorderColor(int Color);
void SPG_SetWindowSize(int Width, int Height);

void SPG_ClearScreen (void);
void SPG_DrawScaleShape (int XCenter, int Height, uint8_t *Pic, int ColorKey);
int SPG_PollRedraw (void);


void SPG_Bar(RenderSetup2D *Setup, int X, int Y, int Width, int Height, int Color);
void SPG_DrawPic(RenderSetup2D *Setup, uint8_t *Source, int ScrX, int ScrY);
void SPG_DrawPicSkip(RenderSetup2D *Setup, uint8_t *Source, int ScrX, int ScrY, int LowerV, int UpperV);
void SPG_MeasureString (char *String, int FontNumber, int *Width, int *Height);
int SPG_DrawString(RenderSetup2D *Setup, int PX, int PY, char *String, int FontNum, int FontColor);

#endif

