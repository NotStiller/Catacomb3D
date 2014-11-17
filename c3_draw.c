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
#if 0
	fullSetup.BufferStart = NULL;
#endif


void GameWindowResizeHook(int NewWidth, int NewHeight) {
	static int firstcall = 1;
	if (firstcall) {
		firstcall = 0;
		guiBuffer.Buffer = malloc(320*200);
		guiBuffer.Pitch = guiBuffer.Width = 320;
		guiBuffer.Height = 200;
		guiBuffer.Scale = 1;
		guiBuffer.ScreenX = 0;
		guiBuffer.ScreenY = 0;
	} else {
		free(renderBuffer.Buffer);
		free(bottomHUDBuffer.Buffer);
		free(rightHUDBuffer.Buffer);
		renderBuffer.Buffer = NULL;
		renderBufferText.Buffer = NULL;
		bottomHUDBuffer.Buffer = NULL;
		rightHUDBuffer.Buffer = NULL;
	}

	int scale = NewWidth/320, scaleH = NewHeight/200;
	if (scaleH < scale) {
		scale = scaleH;
	}
	if (scale < 1) {
		scale = 1;
		NewWidth = 320;
		NewHeight = 200;
	}

	bottomHUDBuffer.Pitch =
	bottomHUDBuffer.Width = scale*320;
	bottomHUDBuffer.Height = scale*56;
	bottomHUDBuffer.Buffer = malloc(bottomHUDBuffer.Height*bottomHUDBuffer.Pitch);
	bottomHUDBuffer.Scale = scale;
	bottomHUDBuffer.ScreenX = NewWidth-bottomHUDBuffer.Width;
	bottomHUDBuffer.ScreenY = NewHeight-bottomHUDBuffer.Height;

	rightHUDBuffer.Pitch =
	rightHUDBuffer.Width = scale*56;
	rightHUDBuffer.Height = scale*144;
	rightHUDBuffer.Buffer = malloc(rightHUDBuffer.Height*rightHUDBuffer.Pitch);
	rightHUDBuffer.Scale = scale;
	rightHUDBuffer.ScreenX = NewWidth-rightHUDBuffer.Width;
	rightHUDBuffer.ScreenY = NewHeight-scale*200;

	renderBuffer.Pitch = 
	renderBuffer.Width = (NewWidth-rightHUDBuffer.Width)/2*2;
	renderBuffer.Height = (NewHeight-bottomHUDBuffer.Height)/2*2;
	renderBuffer.Buffer = malloc(renderBuffer.Pitch*renderBuffer.Height);
	renderBuffer.Scale = 1; // should never be used
	renderBuffer.ScreenX = 0;
	renderBuffer.ScreenY = 0;

	renderBufferText.Width = renderBuffer.Width/scale;
	renderBufferText.Height = renderBuffer.Height/scale;
	renderBufferText.Pitch = renderBuffer.Pitch;
	renderBufferText.Buffer = renderBuffer.Buffer;
	renderBufferText.Scale = scale;
	renderBufferText.ScreenX = -1; // will not be blitted
	renderBufferText.ScreenY = -1;

	SPG_SetupRenderer(renderBuffer.Width, renderBuffer.Height, renderBuffer.Buffer, renderBuffer.Pitch); 
}

void FlipBuffer(void) {
	if (!SP_GameActive()) {
		SPG_ClearScaleAndFlip(0, &guiBuffer);
	} else {
		BufferSetup *bufs[4] = {&renderBuffer, &rightHUDBuffer, &bottomHUDBuffer, NULL};
		SPG_ClearBlitAndFlip(-1, bufs);
	}
}

void FizzleFade(void) {
	if (!SP_GameActive()) {
		SPG_ClearScaleAndFizzle(0, &guiBuffer);
	} else {
		BufferSetup *bufs[4] = {&renderBuffer, &rightHUDBuffer, &bottomHUDBuffer, NULL};
		SPG_ClearBlitAndFizzle(-1, bufs);
	}
}


void	DrawHand (void)
{
	int	picnum;

	picnum = HAND1PICM;
	if (gamestate.shotpower || boltsleft)
		picnum += (((unsigned)SP_TimeCount()>>3)&1);

	SPG_DrawPicSkip(&renderBuffer, grsegs[picnum], renderBuffer.Width/2-32, renderBuffer.Height-handheight, 0, handheight);
}

