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

void DrawPlayScreen (void);


//
// near data map array (wall values only, get text number from far data)
//


int bordertime;

void StopMusic(void);
void StartMusic(void);



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

void CheckMenu (void)
{
	if (SP_LastScan() == sc_Escape)
	{
		SP_GameLeave();
		StopMusic ();
		US_ControlPanel();
		if (abortgame)
		{
			playstate = ex_abort;
			return;
		}
		StartMusic ();
		IN_ClearKeysDown();
		if (restartgame)
			playstate = ex_resetgame;
		if (loadedgame)
			playstate = ex_loadedgame;
		DrawPlayScreen ();
		lasttimecount = SP_TimeCount();
		MouseDelta(NULL, NULL);	// Clear accumulated mouse movement
		SP_GameEnter();
	}
}


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

	IN_ReadControl(0,&control);

	MouseButtons(&buttons);

	if (buttons&1)
		control.button0 = 1;
	if (buttons&2)
		control.button1 = 1;

	{
		if (SP_Keyboard(sc_RShift)||SP_Keyboard(sc_LShift))
			running = true;
		else
			running = false;
	}
}

//==========================================================================

/*
=================
=
= StopMusic
=
=================
*/

void StopMusic(void)
{
	int	i;

	SD_MusicOff();
}

//==========================================================================


/*
=================
=
= StartMusic
=
=================
*/

// JAB - Cache & start the appropriate music for this level
void StartMusic(void)
{
	musicnames	chunk;

	SD_MusicOff();
	SD_StartMusic(TOOHOT_MUS);
}

//==========================================================================


/*
===================
=
= PlayLoop
=
===================
*/

void PlayLoop (void)
{
	int		give;

	void (*think)();

	ingame = true;
	playstate = 0;
	gamestate.shotpower = handheight = 0;
	pointcount = pointsleft = 0;

	DrawLevelNumber (gamestate.mapon);
	DrawBars ();

	lasttimecount = SP_TimeCount();
	lastnuke = 0;

	PollControls ();				// center mouse
	StartMusic ();
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
				SD_PlaySound (GETPOINTSSND);
				AddPoints (give);
				pointsleft -= give;
				if (!pointsleft)
					pointcount = 0;
			}
		}

		ThreeDRefresh ();
		if (SPG_PollRedraw()) {
			DrawPlayScreen ();
		}
		CheckMenu();
	} while (!playstate);
	StopMusic ();

	ingame = false;
	if (bordertime)
	{
		bordertime = 0;
		SPG_SetBorderColor(3);
	}

	if (!abortgame)
		AddPoints (pointsleft);
	else
		abortgame = false;
}

