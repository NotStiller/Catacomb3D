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
extern statetype *statetypes[]; // c3_state.c


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

/*
==================
=
= SaveTheGame
=
==================
*/

boolean	SaveTheGame(FILE *file)
{
	uint8_t *buffer, *p;
	int i, j;
	objtype *o;

	int maxsize = 20*4+2*3*64*64+MAXACTORS*27;
	p = buffer = malloc(maxsize);
	SPD_WriteU32(&p, gamestate.difficulty);
	SPD_WriteU32(&p, gamestate.mapon);
	SPD_WriteU32(&p, gamestate.bolts);
	SPD_WriteU32(&p, gamestate.nukes);
	SPD_WriteU32(&p, gamestate.potions);
	for (i = 0; i < 4; i++) {
		SPD_WriteU32(&p, gamestate.keys[i]);
	}
	for (i = 0; i < 8; i++) {
		SPD_WriteU32(&p, gamestate.scrolls[i]);
	}
	SPD_WriteU32(&p, gamestate.score);
	SPD_WriteU32(&p, gamestate.body);
	SPD_WriteU32(&p, gamestate.shotpower);

	for (i = 0;i < 2;i++)	// Write planes 0 and 2
	{
		int num=curmap->width*curmap->height;
		printf("Plane %i size %i\n", 2*i, num);
		SPD_WriteU32(&p, num);
		for (j = 0; j < num; j++) {
			SPD_WriteU16(&p, gamestate.mapsegs[2*i][j]);
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
/*	word	i,compressed,expanded;
	objtype	*o;
	memptr	bigbuffer;

	SPD_WriteToFile(file,(void*)&gamestate,sizeof(gamestate));

	expanded = mapwidth * mapheight * 2;
	MM_GetPtr (&bigbuffer,expanded);

	for (i = 0;i < 3;i+=2)	// Write planes 0 and 2
	{
		SPD_WriteToFile(file,(uint8_t*)gamestate.mapsegs[i],expanded);
	}

	for (o = player;o;o = o->next)
		SPD_WriteToFile(file,(void*)o,sizeof(objtype));

	MM_FreePtr (&bigbuffer);*/

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
	uint8_t *buffer, *p;
	int i, j;
	long size;

	p = buffer = SPD_ReadFile2(file, &size);
	printf("loaded length %i\n", size);

	gamestate.difficulty = SPD_ReadU32(&p);
	gamestate.mapon = SPD_ReadU32(&p);
	gamestate.bolts = SPD_ReadU32(&p);
	gamestate.nukes = SPD_ReadU32(&p);
	gamestate.potions = SPD_ReadU32(&p);
	for (i = 0; i < 4; i++) {
		gamestate.keys[i] = SPD_ReadU32(&p);
	}
	for (i = 0; i < 8; i++) {
		gamestate.scrolls[i] = SPD_ReadU32(&p);
	}
	gamestate.score = SPD_ReadU32(&p);
	gamestate.body = SPD_ReadU32(&p);
	gamestate.shotpower = SPD_ReadU32(&p);



	gamestate.mapsegs[1] = NULL;
	for (i = 0;i < 2;i++)	// Write planes 0 and 2
	{
		int num = SPD_ReadU32(&p);
		printf("plane %i num %i\n", 2*i, num);
		gamestate.mapsegs[2*i] = (uint16_t*)malloc(2*num);
		for (j = 0; j < num; j++) {
			gamestate.mapsegs[2*i][j] = SPD_ReadU16(&p);
		}
	}
	assert((p-buffer) <= size);

	SetupGameLevel ();		// load in and cache the base old level
	assert((p-buffer) <= size);


//
// copy the wall data to a data segment array again, to handle doors and
// bomb walls that are allready opened
//
	ClearTileMap();
	ClearActorAt();
	uint16_t *map;
	int x,y;
	map = gamestate.mapsegs[0];
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

	memset (&gamestate,0,sizeof(gamestate));

	US_Setup ();
	US_SetLoadSaveHooks(LoadTheGame,SaveTheGame,ResetGame);

//
// load in and lock down some basic chunks
//

	SPD_LoadGrChunk(STARTFONT);
	SPD_LoadGrChunk(STARTTILE8);
	SPD_LoadGrChunk(STARTTILE8M);
	SPD_LoadGrChunk(HAND1PICM);
	SPD_LoadGrChunk(HAND2PICM);
	SPD_LoadGrChunk(ENTERPLAQUEPIC);

	fontcolor = WHITE; 


//
// build some tables
//
	BuildTables ();			// 3-d tables, c3_draw.c

	SPG_SetBorderColor(3);
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
		SPD_LoadGrChunk (TITLEPIC);
		SPG_DrawPic(&guiBuffer, grsegs[TITLEPIC],0,0);
		FizzleFade (320,200,true);

		if (!IN_UserInput(TickBase*3,false))
		{
			SPD_LoadGrChunk (CREDITSPIC);
			SPG_DrawPic(&guiBuffer, grsegs[CREDITSPIC],0,0);
			FizzleFade();

		}

		if (!IN_UserInput(TickBase*3,false))
		{
			DrawHighScores ();
			FizzleFade();
			IN_UserInput(TickBase*3,false);
		}

		if (IN_IsUserInput())
		{
			US_ControlPanel ();

			if (restartgame || loadedgame)
			{
				GameLoop ();
				DrawHighScores ();
				FizzleFade();
				IN_UserInput(TickBase*3,false);
			}
		}

	}
}

//===========================================================================




