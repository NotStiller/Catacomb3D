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

						 LOCAL VARIABLES

=============================================================================
*/

static gametype *game = NULL;
boolean tileneeded[NUMFLOORS];
boolean lumpneeded[NUMLUMPS];


//===========================================================================


extern statetype
	s_boltbonus, s_boltbonus2, 
	s_nukebonus, s_nukebonus2,
	s_potionbonus,
	s_rkeybonus, s_ykeybonus, s_gkeybonus, s_bkeybonus,
	s_scrollbonus, s_chestbonus, s_goalbonus,
	s_walldie1, s_walldie2, s_walldie3, s_walldie4, s_walldie5, s_walldie6,
	s_gate1, s_gate2, s_gate3, s_gate4,
	s_fgate1, s_fgate2, s_fgate3, s_fgate4,
	s_trollpause, s_troll1, s_troll2, s_troll3, s_troll4,
	s_trollattack1, s_trollattack2, s_trollattack3,
	s_trollouch, s_trolldie1, s_trolldie2, s_trolldie3,
	s_orcpause, s_orc1, s_orc2, s_orc3, s_orc4,
	s_orcattack1, s_orcattack2, s_orcattack3,
	s_orcouch, s_orcdie1, s_orcdie2, s_orcdie3,
	s_demonpause, s_demon1, s_demon2, s_demon3, s_demon4,
	s_demonattack1, s_demonattack2, s_demonattack3,
	s_demonouch, s_demondie1, s_demondie2, s_demondie3,
	s_mshot1, s_mshot2, s_magepause, s_mage1, s_mage2,
	s_mageattack1, s_mageattack2, s_mageattack3,
	s_mageouch, s_magedie1, s_magedie2,
	s_grelpause, s_grel1, s_grel2,
	s_grelattack1, s_grelattack2, s_grelattack3, s_grelouch,
	s_greldie1, s_greldie2, s_greldie3, s_greldie4, s_greldie5, s_greldie6,
	s_bat1, s_bat2, s_bat3, s_bat4,
	s_batpast, s_batdie1, s_batdie2,
	s_bounce1, s_bounce2,
	s_pshot1, s_pshot2,
	s_shotexplode,
	s_bigpshot1, s_bigpshot2,
	s_player;

statetype *statetypes[] = { NULL,
&s_boltbonus, &s_boltbonus2, &s_nukebonus, &s_nukebonus2,
&s_potionbonus, &s_rkeybonus, &s_ykeybonus, &s_gkeybonus, &s_bkeybonus,
&s_scrollbonus, &s_chestbonus, &s_goalbonus,
&s_walldie1, &s_walldie2, &s_walldie3, &s_walldie4, &s_walldie5, &s_walldie6,
&s_gate1, &s_gate2, &s_gate3, &s_gate4,
&s_fgate1, &s_fgate2, &s_fgate3, &s_fgate4,
&s_trollpause, &s_troll1, &s_troll2, &s_troll3, &s_troll4,
&s_trollattack1, &s_trollattack2, &s_trollattack3,
&s_trollouch, &s_trolldie1, &s_trolldie2, &s_trolldie3,
&s_orcpause, &s_orc1, &s_orc2, &s_orc3, &s_orc4,
&s_orcattack1, &s_orcattack2, &s_orcattack3, &s_orcouch,
&s_orcdie1, &s_orcdie2, &s_orcdie3,
&s_demonpause, &s_demon1, &s_demon2, &s_demon3, &s_demon4,
&s_demonattack1, &s_demonattack2, &s_demonattack3, &s_demonouch,
&s_demondie1, &s_demondie2, &s_demondie3,
&s_mshot1, &s_mshot2, &s_magepause, &s_mage1, &s_mage2,
&s_mageattack1, &s_mageattack2, &s_mageattack3,
&s_mageouch, &s_magedie1, &s_magedie2, &s_grelpause,
&s_grel1, &s_grel2, &s_grelattack1, &s_grelattack2, &s_grelattack3, &s_grelouch,
&s_greldie1, &s_greldie2, &s_greldie3, &s_greldie4, &s_greldie5, &s_greldie6,
&s_bat1, &s_bat2, &s_bat3, &s_bat4, &s_batpast, &s_batdie1, &s_batdie2,
&s_bounce1, &s_bounce2,
&s_pshot1, &s_pshot2, &s_shotexplode, &s_bigpshot1, &s_bigpshot2, &s_player,
NULL,
	};


static uint32_t StateToInt(statetype *StateType) {
	printf("StateToInt(%p)\n", StateType);
	if (StateType == NULL) {
		return 0;
	} else {
		uint32_t i;
		for (i = 1; statetypes[i] != NULL; i++) {
			if (statetypes[i] == StateType) {
				printf("Resolves to %i.\n", i);
				return i;
			}
		}
	}
	printf("ERROR: the state %p did not match anything !\n", StateType);
	assert(NULL);
}

static statetype *IntToState(uint32_t StateNum) {
	printf("IntToState(%i)\n", StateNum);
	if (StateNum == 0) {
		return NULL;
	}
// I can't remember myself, why not just statetypes[StateNum] ?
// To prevent memory violation I guess.
	uint32_t i;
	for (i = 1; statetypes[i] != NULL; i++) {
		if (i == StateNum) { 
			printf("resolves to %p.\n", statetypes[i]);
			return statetypes[i];
		}
	}
	printf("ERROR: invalid enumerated state type %i\n", StateNum);
	assert(NULL);
}


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

void ScanInfoPlane (gametype *Gamestate)
{
	uint16_t        x,y,i,j;
	int             tile;
	uint16_t        *start;


	memset (lumpneeded,0,sizeof(lumpneeded));

	start = Gamestate->mapsegs[2];
	for (y=0;y<curmap->height;y++)
	{
		for (x=0;x<curmap->width;x++)
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
				if (Gamestate->difficulty <gd_Hard)
					break;
			case 36:
				if (Gamestate->difficulty <gd_Normal)
					break;
			case 22:
				lumpneeded[TROLLLUMP] = true;
				SpawnTroll (x,y);
				break;

			case 42:
				if (Gamestate->difficulty <gd_Hard)
					break;
			case 37:
				if (Gamestate->difficulty <gd_Normal)
					break;
			case 23:
				lumpneeded[ORCLUMP] = true;
				SpawnOrc (x,y);
				break;

			case 43:
				if (Gamestate->difficulty <gd_Hard)
					break;
			case 38:
				if (Gamestate->difficulty <gd_Normal)
					break;
			case 25:
				lumpneeded[BATLUMP] = true;
				SpawnBat (x,y);
				break;

			case 44:
				if (Gamestate->difficulty <gd_Hard)
					break;
			case 39:
				if (Gamestate->difficulty <gd_Normal)
					break;
			case 26:
				lumpneeded[DEMONLUMP] = true;
				SpawnDemon (x,y);
				break;

			case 45:
				if (Gamestate->difficulty <gd_Hard)
					break;
			case 40:
				if (Gamestate->difficulty <gd_Normal)
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




//==========================================================================

/*
==================
=
= SetupGameLevel
=
==================
*/

void SetupGameLevel (gametype *Gamestate)
{
	int      x,y,i,j;
	uint16_t *map,tile,spot;

//
// randomize if not a demo
//
	US_InitRndT(true);

//
// load the level
//
	SPD_LoadMap(LEVEL1TEXT+Gamestate->mapon,Gamestate->mapon);
	int plane;
	for (plane = 0; plane<MAPPLANES; plane++)
	{
		int size=curmap->rawplaneslength[plane];
		if (!size)
			continue;
		MM_GetPtr (&Gamestate->mapsegs[plane], size);
		memcpy(Gamestate->mapsegs[plane], curmap->rawplanes[plane], size);
	}

//
// copy the wall data to a data segment array
//
	memset (tileneeded,0,sizeof(tileneeded));
	ClearTileMap();
	ClearActorAt();
	map = Gamestate->mapsegs[0];
	for (y=0;y<curmap->height;y++) 
	{
		for (x=0;x<curmap->width;x++)
		{
			tile = *map++;
			if (tile<NUMFLOORS)
			{
				printf("%c", 'a'+tile);
				tileneeded[tile] = true;
				SetTileMap(x,y,tile);
				if (tile>=EXPWALLSTART && tile<EXPWALLSTART+NUMEXPWALLS)
				{
					tileneeded[WALLEXP] = tileneeded[WALLEXP+1] = tileneeded[WALLEXP+2] = true;
				}

				if (tile>0)
					SetActorAtInt(x,y,tile);
			} else if (tile >= NAMESTART) {
				printf(" ");
			} else {
				printf(" ");
			}
		}
		printf("\n");
	}

	InitObjList();                  // start spawning things with a clean slate

//
// decide which graphics are needed and spawn actors
//
	ScanInfoPlane (Gamestate);


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


//==========================================================================


/*
==================
=
= SaveTheGame
=
==================
*/


boolean SaveTheGame(FILE *file)
{
	uint8_t *buffer, *p;
	int i, j;
	objtype *o;

	assert(game != NULL);
	gametype *Gamestate = game;

	int maxsize = 20*4+2*3*64*64+MAXACTORS*27;
	p = buffer = malloc(maxsize);
	SPD_WriteU32(&p, Gamestate->difficulty);
	SPD_WriteU32(&p, Gamestate->mapon);
	SPD_WriteU32(&p, Gamestate->bolts);
	SPD_WriteU32(&p, Gamestate->nukes);
	SPD_WriteU32(&p, Gamestate->potions);
	for (i = 0; i < 4; i++) {
		SPD_WriteU32(&p, Gamestate->keys[i]);
	}
	for (i = 0; i < 8; i++) {
		SPD_WriteU32(&p, Gamestate->scrolls[i]);
	}
	SPD_WriteU32(&p, Gamestate->score);
	SPD_WriteU32(&p, Gamestate->body);
	SPD_WriteU32(&p, Gamestate->shotpower);

	for (i = 0;i < 2;i++)	// Write planes 0 and 2
	{
		int num=curmap->width*curmap->height;
		printf("Plane %i size %i\n", 2*i, num);
		SPD_WriteU32(&p, num);
		for (j = 0; j < num; j++) {
			SPD_WriteU16(&p, Gamestate->mapsegs[2*i][j]);
		}
	}

	for (o = player; o; o = o->next) {
		SPD_WriteU32(&p, o->active);
		SPD_WriteS32(&p, o->ticcount);
		SPD_WriteS32(&p, o->obclass);
		printf("obj %p state %p -> num %i\n", o, o->state, StateToInt(o->state));
		SPD_WriteU32(&p, StateToInt(o->state));
		SPD_WriteU32(&p, o->flags);
		SPD_WriteU32(&p, o->shootable);
		SPD_WriteU32(&p, o->tileobject);
		SPD_WriteS32(&p, o->distance);
		SPD_WriteU32(&p, o->dir);
		SPD_WriteS32(&p, o->x);
		SPD_WriteS32(&p, o->y);
		SPD_WriteU32(&p, o->tilex);
		SPD_WriteU32(&p, o->tiley);
		SPD_WriteS32(&p, o->viewx);
		SPD_WriteU32(&p, o->viewheight);
		SPD_WriteS32(&p, o->angle);
		SPD_WriteS32(&p, o->hitpoints);
		SPD_WriteS32(&p, o->speed);
		SPD_WriteS32(&p, o->size);
		SPD_WriteS32(&p, o->xl);
		SPD_WriteS32(&p, o->xh);
		SPD_WriteS32(&p, o->yl);
		SPD_WriteS32(&p, o->yh);
		SPD_WriteS32(&p, o->temp1);
		SPD_WriteS32(&p, o->temp2);
		SPD_WriteU32(&p, (o->next!=NULL)?1:0);
		printf("%i %i %i %i\n", o->x, o->y, o->tilex, o->tiley);
	}


	printf("saving length %i\n", (int)(p-buffer));
	assert((int)(p-buffer) <= maxsize);
	
	SPD_WriteToFile(file, buffer, p-buffer);
	free(buffer);

	return true;
}

//===========================================================================

/*

==================
=

= LoadTheGame
=
==================
*/

gametype *LoadTheGame(char *Filename, int Skip)
{
	uint8_t *buffer, *p;
	int i, j;
	long size;
	
	FILE *file = fopen(Filename, "rb");
	assert(file != NULL);
	fseek(file, Skip, SEEK_SET);

	p = buffer = SPD_ReadFile2(file, &size);
	printf("loaded length %i\n", size);

	gametype *Gamestate = malloc(sizeof(gametype));
	assert(Gamestate != NULL);
	Gamestate->difficulty = SPD_ReadU32(&p);
	Gamestate->mapon = SPD_ReadU32(&p);
	Gamestate->bolts = SPD_ReadU32(&p);
	Gamestate->nukes = SPD_ReadU32(&p);
	Gamestate->potions = SPD_ReadU32(&p);
	for (i = 0; i < 4; i++) {
		Gamestate->keys[i] = SPD_ReadU32(&p);
	}
	for (i = 0; i < 8; i++) {
		Gamestate->scrolls[i] = SPD_ReadU32(&p);
	}
	Gamestate->score = SPD_ReadU32(&p);
	Gamestate->body = SPD_ReadU32(&p);
	Gamestate->shotpower = SPD_ReadU32(&p);



	Gamestate->mapsegs[1] = NULL;
	for (i = 0;i < 2;i++)	// Write planes 0 and 2
	{
		int num = SPD_ReadU32(&p);
		printf("plane %i num %i\n", 2*i, num);
		Gamestate->mapsegs[2*i] = (uint16_t*)malloc(2*num);
		for (j = 0; j < num; j++) {
			Gamestate->mapsegs[2*i][j] = SPD_ReadU16(&p);
		}
	}
	assert((p-buffer) <= size);

	SetupGameLevel (Gamestate);		// load in and cache the base old level
	assert((p-buffer) <= size);


//
// copy the wall data to a data segment array again, to handle doors and
// bomb walls that are allready opened
//
	ClearTileMap();
	ClearActorAt();
	uint16_t *map;
	int x,y;
	map = Gamestate->mapsegs[0];
	for (y=0;y<curmap->height;y++) {
		for (x=0;x<curmap->width;x++) {
			uint16_t tile;
			tile = *map++;
			if (tile<NUMFLOORS)
			{
				SetTileMap(x,y,tile);
				if (tile>0) {
					SetActorAtInt(x,y,tile);
				}
			}
		}
	}
	assert((p-buffer) <= size);


	// Read the object list back in - assumes at least one object in list

	statetype	*state;
	struct	objstruct	*next,*prev, *o;
	InitObjList ();
	objtype *new;
	new = player;
	while (true)
	{
		prev = new->prev;
		next = new->next;

		o = new;
		o->active = SPD_ReadU32(&p);
		o->ticcount = SPD_ReadS32(&p);
		o->obclass = SPD_ReadS32(&p);
		o->state = IntToState(SPD_ReadU32(&p));
		o->flags = SPD_ReadU32(&p);
		o->shootable = SPD_ReadU32(&p);
		o->tileobject = SPD_ReadU32(&p);
		o->distance = SPD_ReadS32(&p);
		o->dir = SPD_ReadU32(&p);
		o->x = SPD_ReadS32(&p);
		o->y = SPD_ReadS32(&p);
		o->tilex = SPD_ReadU32(&p);
		o->tiley = SPD_ReadU32(&p);
		o->viewx = SPD_ReadS32(&p);
		o->viewheight = SPD_ReadU32(&p);
		o->angle = SPD_ReadS32(&p);
		o->hitpoints = SPD_ReadS32(&p);
		o->speed = SPD_ReadS32(&p);
		o->size = SPD_ReadS32(&p);
		o->xl = SPD_ReadS32(&p);
		o->xh = SPD_ReadS32(&p);
		o->yl = SPD_ReadS32(&p);
		o->yh = SPD_ReadS32(&p);
		o->temp1 = SPD_ReadS32(&p);
		o->temp2 = SPD_ReadS32(&p);
		int followed = SPD_ReadU32(&p);
		printf("%i %i %i %i\n", o->x, o->y, o->tilex, o->tiley);
		
		new->prev = prev;
		new->next = next;
		SetActorAt(new->tilex,new->tiley,new); // drop a new marker

		if (followed)
			new = GetNewObj (false);
		else
			break;
	}
	assert((p-buffer) <= size);
	free(buffer);

	return Gamestate;
}

//===========================================================================



/*
=====================
=
= NewGame
=
= Set up new game to start from the beginning
=
=====================
*/

gametype *NewGame (GameDiff Difficulty)
{
	gametype *gs;
	gs = malloc(sizeof(gametype));
	memset (gs,0,sizeof(gametype));
	gs->mapon = 0;
	gs->body = MAXBODY;
	gs->difficulty = Difficulty;
	gs->shotpower = handheight = 0;
	pointcount = pointsleft = 0;
	lastnuke = 0;
	SetupGameLevel(gs);

	return gs;
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
	SPD_LoadGrChunk(FINALEPIC);
	SPG_DrawPic(&guiBuffer, grsegs[FINALEPIC], 0, 0);
	SPG_FlipBuffer();
	SPA_PlaySound (GETBOLTSND);
	SPA_WaitUntilSoundIsDone ();
	SPA_PlaySound (GETNUKESND);
	SPA_WaitUntilSoundIsDone ();
	SPA_PlaySound (GETPOTIONSND);
	SPA_WaitUntilSoundIsDone ();
	SPA_PlaySound (GETKEYSND);
	SPA_WaitUntilSoundIsDone ();
	SPA_PlaySound (GETSCROLLSND);
	SPA_WaitUntilSoundIsDone ();
	SPA_PlaySound (GETPOINTSSND);
	SPA_WaitUntilSoundIsDone ();
	SPI_ClearKeysDown ();
	SPI_WaitForever();
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
	SPA_PlaySound (GAMEOVERSND);
	SPG_ClearBuffer(0);
	SPG_DrawPic(&guiBuffer, grsegs[DEADPIC],0,0);
	SPI_ClearKeysDown();
	SPG_FizzleFadeBuffer();
	SPI_ClearKeysDown();
	SPI_WaitForever();
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


	SPD_LoadGrChunk (HIGHSCORESPIC);

	SPG_DrawPic(&guiBuffer, grsegs[HIGHSCORESPIC], 0, 0);

	fontcolor = WHITE^14; // white xor'd on yellow is blue ! 
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
		fontcolor = 1;
		US_LineInput(PrintX,PrintY,Scores[n].name,NULL,true,MaxHighName,100,14);
		fontcolor = WHITE;
	}
}


/*
===================
=
= FinishGame
=
===================
*/

void DestroyGame(gametype *Gamestate) {
	assert(Gamestate != NULL);
#warning also release objlist, mapsegs, what else ?
	free(Gamestate);
}


//==========================================================================

/*
===================
=
= GameLoop
=
===================
*/

boolean GameLoop (void) {
	US_SetSaveHook(SaveTheGame);
	ControlPanelExitType cpexit;
	cpexit = US_ControlPanel(false);

	assert(game == NULL);
	boolean newGameOrLoadedGame = cpexit.Result == CPE_NEWGAME || cpexit.Result == CPE_LOADEDGAME;
	boolean backToGame = false;
	while (newGameOrLoadedGame) {
		assert(game == NULL);
		boolean drawEnterScreen=false;
		if (cpexit.Result == CPE_NEWGAME) {
			game = NewGame(cpexit.Difficulty);
			assert(game != NULL);
			drawEnterScreen = true;
		} else if (cpexit.Result == CPE_LOADEDGAME) {
			assert(game == NULL);
			printf("SavegameToLoad = %s\n", cpexit.SavegameToLoad);
			game = LoadTheGame(cpexit.SavegameToLoad, cpexit.SavegameSkip);
			assert(game != NULL);
			drawEnterScreen = false;
	//		gamestate = cpexit.LoadedGame;
		}
		newGameOrLoadedGame = false;

		PlayLoopExitType transition;

		do { // while (backToGame)
			SP_GameEnter();
			transition = PlayLoop(game, drawEnterScreen);
			SP_GameLeave();

			switch (transition) {
			case PLE_MENU:
				cpexit = US_ControlPanel(true);
				drawEnterScreen = false;
				backToGame = cpexit.Result == CPE_NOTHING;
				newGameOrLoadedGame = !(cpexit.Result == CPE_ABORTGAME);
				break;
			case PLE_WARPED:
				if (game->mapon >= NUMLEVELS) {
					Victory();
					CheckHighScore(game->score, game->mapon+1);
					backToGame = false;
					newGameOrLoadedGame = false;
				} else {
					SetupGameLevel(game);
					drawEnterScreen = true;
					backToGame = true;
					newGameOrLoadedGame = false; // doesn't matter
				}
				break;
			case PLE_DEATH:
				Died();
				CheckHighScore(game->score, game->mapon+1);
				backToGame = false;
				newGameOrLoadedGame = false;
				break;
			case PLE_VICTORY:
				Victory();
				CheckHighScore(game->score, game->mapon+1);
				backToGame = false;
				newGameOrLoadedGame = false;
				break;
			default:
				assert(0);
			}
		} while (backToGame);
		DestroyGame(game);
		game = NULL;
	} // while (newGameOrLoadedGame)
	return true;
}
