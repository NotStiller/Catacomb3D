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

// C3_GAME.C

#include "c3_def.h"
#pragma hdrstop

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

#define NUMLUMPS        25

#define CONTROLSLUMP    0
#define ORCLUMP         1
#define TROLLLUMP        2
#define WARPLUMP        3
#define BOLTLUMP        4
#define NUKELUMP        5
#define POTIONLUMP      6
#define RKEYLUMP        7
#define YKEYLUMP        8
#define GKEYLUMP        9
#define BKEYLUMP        10
#define SCROLLLUMP      11
#define CHESTLUMP       12
#define PLAYERLUMP      13
#define WALL1LUMP       14
#define WALL2LUMP       15
#define BDOORLUMP       16
#define DEMONLUMP               17
#define MAGELUMP                18
#define BATLUMP                 19
#define GRELLUMP                20
#define GOALLUMP                21


int     lumpstart[NUMLUMPS] = {
CONTROLS_LUMP_START,
ORC_LUMP_START,
TROLL_LUMP_START,
WARP_LUMP_START,
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
WALL1_LUMP_START,
WALL2_LUMP_START,
BDOOR_LUMP_START,
DEMON_LUMP_START,
MAGE_LUMP_START,
BAT_LUMP_START,
GREL_LUMP_START,
NEMESISPIC
};


int     lumpend[NUMLUMPS] = {
CONTROLS_LUMP_END,
ORC_LUMP_END,
TROLL_LUMP_END,
WARP_LUMP_END,
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
WALL1_LUMP_END,
WALL2_LUMP_END,
BDOOR_LUMP_END,
DEMON_LUMP_END,
MAGE_LUMP_END,
BAT_LUMP_END,
GREL_LUMP_END,
NEMESISPIC
};



/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

memptr  textstarts[27];

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

boolean lumpneeded[NUMLUMPS];


//===========================================================================


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
	uint16_t        x,y,i,j;
	int             tile;
	uint16_t        *start;

	InitObjList();                  // start spawning things with a clean slate

	memset (lumpneeded,0,sizeof(lumpneeded));

	start = mapsegs[2];
	for (y=0;y<mapheight;y++)
	{
		for (x=0;x<mapwidth;x++)
		{
			tile = *start++;
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

			case 20:        // goal
				lumpneeded[GOALLUMP] = true;
				SpawnBonus(x,y,B_GOAL);
				break;

			case 21:        // chest
				lumpneeded[CHESTLUMP] = true;
				SpawnBonus(x,y,B_CHEST);
				break;

			case 24:
				lumpneeded[WARPLUMP] = true;
				SpawnWarp (x,y,0);
				break;
//------
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
				lumpneeded[GRELLUMP] = true;
				SpawnNemesis (x,y);
				break;

			case 29:
				SpawnBounce (x,y,0);
				break;

			case 30:
				SpawnBounce (x,y,1);
				break;

			case 31:
			case 32:
			case 33:
			case 34:
				lumpneeded[WARPLUMP] = true;
				SpawnWarp (x,y,tile-30);
				break;
			}
		}
	}
}

//==========================================================================

/*
==================
=
= ScanText
=
==================
*/

void ScanText (void)
{
	int     i;
	char *text;

	text = (char*)grsegs[LEVEL1TEXT+loadedmap];

	textstarts[0] = 0;

	for (i=1;i<=26;i++)
	{
		while (*text != '\n')
		{
			if (*text == '\r')
				*text = 0;
			text++;
		}
		text++;
		textstarts[i] = text;
	}

}

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
void DrawEnterScreen (void)
{
	int     x,y;

	VW_Bar(0,0,VIEWWIDTH,VIEWHEIGHT,9);     // Medium blue

	x = (VIEWWIDTH - (18 * 8)) / 2 -3;
	y = (VIEWHEIGHT - (5 * 8)) / 2;
	VW_DrawPic(x / 8,y,ENTERPLAQUEPIC);

	WindowX = x;
	WindowW = 18 * 8;
	PrintY = (VIEWHEIGHT/2) + 3;
	US_CPrint (levelnames[gamestate.mapon]);
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
	int     i,j;
	unsigned        source,dest;

	CA_CacheGrChunk(LEVEL1TEXT+loadedmap);
	ScanText ();

//
// cache wall pictures
//
	for (i=1;i<NUMFLOORS;i++)
		if (tileneeded[i])
		{
			SetupScaleWall (walllight1[i]);
			SetupScaleWall (walllight2[i]);
			SetupScaleWall (walldark1[i]);
			SetupScaleWall (walldark2[i]);
		}

//
// cache the actor pictures
//
	for (i=0;i<NUMLUMPS;i++)
		if (lumpneeded[i])
			for (j=lumpstart[i];j<=lumpend[i];j++)
				SetupScalePic(j);

	screenpage = 1;
}

//==========================================================================

/*
==================
=
= SetupGameLevel
=
==================
*/

void SetupGameLevel (void)
{
	int      x,y,i;
	uint16_t *map,tile,spot;

	memset (tileneeded,0,sizeof(tileneeded));
//
// randomize if not a demo
//
	if (DemoMode)
	{
		US_InitRndT(false);
		gamestate.difficulty = gd_Normal;
	}
	else
		US_InitRndT(true);

//
// load the level
//
	CA_CacheMap (gamestate.mapon);

	mapwidth = mapheaderseg[loadedmap]->width;
	mapheight = mapheaderseg[loadedmap]->height;

//
// copy the wall data to a data segment array
//
	memset (tilemap,0,sizeof(tilemap));
	memset (actorat,0,sizeof(actorat));
	map = mapsegs[0];
	for (y=0;y<mapheight;y++) 
	{
		for (x=0;x<mapwidth;x++)
		{
			tile = *map++;
			if (tile<NUMFLOORS)
			{
				printf("%c", 'a'+tile);
				tileneeded[tile] = true;
				tilemap[x][y] = tile;
				if (tile>=EXPWALLSTART && tile<EXPWALLSTART+NUMEXPWALLS)
				{
					tileneeded[WALLEXP] = tileneeded[WALLEXP+1] = tileneeded[WALLEXP+2] = true;
				}

				if (tile>0)
					CASTAT(intptr_t,actorat[x][y]) = tile;
			} else if (tile >= NAMESTART) {
				printf(" ");
			} else {
				printf(" ");
			}
		}
		printf("\n");
	}

//
// decide which graphics are needed and spawn actors
//
	ScanInfoPlane ();

//
// have the caching manager load and purge stuff to make sure all marks
// are in memory
//
	CA_LoadAllSounds ();

}


//==========================================================================

/*
=====================
=
= LatchDrawPic
=
=====================
*/

void LatchDrawPic (unsigned x, unsigned y, unsigned picnum)
{
	VW_DrawPic(x, y, picnum);
/*
	EGAWRITEMODE(1);
	EGAMAPMASK(15);
*/
}


//==========================================================================

/*
=====================
=
= Victory
=
=====================
*/

void Victory (void)
{
	NormalScreen ();
	CA_CacheGrChunk (FINALEPIC);
	VWB_DrawPic (0,0,FINALEPIC);
	VW_UpdateScreen ();
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
	SD_WaitSoundDone ();
	IN_ClearKeysDown ();
	IN_Ack();
}

//==========================================================================

/*
===================
=
= Died
=
===================
*/

void Died (void)
{
	unsigned page1,page2;
//
// fizzle fade screen to grey
//
	SD_PlaySound (GAMEOVERSND);
	LatchDrawPic(0,0,DEADPIC);
	FizzleFade(VIEWWIDTH,VIEWHEIGHT,false);
	IN_ClearKeysDown();
	IN_Ack();
}

//==========================================================================

/*
===================
=
= NormalScreen
=
===================
*/

void NormalScreen (void)
{
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
	int     i,j,p,m;

	screenpage = 0;

	VW_Bar (0,SPLITSCREENOFFSET+0,320,STATUSLINES,7);
	VW_Bar (0,0,320,VIEWHEIGHT,7);

	CA_CacheGrChunk (STATUSPIC);
	CA_CacheGrChunk (SIDEBARSPIC);

	VW_DrawPic (0,SPLITSCREENOFFSET+0,STATUSPIC);

	for (i=0;i<3;i++)
	{
		VW_DrawPic (33,0,SIDEBARSPIC);
	}

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
	int     i;
	for (i=0;i<NUMTILE16;i++)
	{
		CA_CacheGrChunk (STARTTILE16+i);
	}
	for (i=FIRSTLATCHPIC+1;i<FIRSTSCALEPIC;i++)
	{
		CA_CacheGrChunk (i);
	}
}


/*
===================
=
= FizzleOut
=
===================
*/

void FizzleOut (int showlevel)
{
	unsigned page1,page2;
//
// fizzle fade screen to grey
//
	if (showlevel)
		DrawEnterScreen ();
	FizzleFade(VIEWWIDTH,VIEWHEIGHT,false);
}


//==========================================================================

/*
==================
=
= DrawHighScores
=
==================
*/

void    DrawHighScores(void)
{
	char            buffer[16],*str;
	word            i,j,
				w,h,
				x,y;
	HighScore       *s;


	CA_CacheGrChunk (HIGHSCORESPIC);
	VWB_DrawPic (0,0,HIGHSCORESPIC);

	for (i = 0,s = Scores;i < MaxScores;i++,s++)
	{
		PrintY = 68 + (16 * i);

		//
		// name
		//
		PrintX = 60;
		US_Print(s->name);

		//
		// level
		//
		sprintf(buffer, "%i", s->completed);
		for (str = buffer;*str;str++)
			*str = *str + (129 - '0');      // Used fixed-width numbers (129...)
		USL_MeasureString(buffer,&w,&h);
		PrintX = (25 * 8) - 8 - w;
		US_Print(buffer);

		//
		// score
		//
		sprintf(buffer, "%i", s->score);
		for (str = buffer;*str;str++)
			*str = *str + (129 - '0');      // Used fixed-width numbers (129...)
		USL_MeasureString(buffer,&w,&h);
		PrintX = (34 * 8) - 8 - w;
		US_Print(buffer);
	}

	fontcolor = F_BLACK;
}



/*
=======================
=
= CheckHighScore
=
=======================
*/

void    CheckHighScore (long score,word other)
{
	word            i,j;
	int                     n;
	HighScore       myscore;

	strcpy(myscore.name,"");
	myscore.score = score;
	myscore.completed = other;

	for (i = 0,n = -1;i < MaxScores;i++)
	{
		if
		(
			(myscore.score > Scores[i].score)
		||      (
				(myscore.score == Scores[i].score)
			&&      (myscore.completed > Scores[i].completed)
			)
		)
		{
			for (j = MaxScores;--j > i;)
				Scores[j] = Scores[j - 1];
			Scores[i] = myscore;
			n = i;
			HighScoresDirty = true;
			break;
		}
	}

	if (n != -1)
	{
	//
	// got a high score
	//
		DrawHighScores ();
		PrintY = 68 + (16 * n);
		PrintX = 60;
		US_LineInput(PrintX,PrintY,Scores[n].name,nil,true,MaxHighName,100);
	}
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
	int i,xl,yl,xh,yh;
	char num[20];

	DrawPlayScreen ();

restart:
	if (!loadedgame)
	{
		gamestate.difficulty = restartgame;
		restartgame = gd_Continue;
		DrawEnterScreen ();
	}

	do
	{
		playstate = gd_Continue;
		if (!loadedgame)
			SetupGameLevel ();
		else
			loadedgame = false;

		CacheScaleds ();

		SP_GameEnter();
		PlayLoop ();
		SP_GameLeave();

		switch (playstate)
		{
		case ex_died:
			Died ();
			NormalScreen ();
			CheckHighScore (gamestate.score,gamestate.mapon+1);
			return;
		case ex_warped:
			FizzleOut (true);
			if (gamestate.mapon >= NUMLEVELS)
			{
				Victory ();
				CheckHighScore(gamestate.score,gamestate.mapon+1);
				return;
			}
			break;
		case ex_abort:
			return;
		case ex_resetgame:
		case ex_loadedgame:
			goto restart;
		case ex_victorious:
			Victory ();
			CheckHighScore(gamestate.score,gamestate.mapon+1);
			return;
		}

	} while (1);

}
