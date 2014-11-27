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

#include "id_heads.h"
#include "c4_audio.h"
#include "c4_gfxe.h"
#include "c4_maps.h"
#include <math.h>
#include <values.h>

extern boolean loadedgame, ingame, abortgame;
extern GameDiff restartgame;
extern gametype *gamestate;

#define tf_SOLID							0x01
#define tf_SPECIAL						0x02
#define tf_EMBEDDED_KEY_COLOR			0x04

#define TILE_FLAGS(tile)   (C4_tile_flags[(tile)])

#define GATE_KEY_COLOR(tile)		((unsigned char)(TILE_FLAGS(tile)>>4))
extern uint8_t C4_tile_flags[];


/*
=============================================================================

						 GLOBAL CONSTANTS

=============================================================================
*/

//
// MAX_BASE - represents 100 percent in 1st base
// MAX_PERC - represents 100 percent in 2nd base
// PERC - fractional portion of 2nd base
// SCALE - arbitrary scaling value (bigger number means more accurate)
//
// ex: PERCENTAGE(320,16,8,8)    returns  160
//
//
#define PERCENTAGE(MAX_BASE,MAX_PERC,PERC,SCALE) ((unsigned)(MAX_BASE*((PERC<<SCALE)/MAX_PERC))>>SCALE)

#define SIZE_TEST			0 //50000			// DO NOT USE!! ARGH!

#define FL_QUICK 			0x01
#define FL_NOMEMCHECK	0x02

#define FL_CLEAR		(FL_QUICK|FL_NOMEMCHECK)

#define GEM_SHIFT		2
#define FL_RGEM		0x04
#define FL_GGEM		0x08
#define FL_BGEM		0x10
#define FL_YGEM		0x20
#define FL_PGEM		0x40
#define FL_DEAD		0x80



#define MAXBOLTS		10
#define MAXNUKES		10
#define MAXPOTIONS	10

#define NUKE_COST		(1000)
#define BOLT_COST    (1200)
#define POTION_COST	(1300)

#define NUKE_COST_TXT		("1000")		// Allows for Q&D positioning..
#define BOLT_COST_TXT    	("1200")
#define POTION_COST_TXT		("1300")

#define RADARX	31															// bytes
#define RADARY	11                                           // pixels
#define RADAR_WIDTH     51                                  //   "
#define RADAR_HEIGHT		51                                  //   "
#define RADAR_XCENTER	((RADARX*8)+(RADAR_WIDTH/2)+3)      //   "
#define RADAR_YCENTER	((RADARY-8)+(RADAR_HEIGHT/2)+5)       //   "
#define MAX_RADAR_BLIPS	60


#define RADAR_RADIUS			17
#define RADAR_RADIUS_NSEW	15
#define RADAR_X_IRADIUS		(113/5)
#define RADAR_Y_IRADIUS		(113/7)
#define RADAR_ICON_CENTER	4						// Center offset into icon.

#define NAMESTART	180
#define REMOVED_DOOR_TILE	NAMESTART

#define NEXT_LEVEL_CODE		0xff
#define REMOVE_DOOR_CODE	0xfe
#define CANT_OPEN_CODE		0xfd
#define EXP_WALL_CODE		0xfc

#define EXPWALLSTART	8
#define NUMEXPWALLS		7
#define WALLEXP			15
#define WATEREXP			29
#define NUMFLOORS		71

#define NUMLATCHPICS	 (FIRSTWALLPIC-FIRSTLATCHPIC) //+5
#define NUMSCALEPICS	 (FIRSTWALLPIC-FIRSTSCALEPIC) //+5
#define NUMSCALEWALLS (LASTWALLPIC-FIRSTWALLPIC) //+5


#define FLASHCOLOR	12
#define FLASHTICS	4


#define NUMLEVELS	21

#define MAXBODY		100
#define MAXSHOTPOWER	56

#define SCREEN1START	0
#define SCREEN2START	8320

#define STATUSLEN			0xc80
#define PAGELEN			0x1700

#define PAGE1START	STATUSLEN
#define PAGE2START	(PAGE1START+PAGELEN)
#define PAGE3START	(PAGE2START+PAGELEN)
#define FREESTART		(PAGE3START+PAGELEN)

#define PIXRADIUS		512

#define STATUSLINES		(200-VIEWHEIGHT)

enum bonusnumbers {B_BOLT,B_NUKE,B_POTION,B_RKEY,B_YKEY,B_GKEY,B_BKEY,B_SCROLL1,
 B_SCROLL2,B_SCROLL3,B_SCROLL4,B_SCROLL5,B_SCROLL6,B_SCROLL7,B_SCROLL8,
 B_GOAL,B_CHEST,B_RGEM,B_GGEM,B_BGEM,B_YGEM,B_PGEM,B_RKEY2};


#define MAX_DOOR_STORAGE 5

#define GEM_DELAY_TIME	(120*60)

#define ROTATE_SPEED		(6)

#define WALL_SKELETON_CODE 6

#define MAXREALTICS (2*60)

#define MAXFREEZETIME			(100*30)				// 50 secs (100 half)
extern	boolean EASYMODEON;


/*
=============================================================================

						   GLOBAL TYPES

=============================================================================
*/

enum {BLANKCHAR=9,BOLTCHAR,NUKECHAR,POTIONCHAR,KEYCHARS,SCROLLCHARS=17,
	NUMBERCHARS=25};

typedef enum
  {nothing,playerobj,bonusobj,orcobj,batobj,skeletonobj,trollobj,demonobj,
  mageobj,pshotobj,bigpshotobj,mshotobj,inertobj,bounceobj,grelmobj,
  gateobj,zombieobj,spookobj,wetobj,expobj,eyeobj,wallskelobj,eshotobj,
  gshotobj,reddemonobj,freezeobj,solidobj} classtype;

#define of_shootable		0x01
#define of_damagedone	0x02





typedef enum {S_NONE,     S_HELP,    S_SND,     S_SAVING,   S_RESTORING,
			S_JOYSTICK, S_RETREAT, S_ADVANCE, S_SIDESTEP, S_QTURN,
			S_MISSLE,   S_ZAPPER,  S_XTER,    S_CURING,   S_READ,
			S_VIEWING,  S_ITEMDES,  S_DAMAGE,  S_TURN,    S_TIMESTOP} status_flags;

typedef struct {
	char x,y;
	unsigned ondoor,underdoor;
} doorinfo;

typedef struct {
	char x,y;
	short angle;
	doorinfo doors[MAX_DOOR_STORAGE];
} levelinfo;



/*

=============================================================================

						 some vars

=============================================================================
*/



extern BufferSetup renderBuffer; // for blitting
extern BufferSetup renderBufferText; // for "Entering Level XY" windows
extern BufferSetup bottomHUDBuffer;

/*
=============================================================================

						 C3_MAIN DEFINITIONS

=============================================================================
*/

extern char inlevel[][2];
extern	char		str[80];
extern	unsigned	tedlevelnum;
extern	boolean		tedlevel;
extern	exittype	playstate;
extern 	char SlowMode;
extern   unsigned Flags;


void ResetGame(void);
void ShutdownId (void);
void InitGame (void);
void DemoLoop (void);
void SetupScalePic (unsigned picnum);
void SetupScaleWall (unsigned picnum);
void SetupScaling (void);
void main (void);

/*
=============================================================================

						 C3_GAME DEFINITIONS

=============================================================================
*/

extern	unsigned	tileoffsets[NUMTILE16];
extern	unsigned	textstarts[27];


#define	L_CHARS		0
#define L_NOSHOT	1
#define L_SHOTBAR	2
#define L_NOBODY	3
#define L_BODYBAR	4


void ScanInfoPlane (void);
void ScanText (void);
void SetupGameLevel (void);
void Victory (boolean playsounds);
void Died (void);
void NormalScreen (void);
void DrawPlayScreen (void);
void FizzleOut (int showlevel);
void FreeUpMemory (void);
void GameLoop (void);


/*
=============================================================================

						 C3_PLAY DEFINITIONS

=============================================================================
*/

#define BGF_NIGHT					0x01			// it is officially night
#define BGF_NOT_LIGHTNING		0x02			// lightning flash has ended

extern byte BGFLAGS,bcolor;

extern unsigned *skycolor,*groundcolor;

extern	int			bordertime;

extern	byte		update[];

extern	boolean		godmode,singlestep;
extern	int			extravbls;

extern	status_flags    status_flag;
extern  int             status_delay;
extern  char			status_text[500];

extern 	objtype 		dummyobj;
extern 	short 		BeepTime;				//


void PicturePause (void);
void PollControlls (void);

void PlayLoop (void);
void InitBgChange(short stimer, unsigned *scolors, short gtimer, unsigned *gcolors, byte flag);

void DisplayStatus (status_flags *stat_flag);

/*
=============================================================================

						 C3_STATE DEFINITIONS

=============================================================================
*/

//void SpawnNewObj (unsigned x, unsigned y, statetype *state, unsigned size);
//void SpawnNewObjFrac (long x, long y, statetype *state, unsigned size);

objtype *Internal_SpawnNewObj (unsigned x, unsigned y, statetype *state, unsigned size,boolean UseDummy);
objtype *Internal_SpawnNewObjFrac (long x, long y, statetype *state, unsigned size,boolean UseDummy);

#define DSpawnNewObj(x, y, state, size)				Internal_SpawnNewObj(x,y,state,size,true)
#define SpawnNewObj(x, y, state, size)					Internal_SpawnNewObj(x,y,state,size,false)
#define SpawnNewObjFrac(x, y, state, size,Dummy)	Internal_SpawnNewObjFrac(x, y, state, size,false)
#define DSpawnNewObjFrac(x, y, state, size)			Internal_SpawnNewObjFrac(x, y, state, size,true)

boolean CheckHandAttack (objtype *ob);
void T_DoDamage (objtype *ob);
boolean Walk (objtype *ob);
void ChaseThink (objtype *obj, boolean diagonal);
void MoveObj (objtype *ob, long move);
boolean Chase (objtype *ob, boolean diagonal);

extern	dirtype opposite[9];

/*
=============================================================================

						 C3_TRACE DEFINITIONS

=============================================================================
*/

int FollowTrace (fixed tracex, fixed tracey, long deltax, long deltay, int max);
int BackTrace (int finish);
void ForwardTrace (void);
int FinishWall (void);
void InsideCorner (void);
void OutsideCorner (void);
void FollowWalls (void);

extern	boolean	aborttrace;

/*
=============================================================================

						 C3_DRAW DEFINITIONS

=============================================================================
*/

#define MAXWALLS	50
#define DANGERHIGH	45

#define	MIDWALL		(MAXWALLS/2)

//==========================================================================

//==========================================================================

void	DrawLine (int xl, int xh, int y,int color);
void	DrawWall (walltype *wallptr);
void	TraceRay (unsigned angle);
fixed	FixedByFrac (fixed a, fixed b);
void	TransformPoint (fixed gx, fixed gy, int *screenx, unsigned *screenheight);
fixed	TransformX (fixed gx, fixed gy);
int	FollowTrace (fixed tracex, fixed tracey, long deltax, long deltay, int max);
void	ForwardTrace (void);
int	FinishWall (void);
int	TurnClockwise (void);
int	TurnCounterClockwise (void);
void	FollowWall (void);

void	NewScene (void);
void	BuildTables (void);


/*
=============================================================================

						 C3_SCALE DEFINITIONS

=============================================================================
*/


typedef struct
{
	unsigned	width;
	unsigned	codeofs[64];
}	t_compshape;


extern unsigned	scaleblockwidth,
		scaleblockheight,
		scaleblockdest;

extern	byte	plotpix[8];
extern	byte	bitmasks1[8][8];
extern	byte	bitmasks2[8][8];


extern	t_compshape *shapedirectory[NUMSCALEPICS];
extern	memptr			walldirectory[NUMSCALEWALLS];
extern	unsigned	shapesize[NUMSCALEPICS];

void 		DeplanePic (int picnum);
void ScaleShape (int xcenter, t_compshape *compshape, unsigned scale);
unsigned	BuildCompShape (t_compshape **finalspot);


/*
=============================================================================

						 C3_ASM DEFINITIONS

=============================================================================
*/

extern int screenfaded;

void	ScaleWalls (void);

/*
=============================================================================

						 C3_WIZ DEFINITIONS

=============================================================================
*/

#define MAXHANDHEIGHT	72

extern statetype s_pshot_exp1;
extern statetype s_pshot_exp2;
extern statetype s_pshot_exp3;

extern	long	lastnuke;
extern   int lasttext;
extern	int		handheight;
extern	int		boltsleft,bolttimer;
extern short RadarXY[][3];

extern short RotateAngle;
extern short FreezeTime;

char DisplayMsg(char *text,char *choices);
char DisplaySMsg(char *text,char *choices);

extern statetype s_explode;

void SpawnExplosion(fixed x, fixed y,short Delay);
void T_ExpThink(objtype *obj);
void SpawnBigExplosion(fixed x, fixed y, short Delay, fixed Range);


/*
=============================================================================

						 C3_ACT1 DEFINITIONS

=============================================================================
*/


int EasyHitPoints(int NrmHitPts);
int EasyDoDamage(int Damage);

extern short zombie_base_delay;

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

extern	statetype s_zombie_death1;
extern	statetype s_zombie_ouch;

extern statetype s_zombie_rise1;
extern statetype s_zombie_rise2;
extern statetype s_zombie_rise3;
extern statetype s_zombie_rise4;

extern statetype s_spook0_1;
extern statetype s_spook0_2;
extern statetype s_spook0_3;
extern statetype s_spook0_4;
extern statetype s_spook0;
extern statetype s_spook1;
extern statetype s_spook2;
extern statetype s_spook3;
extern statetype s_spook4;
extern statetype s_spook5;
extern statetype s_spook6;
extern statetype s_spook_pause;
extern statetype s_spook_attack1;
extern statetype s_spook_attack2;
extern statetype s_spook_attack3;
extern statetype s_spookouch;
extern statetype s_spookdie;
extern statetype s_spookdie1;
extern statetype s_spookdie2;
extern statetype s_spookdie3;
extern statetype s_spookdie4;
extern statetype s_spookdie5;
extern statetype s_spookdie6;
extern statetype s_spookdie7;
extern statetype s_spook_wait;


extern statetype s_skel_pause;
extern statetype s_skel_1;
extern statetype s_skel_2;
extern statetype s_skel_3;
extern statetype s_skel_4;
extern statetype s_skel_attack1;
extern statetype s_skel_attack2;
extern statetype s_skel_attack3;
extern statetype s_skel_ouch;
extern statetype s_skel_die1;
extern statetype s_skel_die2;
extern statetype s_skel_die3;

extern statetype s_wet_pause;

extern statetype s_wet_bubbles1;
extern statetype s_wet_bubbles2;
extern statetype s_wet_bubbles3;
extern statetype s_wet_bubbles4;

extern statetype s_wet_peek;

extern statetype s_wet_rise1;
extern statetype s_wet_rise2;
extern statetype s_wet_rise3;
extern statetype s_wet_rise4;
extern statetype s_wet_rise5;

extern statetype s_wet_sink1;
extern statetype s_wet_sink2;
extern statetype s_wet_sink3;

extern statetype s_wet_walk1;
extern statetype s_wet_walk2;
extern statetype s_wet_walk3;
extern statetype s_wet_walk4;

extern statetype s_wet_attack1;
extern statetype s_wet_attack2;
extern statetype s_wet_attack3;
extern statetype s_wet_attack4;

extern statetype s_wet_ouch;

extern statetype s_wet_die1;
extern statetype s_wet_die2;
extern statetype s_wet_die3;
extern statetype s_wet_die4;
extern statetype s_wet_die5;

extern	statetype s_obj_gate1;
extern	statetype s_obj_gate2;
extern	statetype s_obj_gate3;
extern	statetype s_obj_gate4;

extern statetype s_eye_pause;

extern statetype s_eye_1;
extern statetype s_eye_2;
extern statetype s_eye_3;
extern statetype s_eye_4;

//statetype s_eye_attack1 = {SKELETON_ATTACK_1PIC,20,NULL,&s_eye_attack2};
//statetype s_eye_attack2 = {SKELETON_ATTACK_2PIC,10,T_DoDamage,&s_eye_attack3};
//statetype s_eye_attack3 = {SKELETON_ATTACK_3PIC,40,NULL,&s_eye_pause};

extern statetype s_eye_ouch;
extern statetype s_eye_ouch2;

extern statetype s_eye_die1;
extern statetype s_eye_die2;
extern statetype s_eye_die3;

extern	statetype s_mshot1;
extern	statetype s_mshot2;

extern statetype s_bonus_die;

extern	statetype s_red_demonouch;

extern	statetype s_red_demondie1;


