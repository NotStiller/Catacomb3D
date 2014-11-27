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

// C3_PLAY.C

#include "c3_def.h"



/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

#define POINTTICS	6


/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

boolean		godmode;
int			bordertime;
objtype *player;
gametype *gamestate;


//
// replacing refresh manager
//
unsigned	tics,realtics;
int		pointcount,pointsleft;

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

static PlayLoopExitType playstate;

void DrawPlayScreen (void);


//
// near data map array (wall values only, get text number from far data)
//


int bordertime;




//==========================================================================


//==========================================================================

///////////////////////////////////////////////////////////////////////////
//
//	CenterWindow() - Generates a window of a given width & height in the
//		middle of the screen
//
///////////////////////////////////////////////////////////////////////////

#define MAXX	264
#define MAXY	146

void	CenterWindow(word w,word h)
{
	US_DrawWindow(((MAXX / 8) - w) / 2,((MAXY / 8) - h) / 2,w,h);
}

//===========================================================================


/*
=====================
=
= CheckKeys
=
=====================
*/

#if 0
void CheckMenu (void)
{
	if (SPI_GetLastKey() == sc_Escape)
	{
		SP_GameLeave();
		SPA_MusicOff();
		ControlPanelExitType exit;
		exit = US_ControlPanel(true);
		SP_GameEnter();
		if (exit.Result == CPE_ABORTGAME) {
			playstate = ex_abort;
			return;
		}
		SPA_StartMusic(TOOHOT_MUS);
		SPI_ClearKeysDown();
		if (exit.Result == CPE_NEWGAME)
			playstate = ex_resetgame;
		else if (exit.Result == CPE_LOADEDGAME)
			playstate = ex_loadedgame;
		else if (exit.Result == CPE_NOTHING) {
			DrawPlayScreen ();
		} else {
			assert(0);
		}
		lasttimecount = SP_TimeCount();
		SPI_GetMouseDelta(NULL, NULL);	// Clear accumulated mouse movement
	}
}
#endif 

//===========================================================================

//==========================================================================

/*
===================
=
= PollControls
=
===================
*/

void PollControls (void)
{
	unsigned buttons;

	SPI_GetPlayerControl(&control);
}

//==========================================================================

/*
===================
=
= DrawPlayScreen
=
===================
*/

void DrawPlayScreen (void)
{
	SPG_ClearBuffer(-1);
	SPD_LoadGrChunk (STATUSPIC);
	SPD_LoadGrChunk (SIDEBARSPIC);

	SPG_DrawPic(&bottomHUDBuffer, grsegs[STATUSPIC],0,0);
	SPG_DrawPic(&rightHUDBuffer, grsegs[SIDEBARSPIC],0,0);

	RedrawStatusWindow ();
}

//==========================================================================

static  char    *levelnames[] =
				{
					"The Approach",
					"Nemesis's Keep",
					"Ground Floor",
					"Second Floor",
					"Third Floor",
					"Tower One",
					"Tower Two",
					"Secret Halls",
					"Access Floor",
					"The Dungeon",
					"Lower Dungeon",
					"Catacomb",
					"Lower Reaches",
					"The Warrens",
					"Hidden Caverns",
					"The Fens of Insanity",
					"Chaos Corridors",
					"The Labyrinth",
					"Halls of Blood",
					"Nemesis's Lair"
				};
void DrawEnterScreen (char *Name, boolean Fizzle)
{
	int     x,y;

	SPG_Bar(&renderBuffer, 0,0,renderBuffer.Width, renderBuffer.Height, 9);

	x = (renderBufferText.Width-18*8)/2;
	y = (renderBufferText.Height-5*8)/2;
	SPG_DrawPic(&renderBufferText, grsegs[ENTERPLAQUEPIC],x,y);

	int width;
	SPG_MeasureString(Name, 0, &width, NULL);
	SPG_DrawString(&renderBufferText, x+(18*8-width)/2, y+23, Name, 0, 8);

	SPI_ClearKeysDown();
	if (Fizzle) {
		SPG_FizzleFadeBuffer();
	} else {
		SPG_FlipBuffer();
	}
	SPI_WaitFor(35);
	SPI_ClearKeysDown ();
	fizzlein = true;
	if (0) {
		SPI_WaitForever();
	}
}



//==========================================================================

void PlayLoop_Died(void) {
	if (PlayLoop_IsDone()) {
		return;
	}
	playstate = PLE_DEATH;
}

boolean PlayLoop_IsDone(void) {
	return playstate != PLE_STILLPLAYING;
}

void PlayLoop_Victory(void) {
	if (PlayLoop_IsDone()) {
		return;
	}
	playstate = PLE_VICTORY;
}

void PlayLoop_Warp(int NewMap) {
	if (PlayLoop_IsDone()) {
		return;
	}
	playstate = PLE_WARPED;
	gamestate->mapon = NewMap;
}


/*
===================
=
= PlayLoop
=
===================
*/


PlayLoopExitType PlayLoop (gametype *Gamestate, boolean DoDrawEnterScreen)
{
	int		give;

	gamestate = Gamestate;
	if (DoDrawEnterScreen) {
		DrawPlayScreen ();
		DrawEnterScreen (levelnames[gamestate->mapon], true);
	}

	void (*think)();

	playstate = PLE_STILLPLAYING;
	lasttimecount = SP_TimeCount();

	if (bordertime) {
		SPG_SetBorderColor(FLASHCOLOR);
	}
	SPA_StartMusic(TOOHOT_MUS);
	do {
		PollControls();

		objtype *obj;
		for (obj = player;obj;obj = obj->next) {
			if (obj->active) {
				if (obj->ticcount) {
					obj->ticcount-=tics;
					while ( obj->ticcount <= 0) {
						think = obj->state->think;
						if (think) {
							think (obj);
							if (!obj->state) {
								RemoveObj (obj);
								goto nextactor;
							}
						}
	
						obj->state = obj->state->next;
						if (!obj->state) {
							RemoveObj (obj);
							goto nextactor;
						}
						if (!obj->state->tictime) {
							obj->ticcount = 0;
							goto nextactor;
						}
						if (obj->state->tictime>0)
							obj->ticcount += obj->state->tictime;
					}
				}
				think =	obj->state->think;
				if (think) {
					think (obj);
					if (!obj->state)
						RemoveObj (obj);
				}
			} // if (obj->active)
			nextactor:;
		} // for obj

		if (bordertime)
		{
			bordertime -= tics;
			if (bordertime<=0) {
				bordertime = 0;
				SPG_SetBorderColor(3);
			}
		}
		if (pointcount) {
			pointcount -= tics;
			if (pointcount <= 0) {
				pointcount += POINTTICS;
				give = (pointsleft > 1000)? 1000 : ((pointsleft > 100)? 100 : ((pointsleft < 20)? pointsleft : 20));
				SPA_PlaySound (GETPOINTSSND);
				AddPoints (give);
				pointsleft -= give;
				if (!pointsleft)
					pointcount = 0;
			}
		}

		SPG_ResizeNow();
		DrawPlayScreen ();
		ThreeDRefresh(0,8);
//		CheckMenu();
		if (SPI_GetLastKey() == sc_Escape) {
			playstate = PLE_MENU;
		}
	} while (!playstate);
	SPA_MusicOff();
	
	SPG_SetBorderColor(3);

	if (playstate == PLE_WARPED || playstate == PLE_DEATH || playstate == PLE_VICTORY) {
		AddPoints (pointsleft);
	}

	return playstate;
}

