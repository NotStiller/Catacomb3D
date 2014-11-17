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

#include "id_heads.h"
#include "c3_gfxe.h"
#include "c3_audio.h"
#include "c3_maps.h"
#include <math.h>
#include <values.h>

/*
=============================================================================

						 GLOBAL CONSTANTS

=============================================================================
*/

#define NAMESTART	180


#define EXPWALLSTART	8
#define NUMEXPWALLS		7
#define WALLEXP			15
#define NUMFLOORS		36

#define NUMFLOORS	36

#define NUMLATCHPICS	100


#define FLASHCOLOR	5
#define FLASHTICS	4


#define NUMLEVELS	20

#define MAXBODY			64
#define MAXSHOTPOWER	56


enum bonusnumbers {B_BOLT,B_NUKE,B_POTION,B_RKEY,B_YKEY,B_GKEY,B_BKEY,B_SCROLL1,
 B_SCROLL2,B_SCROLL3,B_SCROLL4,B_SCROLL5,B_SCROLL6,B_SCROLL7,B_SCROLL8,
 B_GOAL,B_CHEST};


/*
=============================================================================

						   GLOBAL TYPES

=============================================================================
*/

enum {BLANKCHAR=9,BOLTCHAR,NUKECHAR,POTIONCHAR,KEYCHARS,SCROLLCHARS=17,
	NUMBERCHARS=25};

typedef enum
  {nothing,playerobj,bonusobj,orcobj,batobj,skeletonobj,trollobj,demonobj,
  mageobj,pshotobj,bigpshotobj,mshotobj,inertobj,bounceobj,grelmobj
  ,gateobj} classtype;



/*
=============================================================================


						   C3 VARS

=============================================================================
*/


extern BufferSetup renderBuffer, renderBufferText; // for "entering level" text
extern BufferSetup rightHUDBuffer, bottomHUDBuffer;
extern BufferSetup guiBuffer; 


/*
=============================================================================

						 C3_MAIN DEFINITIONS

=============================================================================
*/

extern	exittype	playstate;

void NewGame (void);
boolean	SaveTheGame(FILE *file);
boolean	LoadTheGame(FILE *file);
void ResetGame(void);
void InitGame (void);
void DemoLoop (void);
int main (int argc, char *argv[]);

/*
=============================================================================

						 C3_GAME DEFINITIONS

=============================================================================
*/

void SetupGameLevel (void);
void Victory (void);
void Died (void);
void NormalScreen (void);
void DrawPlayScreen (void);
void LoadLatchMem (void);
//void FizzleFade (unsigned width,unsigned height, boolean abortable);
void GameLoop (void);


/*
=============================================================================

						 C3_PLAY DEFINITIONS

=============================================================================
*/

extern	int			bordertime;
extern	boolean		godmode;
extern	int			pointcount,pointsleft;

void PlayLoop (void);

/*
=============================================================================

						 C3_STATE DEFINITIONS

=============================================================================
*/

objtype *SpawnNewObj (unsigned short x, unsigned short y, statetype *state, unsigned size);
objtype *SpawnNewObjFrac (long x, long y, statetype *state, unsigned size);
boolean CheckHandAttack (objtype *ob);
void T_DoDamage (objtype *ob);
boolean Walk (objtype *ob);
void ChaseThink (objtype *obj, boolean diagonal);
void MoveObj (objtype *ob, long move);
boolean Chase (objtype *ob, boolean diagonal);


/*
=============================================================================

						 C3_WIZ DEFINITIONS

=============================================================================
*/

#define MAXHANDHEIGHT	72

extern	long	lastnuke;
extern	int		handheight;
extern	int		boltsleft;

/*
=============================================================================

						 C3_ACT1 DEFINITIONS

=============================================================================
*/

extern	statetype s_trollouch;
extern	statetype s_trolldie1;


extern	statetype s_orcpause;

extern	statetype s_orc1;
extern	statetype s_orc2;
extern	statetype s_orc3;
extern	statetype s_orc4;

extern	statetype s_orcattack1;
extern	statetype s_orcattack2;
extern	statetype s_orcattack3;

extern	statetype s_orcouch;

extern	statetype s_orcdie1;
extern	statetype s_orcdie2;
extern	statetype s_orcdie3;


extern	statetype s_demonouch;
extern	statetype s_demondie1;

extern	statetype s_mageouch;
extern	statetype s_magedie1;

extern	statetype s_grelouch;
extern	statetype s_greldie1;

extern	statetype s_batdie1;
