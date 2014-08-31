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

	if (!CA_FarWrite(file,(void far *)&gamestate,sizeof(gamestate)))
		return(false);

	expanded = mapwidth * mapheight * 2;
	MM_GetPtr (&bigbuffer,expanded);

	for (i = 0;i < 3;i+=2)	// Write planes 0 and 2
	{
//
// leave a word at start of compressed data for compressed length
//
		compressed = (unsigned short)CA_RLEWCompress ((unsigned short *)mapsegs[i]
			,expanded,((unsigned short*)bigbuffer)+1,RLETAG);

		*(unsigned short *)bigbuffer = compressed;

		if (!CA_FarWrite(file,(void far *)bigbuffer,compressed+2) )
		{
			MM_FreePtr (&bigbuffer);
			return(false);
		}
	}

	for (o = player;o;o = o->next)
		if (!CA_FarWrite(file,(void far *)o,sizeof(objtype)))
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
	unsigned short	far *map,tile;
	memptr		bigbuffer;

	if (!CA_FarRead(file,(void far *)&gamestate,sizeof(gamestate)))
		return(false);

	SetupGameLevel ();		// load in and cache the base old level

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

		CA_RLEWexpand ((unsigned short *)bigbuffer,
			(unsigned short*)mapsegs[i],expanded,RLETAG);
	}

	MM_FreePtr (&bigbuffer);
//
// copy the wall data to a data segment array again, to handle doors and
// bomb walls that are allready opened
//
	memset (tilemap,0,sizeof(tilemap));
	memset (actorat,0,sizeof(actorat));
	map = mapsegs[0];
	for (y=0;y<mapheight;y++)
		for (x=0;x<mapwidth;x++)
		{
			tile = *map++;
			if (tile<NUMFLOORS)
			{
				tilemap[x][y] = tile;
				if (tile>0)
					*(unsigned*)&(actorat[x][y]) = tile;
			}
		}


	// Read the object list back in - assumes at least one object in list

	InitObjList ();
	new = player;
	while (true)
	{
		prev = new->prev;
		next = new->next;
		if (!CA_FarRead(file,(void far *)new,sizeof(objtype)))
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

	ca_levelnum--;
	ca_levelbit>>=1;
	CA_ClearMarks();
	ca_levelbit<<=1;
	ca_levelnum++;
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
  CA_Shutdown ();
  MM_Shutdown ();
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

	MM_Startup ();
	VW_Startup ();
	IN_Startup ();
	SD_Startup ();
	US_Startup ();

//	US_UpdateTextScreen();

	CA_Startup ();
	US_Setup ();

	US_SetLoadSaveHooks(LoadTheGame,SaveTheGame,ResetGame);

//
// load in and lock down some basic chunks
//

	CA_ClearMarks ();

	CA_MarkGrChunk(STARTFONT);
	CA_MarkGrChunk(STARTTILE8);
	CA_MarkGrChunk(STARTTILE8M);
	CA_MarkGrChunk(HAND1PICM);
	CA_MarkGrChunk(HAND2PICM);
	CA_MarkGrChunk(ENTERPLAQUEPIC);

	CA_CacheMarks (NULL);

	MM_SetLock (&grsegs[STARTFONT],true);
	MM_SetLock (&grsegs[STARTTILE8],true);
	MM_SetLock (&grsegs[STARTTILE8M],true);
	MM_SetLock (&grsegs[HAND1PICM],true);
	MM_SetLock (&grsegs[HAND2PICM],true);
	MM_SetLock (&grsegs[ENTERPLAQUEPIC],true);

	fontcolor = WHITE;


//
// build some tables
//
	for (i=0;i<MAPSIZE;i++)
		nearmapylookup[i] = &tilemap[0][0]+MAPSIZE*i;

	for (i=0;i<PORTTILESHIGH;i++)
		uwidthtable[i] = UPDATEWIDE*i;

	blockstart = &blockstarts[0];
	for (y=0;y<UPDATEHIGH;y++)
		for (x=0;x<UPDATEWIDE;x++)
			*blockstart++ = SCREENWIDTH*16*y+x*TILEWIDTH;

	BuildTables ();			// 3-d tables

	SetupScaling ();

	VW_ColorBorder (3);
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

#if 0
	if (!error)
	{
		CA_SetAllPurge ();
		CA_CacheGrChunk (PIRACY);
		finscreen = (unsigned)grsegs[PIRACY];
	}
#endif

	ShutdownId ();
	if (error && *error)
	{
	  puts(error);
	  exit(1);
	}

#if 0
	if (!NoWait)
	{
		movedata (finscreen,0,0xb800,0,4000);
		bioskey (0);
		clrscr();
	}
#endif

	BE_Exit();
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
		VWB_DrawPic (0,0,TITLEPIC);
		MM_SetPurge (&grsegs[TITLEPIC],3);
		UNMARKGRCHUNK(TITLEPIC);
		FizzleFade (320,200,true);

		if (!IN_UserInput(TickBase*3,false))
		{
			CA_CacheGrChunk (CREDITSPIC);
			VWB_DrawPic (0,0,CREDITSPIC);
			MM_SetPurge (&grsegs[CREDITSPIC],3);
			UNMARKGRCHUNK(CREDITSPIC);
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

/*
==========================
=
= SetupScalePic
=
==========================
*/

void SetupScalePic (unsigned short picnum)
{
	unsigned	scnum;

	scnum = picnum-FIRSTSCALEPIC;

	if (shapedirectory[scnum])
	{
		MM_SetPurge ((memptr*)&shapedirectory[scnum],0);
		return;					// allready in memory
	}

	CA_CacheGrChunk (picnum);
	DeplanePic (picnum);
	shapesize[scnum] = BuildCompShape (&shapedirectory[scnum]);
	grneeded[picnum]&= ~ca_levelbit;
	MM_FreePtr (&grsegs[picnum]);
}

//===========================================================================

/*
==========================
=
= SetupScaleWall
=
==========================
*/

void SetupScaleWall (unsigned short picnum)
{
	int		x,y;
	unsigned	scnum;
	byte	far *dest;

	scnum = picnum-FIRSTWALLPIC;

	if (walldirectory[scnum])
	{
		MM_SetPurge (&walldirectory[scnum],0);
		return;					// allready in memory
	}

	CA_CacheGrChunk (picnum);
	DeplanePic (picnum);
	MM_GetPtr(&walldirectory[scnum],64*64);
	dest = (byte*)walldirectory[scnum];
	for (x=0;x<64;x++)
		for (y=0;y<64;y++)
			*dest++ = spotvis[y][x];
	grneeded[picnum]&= ~ca_levelbit;
	MM_FreePtr (&grsegs[picnum]);
}

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
	byte	far *dest;

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

#define MINMEMORY	335000l

void	CheckMemory(void)
{
}

//===========================================================================


/*
==========================
=
= main
=
==========================
*/

int cata3dmain (int argc, char **argv)
{
	short i;

	_argc = argc;
	_argv = argv;
	if (strcmp(_argv[1], "/VER") == 0)
	{
		printf("Catacomb 3-D version 1.22  (Rev 1)\n");
		printf("Copyright 1991-93 Softdisk Publishing\n");
		printf("Developed for use with 100%% IBM compatibles\n");
		printf("that have 640K memory and DOS version 3.3 or later\n");
		printf("and EGA graphics or better.\n");
		exit(0);
	}

	if (strcmp(_argv[1], "/?") == 0)
	{
		printf("Catacomb 3-D version 1.22\n");
		printf("Copyright 1991-93 Softdisk Publishing\n\n");
		printf("Syntax:\n");
		printf("CAT3D [/<switch>]\n\n");
		printf("Switch       What it does\n");
		printf("/?           This Information\n");
		printf("/VER         Display Program Version Information\n");
		printf("/COMP        Fix problems with SVGA screens\n");
		printf("/NOAL        No AdLib or SoundBlaster detection\n");
		printf("/NOJOYS      Tell program to ignore joystick\n");
		printf("/NOMOUSE     Tell program to ignore mouse\n");
		printf("/HIDDENCARD  Overrides video detection\n\n");
		printf("Each switch must include a '/' and multiple switches\n");
		printf("must be seperated by at least one space.\n\n");

		exit(0);
	}

	InitGame ();

	CheckMemory ();

	LoadLatchMem ();

	DemoLoop();
	Quit("Demo loop exited???");
}


/* HACKS and STUFF
*/
int _AX=0, _BX=0, _CX=0, _DX=0;
void geninterrupt(int i) {
}

