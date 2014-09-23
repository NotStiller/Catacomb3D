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
#pragma hdrstop

int _argc;
char **_argv;


/*
=============================================================================

						   CATACOMB 3-D

					  An Id Software production

						   by John Carmack

=============================================================================
*/

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/


/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

memptr		scalesegs[NUMPICS];
char		str[80],str2[20];
gametype	gamestate;
exittype	playstate;

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/



/*
=====================
=
= NewGame
=
= Set up new game to start from the beginning
=
=====================
*/

void NewGame (void)
{
	memset (&gamestate,0,sizeof(gamestate));
	gamestate.mapon = 0;
	gamestate.body = MAXBODY;
}

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
	word	i,compressed,expanded;
	objtype	*o;
	memptr	bigbuffer;

	if (!CA_FarWrite(file,(void*)&gamestate,sizeof(gamestate)))
		return(false);

	expanded = mapwidth * mapheight * 2;
	MM_GetPtr (&bigbuffer,expanded);

	for (i = 0;i < 3;i+=2)	// Write planes 0 and 2
	{
//
// leave a word at start of compressed data for compressed length
//
		compressed = (unsigned short)CA_RLEWCompress ((uint16_t*)mapheaderseg[loadedmap].mapsegs[i],expanded,((uint16_t*)bigbuffer)+1,RLETAG);

		*(uint16_t*)bigbuffer = compressed;

		if (!CA_FarWrite(file,(void*)bigbuffer,compressed+2) )
		{
			MM_FreePtr (&bigbuffer);
			return(false);
		}
	}

	for (o = player;o;o = o->next)
		if (!CA_FarWrite(file,(void*)o,sizeof(objtype)))
		{
			MM_FreePtr (&bigbuffer);
			return(false);
		}

	MM_FreePtr (&bigbuffer);

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
	unsigned short	i,x,y;
	objtype		*obj,*prev,*next,*followed;
	unsigned short	compressed,expanded;
	unsigned short	*map,tile;
	memptr		bigbuffer;

	if (!CA_FarRead(file,(void*)&gamestate,sizeof(gamestate)))
		return(false);

	SetupGameLevel ();		// load in and cache the base old level

	expanded = mapwidth * mapheight * 2;
	MM_GetPtr (&bigbuffer,expanded);
	loadedmap = gamestate.mapon;

	for (i = 0;i < 3;i+=2)	// Read planes 0 and 2
	{
		if (!CA_FarRead(file,(void*)&compressed,sizeof(compressed)) )
		{
			MM_FreePtr (&bigbuffer);
			return(false);
		}

		if (!CA_FarRead(file,(void*)bigbuffer,compressed) )
		{
			MM_FreePtr (&bigbuffer);
			return(false);
		}

		CA_RLEWexpand ((uint16_t*)bigbuffer,
			(uint16_t*)mapheaderseg[loadedmap].mapsegs[i],expanded,RLETAG);
	}

	MM_FreePtr (&bigbuffer);
//
// copy the wall data to a data segment array again, to handle doors and
// bomb walls that are allready opened
//
	memset (tilemap,0,sizeof(tilemap));
	memset (actorat,0,sizeof(actorat));
	map = mapheaderseg[loadedmap].mapsegs[0];
	for (y=0;y<mapheight;y++)
		for (x=0;x<mapwidth;x++)
		{
			tile = *map++;
			if (tile<NUMFLOORS)
			{
				tilemap[x][y] = tile;
				if (tile>0)
					CASTAT(intptr_t, actorat[x][y]) = tile;
			}
		}


	// Read the object list back in - assumes at least one object in list

	InitObjList ();
	new = player;
	while (true)
	{
		prev = new->prev;
		next = new->next;
		if (!CA_FarRead(file,(void*)new,sizeof(objtype)))
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

	return(true);
}

//===========================================================================

/*
==================
=
= ResetGame
=
==================
*/

void ResetGame(void)
{
	NewGame ();
}

//===========================================================================


/*
==========================
=
= ShutdownId
=
= Shuts down all ID_?? managers
=
==========================
*/

void ShutdownId (void)
{
  US_Shutdown ();
  SD_Shutdown ();
  IN_Shutdown ();
}


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
	unsigned	segstart,seglength;
	int			i,x,y;
	unsigned	*blockstart;

//	US_TextScreen();

	IN_Startup ();
	SD_Startup ();
	US_Startup ();

//	US_UpdateTextScreen();

	US_Setup ();

	US_SetLoadSaveHooks(LoadTheGame,SaveTheGame,ResetGame);

//
// load in and lock down some basic chunks
//

	CA_CacheGrChunk(STARTFONT);
	CA_CacheGrChunk(STARTTILE8);
	CA_CacheGrChunk(STARTTILE8M);
	CA_CacheGrChunk(HAND1PICM);
	CA_CacheGrChunk(HAND2PICM);
	CA_CacheGrChunk(ENTERPLAQUEPIC);

	fontcolor = WHITE;


//
// build some tables
//
	for (i=0;i<PORTTILESHIGH;i++)
		uwidthtable[i] = UPDATEWIDE*i;

	blockstart = &blockstarts[0];
	for (y=0;y<UPDATEHIGH;y++)
		for (x=0;x<UPDATEWIDE;x++)
			*blockstart++ = SCREENWIDTH*16*y+x*TILEWIDTH;

	BuildTables ();			// 3-d tables

	SetupScaling ();

	VW_ClearVideo (BLACK);

//
// initialize variables
//
	updateptr = &update[0];
	*(unsigned *)(updateptr + UPDATEWIDE*PORTTILESHIGH) = UPDATETERMINATE;
}

//===========================================================================

void clrscr (void);		// can't include CONIO.H because of name conflicts...

/*
==========================
=
= Quit
=
==========================
*/

void Quit (char *error)
{
	unsigned	finscreen;


	ShutdownId ();
	if (error && *error)
	{
	  puts(error);
	  exit(1);
	}

	SP_Exit();
}

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
	VW_Bar (0,0,320,200,0);

	while (1)
	{
		CA_CacheGrChunk (TITLEPIC);
		SPG_DrawPic(grsegs[TITLEPIC],0,0);
		FizzleFade (320,200,true);

		if (!IN_UserInput(TickBase*3,false))
		{
			CA_CacheGrChunk (CREDITSPIC);
			SPG_DrawPic(grsegs[CREDITSPIC],0,0);
			FizzleFade (320,200,true);

		}

		if (!IN_UserInput(TickBase*3,false))
		{
			DrawHighScores ();
			FizzleFade (320,200,true);
			IN_UserInput(TickBase*3,false);
		}

		if (IN_IsUserInput())
		{
			US_ControlPanel ();

			if (restartgame || loadedgame)
			{
				GameLoop ();
				DrawHighScores ();
				FizzleFade (320,200,true);
				IN_UserInput(TickBase*3,false);
			}
		}

	}
}

//===========================================================================

//===========================================================================

//===========================================================================

/*
==========================
=
= SetupScaling
=
==========================
*/

void SetupScaling (void)
{
	int		i,x,y;
	byte	*dest;

//
// build the compiled scalers
//
	for (i=1;i<=VIEWWIDTH/2;i++)
		BuildCompScale (i*2,&scaledirectory[i]);
}

//===========================================================================

int	showscorebox;
boolean		compatability;

void RF_FixOfs (void)
{
}

void HelpScreens (void)
{
}


/*
==================
=
= CheckMemory
=
==================
*/

void	CheckMemory(void)
{
}

