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

ControlInfo	c;
boolean		running,slowturn;

int			bordertime;
objtype objlist[MAXACTORS],*new,*obj,*player,*lastobj,*objfreelist;

boolean		godmode;

//
// replacing refresh manager
//
unsigned	mapwidth,mapheight,tics,realtics;
boolean		compatability;
unsigned	mapwidthtable[64];

int		mousexmove,mouseymove;
int		pointcount,pointsleft;

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

void CalcBounds (objtype *ob);
void DrawPlayScreen (void);


//
// near data map array (wall values only, get text number from far data)
//
byte		tilemap[MAPSIZE][MAPSIZE];
byte		spotvis[MAPSIZE][MAPSIZE];
objtype		*actorat[MAPSIZE][MAPSIZE];

objtype dummyobj;

int bordertime;
int	objectcount;

void StopMusic(void);
void StartMusic(void);


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

/*
#############################################################################

				  The objlist data structure

#############################################################################

objlist containt structures for every actor currently playing.  The structure
is accessed as a linked list starting at *player, ending when ob->next ==
NULL.  GetNewObj inserts a new object at the end of the list, meaning that
if an actor spawn another actor, the new one WILL get to think and react the
same frame.  RemoveObj unlinks the given object and returns it to the free
list, but does not damage the objects ->next pointer, so if the current object
removes itself, a linked list following loop can still safely get to the
next element.

<backwardly linked free list>

#############################################################################
*/


/*
=========================
=
= InitObjList
=
= Call to clear out the entire object list, returning them all to the free
= list.  Allocates a special spot for the player.
=
=========================
*/

void InitObjList (void)
{
	int	i;

	for (i=0;i<MAXACTORS;i++)
	{
		objlist[i].prev = &objlist[i+1];
		objlist[i].next = NULL;
	}

	objlist[MAXACTORS-1].prev = NULL;

	objfreelist = &objlist[0];
	lastobj = NULL;

	objectcount = 0;

//
// give the player and score the first free spots
//
	GetNewObj (false);
	player = new;
}

//===========================================================================

/*
=========================
=
= GetNewObj
=
= Sets the global variable new to point to a free spot in objlist.
= The free spot is inserted at the end of the liked list
=
= When the object list is full, the caller can either have it bomb out ot
= return a dummy object pointer that will never get used
=
=========================
*/
int numobjects;
void GetNewObj (boolean usedummy)
{
	if (!objfreelist)
	{
		if (usedummy)
		{
			new = &dummyobj;
			return;
		}
		Quit ("GetNewObj: No free spots in objlist!");
	}

	new = objfreelist;
	objfreelist = new->prev;
	memset (new,0,sizeof(*new));
	new->next = NULL;

	if (lastobj)
		lastobj->next = new;
	new->prev = lastobj;	// new->next is allready NULL from memset

	new->active = false;
	lastobj = new;

	objectcount++;
	numobjects++;
}

//===========================================================================

/*
=========================
=
= RemoveObj
=
= Add the given object back into the free list, and unlink it from it's
= neighbors
=
=========================
*/

void RemoveObj (objtype *gone)
{

	objtype **spotat;

	if (gone == player)
		Quit ("RemoveObj: Tried to remove the player!");
	numobjects--;

//
// fix the next object's back link
//
	if (gone == lastobj)
		lastobj = (objtype *)gone->prev;
	else
		gone->next->prev = gone->prev;

//
// fix the previous object's forward link
//
	gone->prev->next = gone->next;

//
// add it back in to the free list
//
	gone->prev = objfreelist;
	objfreelist = gone;
}

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

	IN_ReadControl(0,&c);

	MouseButtons(&buttons);
	MouseDelta(&mousexmove, &mouseymove);

	if (buttons&1)
		c.button0 = 1;
	if (buttons&2)
		c.button1 = 1;

	{
		if (SP_Keyboard(sc_RShift)||SP_Keyboard(sc_LShift))
			running = true;
		else
			running = false;
		if (c.button0)
			slowturn = true;
		else
			slowturn = false;
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
	SP_SetTimeCount(0);
	playstate = 0;
	gamestate.shotpower = handheight = 0;
	pointcount = pointsleft = 0;

	DrawLevelNumber (gamestate.mapon);
	DrawBars ();

	lasttimecount = lastnuke = 0;
	SP_SetTimeCount(0);

	PollControls ();				// center mouse
	StartMusic ();
	do
	{
		PollControls();

		for (obj = player;obj;obj = obj->next) {
			if (obj->active)
			{
				if (obj->ticcount)
				{
					obj->ticcount-=tics;
					while ( obj->ticcount <= 0)
					{
						think = obj->state->think;
						if (think)
						{
							think (obj);
							if (!obj->state)
							{
								RemoveObj (obj);
								goto nextactor;
							}
						}

						obj->state = obj->state->next;
						if (!obj->state)
						{
							RemoveObj (obj);
							goto nextactor;
						}
						if (!obj->state->tictime)
						{
							obj->ticcount = 0;
							goto nextactor;
						}
						if (obj->state->tictime>0)
							obj->ticcount += obj->state->tictime;
					}
				}
				think =	obj->state->think;
				if (think)
				{
					think (obj);
					if (!obj->state)
						RemoveObj (obj);
				}
			}
			nextactor:;
		}

		if (bordertime)
		{
			bordertime -= tics;
			if (bordertime<=0)
			{
				bordertime = 0;
				SPG_SetBorderColor(3);
			}
		}
		if (pointcount)
		{
			pointcount -= tics;
			if (pointcount <= 0)
			{
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

