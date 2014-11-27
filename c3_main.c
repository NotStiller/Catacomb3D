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

// C3_MAIN.C

#include "c3_def.h"

int EnableC4Features=0;
int	showscorebox;
boolean		compatability;

/*
=============================================================================

						   CATACOMB 3-D

					  An Id Software production

						   by John Carmack

=============================================================================
*/

/*
==========================
=
= InitGame
=
= Load a few things right away
=
==========================
*/

void InitGame (void)
{
	int			i;

	US_Setup ();

//
// load in and lock down some basic chunks
//

	SPD_LoadGrChunk(HAND1PICM);
	SPD_LoadGrChunk(HAND2PICM);
	SPD_LoadGrChunk(ENTERPLAQUEPIC);

	fontcolor = WHITE; 
	SPG_SetBorderColor(3);

	for (i=0;i<NUMTILE16;i++)
	{
		SPD_LoadGrChunk (STARTTILE16+i);
	}
	for (i=FIRSTLATCHPIC+1;i<FIRSTSCALEPIC;i++)
	{
		SPD_LoadGrChunk (i);
	}
}

//===========================================================================

//===========================================================================


//===========================================================================

/*
=====================
=
= DemoLoop
=
=====================
*/

void	DemoLoop (void)
{
//
// main game cycle
//
	while (1)
	{
		SPG_ResizeNow();
		SPG_ClearBuffer(0);
		SPD_LoadGrChunk (TITLEPIC);
		SPG_DrawPic(&guiBuffer, grsegs[TITLEPIC],0,0);
		SPG_FizzleFadeBuffer ();

		if (!SPI_WaitFor(TickBase*3))
		{
			SPG_ResizeNow();
			SPG_ClearBuffer(0);
			SPD_LoadGrChunk (CREDITSPIC);
			SPG_DrawPic(&guiBuffer, grsegs[CREDITSPIC],0,0);
			SPG_FizzleFadeBuffer();

		}

		if (!SPI_WaitFor(TickBase*3))
		{
			SPG_ResizeNow();
			SPG_ClearBuffer(0);
			DrawHighScores ();
			SPG_FizzleFadeBuffer();
			SPI_WaitFor(TickBase*3);
		}

		if (SPI_GetLastKey() != sc_None)
		{
			if (GameLoop())
			{
				SPI_ClearKeysDown();
				DrawHighScores ();
				SPI_ClearKeysDown();
				SPG_FizzleFadeBuffer();
				SPI_WaitFor(TickBase*3);
			}
		}

	}
}

//===========================================================================




