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
	uint8_t *Buffer;
	int Pitch, Width, Height, Scale;
	int ScreenX, ScreenY;
} BufferSetup;

extern BufferSetup guiBuffer;

/*dep
void SPG_FlipBuffer();
void SPG_ClearScreen (int Color);
void SPG_DrawFloors (int Floor, int Ceiling);
*/

void SPG_Init();
void SPG_SetWindowSize(int Width, int Height);
void SPG_SetupRenderer(int Width, int Height, char *Buffer, int BufferPitch);
void SPG_ClearBlitAndFlip(int ClearColor, BufferSetup *RenderBuffers[]);
void SPG_ClearBlitAndFizzle(int ClearColor, BufferSetup *RenderBuffers[]);
void SPG_ClearScaleAndFlip(int ClearColor, BufferSetup *RenderBuffer);
void SPG_SetBorderColor(int Color);

void SPG_DrawScaleShape (int XCenter, int Height, uint8_t *Pic, int ColorKey);
int SPG_PollRedraw (void);

void SPG_Bar(BufferSetup *Setup, int X, int Y, int Width, int Height, int Color);
void SPG_DrawPic(BufferSetup *Setup, uint8_t *Source, int ScrX, int ScrY);
void SPG_DrawPicSkip(BufferSetup *Setup, uint8_t *Source, int ScrX, int ScrY, int LowerV, int UpperV);
void SPG_MeasureString (char *String, int FontNumber, int *Width, int *Height);
int SPG_DrawString(BufferSetup *Setup, int PX, int PY, char *String, int FontNum, int FontColor);

#endif

