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

// C3_MAIN.C
#define CATALOG


#include <time.h>
#include <stdarg.h>

#include "c4_def.h"
#include "gelib.h"

/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

int EnableC4Features=1;

GameDiff restartgame;
boolean loadedgame,abortgame,ingame;

gametype	*gamestate;
exittype	playstate;
boolean EASYMODEON;

unsigned Flags=0;

void DisplayIntroText(void);

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/


//===========================================================================

#define RLETAG	0xABCD

/*
==================
=
= SaveTheGame
=
==================
*/

boolean	SaveTheGame(FILE *file)
{
#if 0
	word	i,compressed,expanded;
	objtype	*o;
	memptr	bigbuffer;

	if (!CA_FarWrite(file,(void *)&FreezeTime,sizeof(FreezeTime)))
		return(false);

	if (!CA_FarWrite(file,(void *)&gamestate,sizeof(gamestate)))
		return(false);

	if (!CA_FarWrite(file,(void *)&EASYMODEON,sizeof(EASYMODEON)))
		return(false);

	expanded = mapwidth * mapheight * 2;
	MM_GetPtr (&bigbuffer,expanded);

	for (i = 0;i < 3;i+=2)	// Write planes 0 and 2
	{
//
// leave a word at start of compressed data for compressed length
//
		compressed = (unsigned)CA_RLEWCompress ((unsigned *)gamestate->mapsegs[i]
			,expanded,((unsigned *)bigbuffer)+1,RLETAG);

		*(unsigned *)bigbuffer = compressed;

		if (!CA_FarWrite(file,(void *)bigbuffer,compressed+2) )
		{
			MM_FreePtr (&bigbuffer);
			return(false);
		}
	}

	for (o = player;o;o = o->next)
		if (!CA_FarWrite(file,(void *)o,sizeof(objtype)))
		{
			MM_FreePtr (&bigbuffer);
			return(false);
		}

	MM_FreePtr (&bigbuffer);
#endif
	return(true);
}

//===========================================================================

/*
==================
=
= LoadTheGame
=
==================
*/

boolean	LoadTheGame(FILE *file)
{
#if 0
	unsigned	i,x,y;
	objtype		*obj,*prev,*next,*followed;
	unsigned	compressed,expanded;
	unsigned	*map,tile;
	memptr		bigbuffer;

	playstate = ex_loadedgame;
	if (!CA_FarRead(file,(void *)&FreezeTime,sizeof(FreezeTime)))
		return(false);

	if (!CA_FarRead(file,(void *)&gamestate,sizeof(gamestate)))
		return(false);

	if (!CA_FarRead(file,(void *)&EASYMODEON,sizeof(EASYMODEON)))
		return(false);

	SetupGameLevel ();		// load in and cache the base old level

	if (!FindFile(Filename,"SAVE GAME",-1))
		Quit("Error: Can't find saved game file!");

	expanded = mapwidth * mapheight * 2;
	MM_GetPtr (&bigbuffer,expanded);

	for (i = 0;i < 3;i+=2)	// Read planes 0 and 2
	{
		if (!CA_FarRead(file,(void *)&compressed,sizeof(compressed)) )
		{
			MM_FreePtr (&bigbuffer);
			return(false);
		}

		if (!CA_FarRead(file,(void *)bigbuffer,compressed) )
		{
			MM_FreePtr (&bigbuffer);
			return(false);
		}

		CA_RLEWexpand ((unsigned *)bigbuffer,
			(unsigned *)gamestate->mapsegs[i],expanded,RLETAG);
	}

	MM_FreePtr (&bigbuffer);
//
// copy the wall data to a data segment array again, to handle doors and
// bomb walls that are allready opened
//
	ClearTileMap();
	ClearActorAt();
	map = gamestate->mapsegs[0];
	for (y=0;y<mapheight;y++)
		for (x=0;x<mapwidth;x++)
		{
			tile = *map++;
			if (tile<NUMFLOORS)
			{
				tilemap[x][y] = tile;
				if (tile>0)
					CASTAT(unsigned,actorat[x][y]) = tile;
			}
		}


	// Read the object list back in - assumes at least one object in list

	InitObjList ();
	new = player;
	while (true)
	{
		prev = new->prev;
		next = new->next;
		if (!CA_FarRead(file,(void *)new,sizeof(objtype)))
			return(false);
		followed = new->next;
		new->prev = prev;
		new->next = next;
		actorat[new->tilex][new->tiley] = new;	// drop a new marker

		if (followed)
			GetNewObj (false);
		else
			break;
	}
#endif
	return(true);
}

//===========================================================================

//===========================================================================


//===========================================================================

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
//
// load in and lock down some basic chunks
//
	SPA_SetSoundSource(SND_ADLIB);

	SPD_LoadGrChunk(STARTFONT);
	SPD_LoadGrChunk(STARTTILE8);
	SPD_LoadGrChunk(STARTTILE8M);
	assert(grsegs[STARTTILE8M] != NULL);
	SPD_LoadGrChunk(HAND1PICM);
	SPD_LoadGrChunk(NORTHICONSPR);

	fontcolor = WHITE;


	int i;
	for (i=0;i<NUMTILE16;i++)
	{
		SPD_LoadGrChunk (STARTTILE16+i);
	}

	for (i=FIRSTLATCHPIC+1;i<FIRSTSCALEPIC;i++)
	{
		SPD_LoadGrChunk (i);
	}

	gamestate = malloc(sizeof(gametype));
	memset(gamestate, 0, sizeof(gametype));
}

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
/////////////////////////////////////////////////////////////////////////////
// main game cycle
/////////////////////////////////////////////////////////////////////////////

//	set EASYMODE
//
//		EASYMODEON = true;

// restore game
//
	if (0)
	{
		VW_FadeOut();
		VW_Bar(0,0,320,200,0);
		if (GE_LoadGame())
		{
			loadedgame = true;
			playstate = ex_loadedgame;
//			Keyboard[sc_Enter] = true;
			VW_Bar(0,0,320,200,0);
			ColoredPalette();
		}
		VW_Bar(0,0,320,200,0);
		VW_FadeIn();
	}

	// Play a game
	//
	restartgame = gd_Normal;
	GameLoop();
}

//-------------------------------------------------------------------------
// DisplayIntroText()
//-------------------------------------------------------------------------
void DisplayIntroText()
{
	char *toptext = "You stand before the gate leading into the Towne "
						 "Cemetery. Night is falling as mournful wails mingle "
						 "with the sound of your pounding heart.";

	char *bottomtext = "Equipped with your wits and the Secret Knowledge of "
							 "Magick, you venture forth on your quest to upset "
							 "the dark schemes of Nemesis, your arch rival.";

	char oldfontcolor=fontcolor;

	fontcolor = 14;
	WindowX=WindowY=0;
	PPT_RightEdge=319;
	PPT_LeftEdge=0;

	PrintY = 1;
	PrintPropText(toptext);

	PrintY = 160;
	PrintPropText(bottomtext);

	fontcolor = oldfontcolor;
}

//===========================================================================


