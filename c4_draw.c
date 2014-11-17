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
	static int firstcall = 1;
	if (firstcall) {
		firstcall = 0;
	} else {
		free(renderBuffer.Buffer);
		free(bottomHUDBuffer.Buffer);
		renderBuffer.Buffer = NULL;
		renderBufferText.Buffer = NULL;
		bottomHUDBuffer.Buffer = NULL;
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
	bottomHUDBuffer.Height = scale*80;
	bottomHUDBuffer.Buffer = malloc(bottomHUDBuffer.Height*bottomHUDBuffer.Pitch);
	bottomHUDBuffer.Scale = scale;
	bottomHUDBuffer.ScreenX = (NewWidth-bottomHUDBuffer.Width)/2;
	bottomHUDBuffer.ScreenY = NewHeight-bottomHUDBuffer.Height;

	renderBuffer.Pitch = 
	renderBuffer.Width = (NewWidth)/2*2;
	renderBuffer.Height = (NewHeight-scale*80)/2*2;
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
	BufferSetup *bufs[] = {&renderBuffer, &bottomHUDBuffer, NULL};
	SPG_ClearBlitAndFlip(0, bufs);
}


void FizzleFade(void) {
	BufferSetup *bufs[] = {&renderBuffer, &bottomHUDBuffer, NULL};
	SPG_ClearBlitAndFizzle(0, bufs);
}

void	DrawHand (void)
{
	int	picnum;

	picnum = HAND1PICM;
//	SPG_DrawMaskedPicSkip(grsegs[picnum], ((VIEWWIDTH/16)-(10/2)), VIEWHEIGHT-handheight, 0, handheight);
	SPG_DrawPicSkip(&renderBuffer, grsegs[picnum], renderBuffer.Width/2-32, renderBuffer.Height-handheight, 0, handheight);
}

//==========================================================================


