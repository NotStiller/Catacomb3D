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

// C3_GAME.C

#include <stdlib.h>

#include "c4_def.h"
//#include "gelib.h"


/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

#define NUMLUMPS        36

#define ORCLUMP         		0
#define TROLLLUMP       		1
#define BOLTLUMP        		2
#define NUKELUMP        		3
#define POTIONLUMP      		4
#define RKEYLUMP        5
#define YKEYLUMP        6
#define GKEYLUMP        7
#define BKEYLUMP        8
#define SCROLLLUMP      9
#define CHESTLUMP       10
#define PLAYERLUMP      11
#define WALL1LUMP       12
#define WALL2LUMP       13
#define BDOORLUMP       14
#define DEMONLUMP               15
#define MAGELUMP                16
#define BATLUMP                 17
#define GRELLUMP                18
#define TOMBSTONESLUMP			  19
#define ZOMBIELUMP				  20
#define SPOOKLUMP					  21
#define SKELETONLUMP				  22
#define RGEMLUMP					  23
#define GGEMLUMP					  24
#define BGEMLUMP					  25
#define YGEMLUMP					  26
#define PGEMLUMP					  27
#define RKEY2LUMP					  28
#define WETMANLUMP				  29
#define OBJ_WARPLUMP				  30
#define EYELUMP					  31
#define REDDEMONLUMP            32
#define PITLUMP					  33
#define FTIMELUMP					  34
#define WATERCHESTLUMP			  35



int     lumpstart[NUMLUMPS] = {
ORC_LUMP_START,
TROLL_LUMP_START,
BOLT_LUMP_START,
NUKE_LUMP_START,
POTION_LUMP_START,
RKEY_LUMP_START,
YKEY_LUMP_START,
GKEY_LUMP_START,
BKEY_LUMP_START,
SCROLL_LUMP_START,
CHEST_LUMP_START,
PLAYER_LUMP_START,
//WALL1_LUMP_START,
//WALL2_LUMP_START,
//BDOOR_LUMP_START,
0,0,0,
DEMON_LUMP_START,
MAGE_LUMP_START,
BAT_LUMP_START,
GREL_LUMP_START,
TOMBSTONES_LUMP_START,
ZOMBIE_LUMP_START,
SPOOK_LUMP_START,
SKELDUDE_LUMP_START,
RGEM_LUMP_START,
GGEM_LUMP_START,
BGEM_LUMP_START,
YGEM_LUMP_START,
PGEM_LUMP_START,
RKEY2_LUMP_START,
WETMAN_LUMP_START,
OBJ_WARP_LUMP_START,
EYE_LUMP_START,
REDDEMON_LUMP_START,
PIT_LUMP_START,
TIME_LUMP_START,
O_WATER_CHEST_LUMP_START,
};


int     lumpend[NUMLUMPS] = {
ORC_LUMP_END,
TROLL_LUMP_END,
BOLT_LUMP_END,
NUKE_LUMP_END,
POTION_LUMP_END,
RKEY_LUMP_END,
YKEY_LUMP_END,
GKEY_LUMP_END,
BKEY_LUMP_END,
SCROLL_LUMP_END,
CHEST_LUMP_END,
PLAYER_LUMP_END,
//WALL1_LUMP_END,
//WALL2_LUMP_END,
//BDOOR_LUMP_END,
0,0,0,
DEMON_LUMP_END,
MAGE_LUMP_END,
BAT_LUMP_END,
GREL_LUMP_END,
TOMBSTONES_LUMP_END,
ZOMBIE_LUMP_END,
SPOOK_LUMP_END,
SKELDUDE_LUMP_END,
RGEM_LUMP_END,
GGEM_LUMP_END,
BGEM_LUMP_END,
YGEM_LUMP_END,
PGEM_LUMP_END,
RKEY2_LUMP_END,
WETMAN_LUMP_END,
OBJ_WARP_LUMP_END,
EYE_LUMP_END,
REDDEMON_LUMP_END,
PIT_LUMP_END,
TIME_LUMP_END,
O_WATER_CHEST_LUMP_END,
};




/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

unsigned        latchpics[NUMLATCHPICS];
unsigned        tileoffsets[NUMTILE16];
unsigned        textstarts[27];

boolean splitscreen=false;

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

boolean lumpneeded[NUMLUMPS];


//===========================================================================

//==========================================================================
//
//
//							LOCAL PROTOTYPES
//
//
//==========================================================================

void CashPoints(void);



/*
==========================
=
= ScanInfoPlane
=
= Spawn all actors and mark down special places
=
==========================
*/

void ScanInfoPlane (void)
{
	extern unsigned gnd_colors[];

	char hibyte;
	unsigned        x,y,i,j;
	int                     tile;

	InitObjList();                  // start spawning things with a clean slate

	memset (lumpneeded,0,sizeof(lumpneeded));

	for (y=0;y<curmap->height;y++)
		for (x=0;x<curmap->width;x++)
		{
			tile = GetMapSegs(2,x,y);
			hibyte = tile >> 8;
			tile &= 0xff;
			if (!tile)
				continue;

			switch (tile)
			{
			case 1:
			case 2:
			case 3:
			case 4:
				lumpneeded[PLAYERLUMP] = true;
				SpawnPlayer(x,y,NORTH+tile-1);
				break;

			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
				lumpneeded[tile-5+BOLTLUMP] = true;
				SpawnBonus(x,y,tile-5);
				break;

			case 29:
				lumpneeded[RKEY2LUMP] = true;
				SpawnBonus(x,y,B_RKEY2);
			break;

			case 58:
			case 59:
			case 60:
			case 61:
			case 62:
				lumpneeded[tile-58+RGEMLUMP] = true;
				SpawnBonus(x,y,tile-58+B_RGEM);
			break;

			case 12:
			case 13:
			case 14:
			case 15:
			case 16:
			case 17:
			case 18:
			case 19:
				lumpneeded[SCROLLLUMP] = true;
				SpawnBonus(x,y,B_SCROLL1+tile-12);
				break;

#if 0
			case 20:        // goal
				lumpneeded[GOALLUMP] = true;
				SpawnBonus(x,y,B_GOAL);
				break;
#endif

			case 21:        // chest
				if (gnd_colors[gamestate.mapon] == 0x0101)
					lumpneeded[WATERCHESTLUMP] = true;
				else
					lumpneeded[CHESTLUMP] = true;
				SpawnBonus(x,y,B_CHEST);
			break;

			case 31:
			case 32:
			case 33:
			case 34:
			case 35:
				lumpneeded[OBJ_WARPLUMP] = true;
				SpawnWarp (x,y,tile-30);
				break;

			case 41:
				if (gamestate.difficulty <gd_Hard)
					break;
			case 36:
				if (gamestate.difficulty <gd_Normal)
					break;
			case 22:
				lumpneeded[TROLLLUMP] = true;
				SpawnTroll (x,y);
				break;

			case 42:
				if (gamestate.difficulty <gd_Hard)
					break;
			case 37:
				if (gamestate.difficulty <gd_Normal)
					break;
			case 23:
				lumpneeded[ORCLUMP] = true;
				SpawnOrc (x,y);
				break;

			case 43:
				if (gamestate.difficulty <gd_Hard)
					break;
			case 38:
				if (gamestate.difficulty <gd_Normal)
					break;
			case 25:
				lumpneeded[BATLUMP] = true;
				SpawnBat (x,y);
				break;

			case 44:
				if (gamestate.difficulty <gd_Hard)
					break;
			case 39:
				if (gamestate.difficulty <gd_Normal)
					break;
			case 26:
				lumpneeded[DEMONLUMP] = true;
				SpawnDemon (x,y);
				break;

			case 45:
				if (gamestate.difficulty <gd_Hard)
					break;
			case 40:
				if (gamestate.difficulty <gd_Normal)
					break;
			case 27:
				lumpneeded[MAGELUMP] = true;
				SpawnMage (x,y);
				break;

			case 28:
				lumpneeded[RKEYLUMP] = lumpneeded[GRELLUMP] = true;
				SpawnGrelminar (x,y);
				break;

#if 0
			case 29:
				SpawnBounce (x,y,0);
				break;

			case 30:
				SpawnBounce (x,y,1);
				break;
#endif

			case 46:
			case 47:
			case 48:
				lumpneeded[TOMBSTONESLUMP] = true;
				SpawnTombstone(x,y,tile-46);
			break;

			case 54:
				lumpneeded[PITLUMP]	= true;
				SpawnWarp(x,y,0);
				break;

			case 53:
				if (gamestate.difficulty <gd_Normal)
					break;
			case 52:
				lumpneeded[ZOMBIELUMP] = true;
				SpawnZombie(x,y);
			break;

			case 51:
				if (gamestate.difficulty <gd_Hard)
					break;
			case 50:
				if (gamestate.difficulty <gd_Normal)
					break;
			case 49:
				lumpneeded[SPOOKLUMP] = true;
				SpawnSpook(x,y);
				break;

			case 57:
				lumpneeded[FTIMELUMP] = true;
				SpawnFTime(x,y);
				break;

			case 56:
				if (gamestate.difficulty <gd_Normal)
					break;
			case 55:
				lumpneeded[SKELETONLUMP] = true;
				SpawnSkeleton(x,y);
				break;

			case 65:
				if (gamestate.difficulty <gd_Hard)
					break;
			case 64:
				if (gamestate.difficulty <gd_Normal)
					break;
			case 63:
				lumpneeded[WETMANLUMP] = true;
				SpawnWetMan(x,y);
				break;

			case 68:
				if (gamestate.difficulty <gd_Hard)
					break;
			case 67:
				if (gamestate.difficulty <gd_Normal)
					break;
			case 66:
				lumpneeded[EYELUMP] = true;
				SpawnEye(x,y);
				break;

			case 71:
				if (gamestate.difficulty <gd_Hard)
					break;
			case 70:
				if (gamestate.difficulty <gd_Normal)
					break;
			case 69:
				lumpneeded[SKELETONLUMP] = true;
				SpawnWallSkeleton(x,y);
				break;

			case 20:
			case 24:
			case 30:
				lumpneeded[REDDEMONLUMP] = true;
				SpawnRedDemon (x,y);
				break;
			}
		}

}

//==========================================================================


//==========================================================================

/*
==================
=
= DrawEnterScreen
=
==================
*/

static  char    *levelnames[] =
				{
					"The Towne Cemetery",
					"The Garden of Tears",
					"The Den of Zombies",
					"The Mausoleum Grounds",
					"The Main Floor of the Mausoleum",
					"Mike's Blastable Passage",
					"The Crypt of Nemesis the Undead",
					"The Subterranean Vault",
					"The Ancient Aqueduct",
					"The Orc Mines",
					"The Lair of the Troll",
					"The Demon's Inferno",
					"The Battleground of the Titans",
					"The Coven of Mages",
					"The Inner Sanctum",
					"The Haunt of Nemesis",
					"The Passage to the Surface",
					"Big Jim's Domain",
					"Nolan's Nasty",
				};

void DrawEnterScreen (void)
{
	int width;

	SPG_Bar(&renderBufferText, 0,0,renderBuffer.Width,renderBuffer.Height, 0);
//	VW_Bar(0,0,VIEWWIDTH,VIEWHEIGHT,0);

	width = strlen(levelnames[gamestate.mapon]);
	if (width < 20)
		width = 20;
	Win_Create(&renderBufferText, width,5);
	Win_CPrint("\nYou have arrived at\n");
	Win_CPrint(levelnames[gamestate.mapon]);
}
//==========================================================================

boolean tileneeded[NUMFLOORS];


/*
==================
=
= CacheScaleds
=
==================
*/

void CacheScaleds (void)
{
}

//==========================================================================

/*
==================
=
= SetupGameLevel
=
==================
*/

void SetupGameLevel ()
{
	int     x,y,i,j,loop;
	uint16_t        *map,tile,*spotptr,spot;

	memset (tileneeded,0,sizeof(tileneeded));
	US_InitRndT(true);

//
// load the level
//
	SPD_LoadMap(LEVEL1TEXT+gamestate.mapon,gamestate.mapon);

	int plane;
	for (plane = 0; plane<MAPPLANES; plane++)
	{
		int size=curmap->rawplaneslength[plane];
		printf("INIT MAP SEGS %i of size %i\n",plane, size);
		if (!size)
			continue;
		MM_GetPtr (&gamestate.mapsegs[plane], size);
		memcpy(gamestate.mapsegs[plane], curmap->rawplanes[plane], size);
	}

//
// copy the wall data to a data segment array
//
	ClearTileMap();
	ClearActorAt();
	for (y=0;y<curmap->height;y++)
		for (x=0;x<curmap->width;x++)
		{
			int spot = GetMapSegs(2,x,y)>>8;
			int tile = GetMapSegs(0,x,y);
			if (spot == EXP_WALL_CODE)
			{
				extern unsigned gnd_colors[];

				if (gnd_colors[gamestate.mapon] == 0x0101)
					tileneeded[WATEREXP] = tileneeded[WATEREXP+1] = tileneeded[WATEREXP+2] = true;
				else
					tileneeded[WALLEXP] = tileneeded[WALLEXP+1] = tileneeded[WALLEXP+2] = true;
			}

			if (tile<NUMFLOORS)
			{
				if (tile == WALL_SKELETON_CODE)
					tileneeded[tile+1] = tileneeded[tile+2] = true;

				tileneeded[tile] = true;
				SetTileMap(x,y,tile);
				if (tile>0)
					SetActorAtInt(x,y,tile);
			}
		}
//
// decide which graphics are needed and spawn actors
//
	zombie_base_delay = 0;	// (1*60) + RANDOM(1*60);
	ScanInfoPlane ();


//
// cache wall pictures
//
	for (i=1;i<NUMFLOORS;i++)
		if (tileneeded[i])
		{
			SPD_SetupScaleWall (walllight1[i], walllight1[i]-STARTPICS, walllight1[i]-FIRSTWALLPIC);
			SPD_SetupScaleWall (walllight2[i], walllight2[i]-STARTPICS, walllight2[i]-FIRSTWALLPIC);
			SPD_SetupScaleWall (walldark1[i], walldark1[i]-STARTPICS, walldark1[i]-FIRSTWALLPIC);
			SPD_SetupScaleWall (walldark2[i], walldark2[i]-STARTPICS, walldark2[i]-FIRSTWALLPIC);
		}

//
// cache the actor pictures
//
	for (i=0;i<NUMLUMPS;i++)
		if (lumpneeded[i])
			for (j=lumpstart[i];j<=lumpend[i];j++)
				SPD_SetupScalePic(j, j-STARTPICS, j-FIRSTSCALEPIC);

}

/*
=====================
=
= Victory
=
=====================
*/

void Victory (boolean playsounds)
{
	if (playsounds)
	{
		SD_PlaySound (GETBOLTSND);
		SD_WaitSoundDone ();
		SD_PlaySound (GETNUKESND);
		SD_WaitSoundDone ();
		SD_PlaySound (GETPOTIONSND);
		SD_WaitSoundDone ();
		SD_PlaySound (GETKEYSND);
		SD_WaitSoundDone ();
		SD_PlaySound (GETSCROLLSND);
		SD_WaitSoundDone ();
		SD_PlaySound (GETPOINTSSND);
	}


	if (!screenfaded)
		VW_FadeOut();

	VW_Bar (0,0,320,120,0);

	SPD_LoadGrChunk(FINALEPIC);
	SPG_DrawPic(&renderBufferText, grsegs[FINALEPIC], 0, 0);

	VW_FadeIn();
}

/*
===================
=
= NormalScreen
=
===================
*/

void NormalScreen (void)
{
//	 VW_SetSplitScreen (200);
//	SPG_ClearScreen(0);
	VW_Bar(0,0,320,200,0);
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
//	SPG_ClearScreen(0);
//	VW_Bar (0,0,320,VIEWHEIGHT,0);
	SPD_LoadGrChunk (STATUSPIC);
	SPG_DrawPic(&bottomHUDBuffer, grsegs[STATUSPIC], 0,0);
	RedrawStatusWindow ();
}


//==========================================================================

/*
===================
=
= LoadLatchMem
=
===================
*/

void LoadLatchMem (void)
{
	int i;

	for (i=0;i<NUMTILE16;i++)
	{
		SPD_LoadGrChunk (STARTTILE16+i);
	}

	for (i=FIRSTLATCHPIC+1;i<FIRSTSCALEPIC;i++)
	{
		SPD_LoadGrChunk (i);
	}
}



//==========================================================================


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
	if (!loadedgame)
	{
		memset (&gamestate,0,sizeof(gamestate));
		gamestate.mapon = 0;
		gamestate.body = MAXBODY;
	}

	BGFLAGS = 0;
	Flags &= FL_CLEAR;

	boltsleft = bolttimer = 0;
	FreezeTime = 0;
}


//==========================================================================

/*
===================
=
= GameLoop
=
===================
*/

void GameLoop (void)
{
	boolean wait = false;
	int i,xl,yl,xh,yh;
	char num[20];

	SP_GameEnter();
	NewGame();
	DrawPlayScreen ();
	IN_ClearKeysDown();

restart:
	if (!loadedgame)
	{
		gamestate.difficulty = restartgame;
		restartgame = gd_Continue;
		DrawEnterScreen ();
		if (gamestate.mapon != 8)
			fizzlein = true;
		wait = true;
	}

	do
	{
		playstate = gd_Continue;
		if (!loadedgame)
			SetupGameLevel ();
		else
			loadedgame = false;

		if (EASYMODEON)
			DisplaySMsg("*** NOVICE ***", NULL);
		else
			DisplaySMsg("*** WARRIOR ***", NULL);

		status_delay = 250;

		RedrawStatusWindow();
		PlayLoop ();
		switch (playstate)
		{
		case ex_abort:
			SP_GameLeave();
			return;
		case ex_resetgame:
			NewGame();
		case ex_loadedgame:
		case ex_warped:
			goto restart;
		break;
		}

	} while (1);

}


