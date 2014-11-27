/* Catacomb Abyss Source Code
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

// C3_DRAW.C

#include "c4_def.h"

BufferSetup renderBuffer; // for blitting
BufferSetup renderBufferText; // for "entering level" text
BufferSetup bottomHUDBuffer;

void GameWindowResizeHook(int NewWidth, int NewHeight) {
	int scale = NewWidth/320, scaleH = NewHeight/200;
	if (scaleH < scale) {
		scale = scaleH;
	}
	if (scale < 1) {
		scale = 1;
		NewWidth = 320;
		NewHeight = 200;
	}

	bottomHUDBuffer.Width = 320;
	bottomHUDBuffer.Height = 80;
	bottomHUDBuffer.Scale = scale;
	bottomHUDBuffer.ScreenX = (NewWidth-scale*bottomHUDBuffer.Width)/2;
	bottomHUDBuffer.ScreenY = NewHeight-scale*bottomHUDBuffer.Height;

	renderBuffer.Width = (NewWidth)/2*2;
	renderBuffer.Height = (NewHeight-scale*80)/2*2;
	renderBuffer.Scale = 1; // should never be used
	renderBuffer.ScreenX = NewWidth&1;
	renderBuffer.ScreenY = (NewHeight-scale*80)&1;

	renderBufferText.Width = renderBuffer.Width/scale;
	renderBufferText.Height = renderBuffer.Height/scale;
	renderBufferText.Scale = scale;
	renderBufferText.ScreenX = renderBuffer.ScreenX; // will not be blitted
	renderBufferText.ScreenY = renderBuffer.ScreenY;

	SPG_SetupRenderer(&renderBuffer); 
}

void	DrawHand (void)
{
	int	picnum;

	picnum = HAND1PICM;
//	SPG_DrawMaskedPicSkip(grsegs[picnum], ((VIEWWIDTH/16)-(10/2)), VIEWHEIGHT-handheight, 0, handheight);
	SPG_DrawPicSkip(&renderBufferText, grsegs[picnum], renderBufferText.Width/2-32, renderBufferText.Height-handheight, 0, handheight);
}

//==========================================================================


