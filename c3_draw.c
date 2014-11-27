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

#include "c3_def.h"

BufferSetup renderBuffer, renderBufferText;
BufferSetup rightHUDBuffer,
			bottomHUDBuffer;

const int hudWidth=7*8, hudHeight=7*8;


void GameWindowResizeHook(int NewWidth, int NewHeight) {
	static int firstcall = 1;

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
	bottomHUDBuffer.Height = 56;
	bottomHUDBuffer.Scale = scale;
	bottomHUDBuffer.ScreenX = NewWidth-scale*bottomHUDBuffer.Width;
	bottomHUDBuffer.ScreenY = NewHeight-scale*bottomHUDBuffer.Height;

	rightHUDBuffer.Width = 56;
	rightHUDBuffer.Height = 144;
	rightHUDBuffer.Scale = scale;
	rightHUDBuffer.ScreenX = NewWidth-scale*rightHUDBuffer.Width;
	rightHUDBuffer.ScreenY = NewHeight-scale*200;

	guiBuffer.Width = 320;
	guiBuffer.Height = 200;
	guiBuffer.Scale = scale;
	guiBuffer.ScreenX = (NewWidth-scale*320)/2;
	guiBuffer.ScreenY = (NewHeight-scale*200)/2;

	renderBuffer.Width = (NewWidth-scale*rightHUDBuffer.Width)/2*2;
	renderBuffer.Height = (NewHeight-scale*bottomHUDBuffer.Height)/2*2;
	renderBuffer.Scale = 1; // should never be used
	renderBuffer.ScreenX = (NewWidth-scale*rightHUDBuffer.Width)&1;
	renderBuffer.ScreenY = (NewHeight-scale*bottomHUDBuffer.Height)&1;

	renderBufferText.Width = renderBuffer.Width/scale;
	renderBufferText.Height = renderBuffer.Height/scale;
	renderBufferText.Scale = scale;
	renderBufferText.ScreenX = renderBuffer.ScreenX+renderBuffer.Width%scale; // will not be blitted
	renderBufferText.ScreenY = renderBuffer.ScreenY+renderBuffer.Height%scale;

	SPG_SetupRenderer(&renderBuffer); 
}

void	DrawHand (void)
{
	int	picnum;

	picnum = HAND1PICM;
	if (gamestate->shotpower || boltsleft)
		picnum += (((unsigned)SP_TimeCount()>>3)&1);

	SPG_DrawPicSkip(&renderBufferText, grsegs[picnum], renderBufferText.Width/2-32, renderBufferText.Height-handheight, 0, handheight);
}

