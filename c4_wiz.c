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

// C3_WIZ.C

#include "c4_def.h"
#pragma hdrstop

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

#define NUMSCROLLS	8

#define	SHOWITEMS	9

#define	NUKETIME	40
#define NUMBOLTS	10
#define BOLTTICS	6

#define STATUSCOLOR	8
#define TEXTCOLOR	14

#define SIDEBARWIDTH	5

#define BODYLINE    8
#define POWERLINE	80

#define SPECTILESTART	0			// 18

#define SHOTDAMAGE		1
#define BIGSHOTDAMAGE	3


#define PLAYERSPEED	5120
#define RUNSPEED	(8192<<1)

#define SHOTSPEED	10000

//#define LASTWALLTILE	47
//#define LASTSPECIALTILE	37

#define LASTTILE  (LASTWALLPIC-FIRSTWALLPIC)							// 47

#define FIRETIME	2

#define HANDPAUSE	60

/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

long		lastnuke,lasthand;
int			lasttext;
int			handheight;
int			boltsleft,bolttimer;
short RadarXY[MAX_RADAR_BLIPS][3]={-1,-1,-1};
short radarx=RADARX,radary=RADARY,radar_xcenter=RADAR_XCENTER,radar_ycenter=RADAR_YCENTER;
int key_x[4]={20,23,23,20},key_y[4]={30,57,30,57};

boolean redraw_gems,button0down;

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

int			lastradar;
unsigned	lastfiretime;

int	strafeangle[9] = {0,90,180,270,45,135,225,315,0};

short RotateAngle = -1;				// -1 == No Angle to turn to...
short FreezeTime = 0;				// Stops all think (except player)
short RotateSpeed;					// Speed (and dir) to rotate..

short turntime = 0;					// accumulated time for fast turning..

//===========================================================================

void CalcBounds(objtype *ob);
boolean VerifyGateExit(void);
void DrawNSEWIcons(void);
void DrawGems(void);
void DrawRadar (void);
void DrawChar (unsigned x, unsigned y, unsigned tile);
void RedrawStatusWindow (void);
void GiveBolt (void);
void TakeBolt (void);
void GiveNuke (void);
void TakeNuke (void);
void GivePotion (void);
void TakePotion (void);
void GiveKey (int keytype);
void TakeKey (int keytype);
void GiveScroll (int scrolltype,boolean show);
void ReadScroll (int scroll);
void DrawScrolls(void);

void DrawNum(short x,short y,short value,short maxdigits);

//----------

void Shoot (void);
void CastBolt (void);
void CastNuke (void);
void DrinkPotion (void);

//----------
void DrawHealth(void);

void SpawnPlayer (int tilex, int tiley, int dir);
void Thrust (int angle, unsigned speed);
void T_Player (objtype *ob);

//void AddPoints (int points);

void ClipMove (objtype *ob, long xmove, long ymove);
boolean ShotClipMove (objtype *ob, long xmove, long ymove);

//===========================================================================


/*
===============
=
= DrawChar
=
===============
*/


void DrawChar (unsigned x, unsigned y, unsigned tile)
{
	SPG_DrawTile8(&bottomHUDBuffer, 8*x,y,(byte*)grsegs[STARTTILE8]+64*tile);
}



//===========================================================================

/*
===============
=
= DrawFreezeTime
=
===============
*/
void DrawFreezeTime()
{
	long percentage;
	percentage = PERCENTAGE(100,MAXFREEZETIME,(long)FreezeTime,7);
	DrawNum(23,70,percentage,3);
}

//===========================================================================

/*
===============
=
= RedrawStatusWindow
=
===============
*/

void RedrawStatusWindow (void)
{
	short keytype;

//	EGABITMASK(0xff);
	for (keytype=0; keytype<4; keytype++)
		DrawNum(key_x[keytype],key_y[keytype],gamestate->keys[keytype],2);
	DrawNum(17,54,gamestate->potions,2);
	DrawNum(17,36,gamestate->nukes,2);
	DrawNum(17,18,gamestate->bolts,2);

	DrawHealth();
	if (FreezeTime)
		DrawFreezeTime();
	DrawRadar();
	DrawNSEWIcons();
	DrawGems();
	DrawScrolls();
	DrawText();
}


//===========================================================================

/*
===============
=
= GiveBolt
=
===============
*/

void GiveBolt (void)
{
	if (gamestate->bolts == 99)
		return;

	SPA_PlaySound (GETBOLTSND);
	DrawNum(17,18,++gamestate->bolts,2);
}


/*
===============
=
= TakeBolt
=
===============
*/

void TakeBolt (void)
{
	SPA_PlaySound (USEBOLTSND);
	DrawNum(17,18,--gamestate->bolts,2);
}

//===========================================================================

/*
===============
=
= GiveNuke
=
===============
*/

void GiveNuke (void)
{
	if (gamestate->nukes == 99)
		return;

	SPA_PlaySound (GETNUKESND);
	DrawNum(17,36,++gamestate->nukes,2);
}


/*
===============
=
= TakeNuke
=
===============
*/

void TakeNuke (void)
{
	SPA_PlaySound (USENUKESND);
	DrawNum(17,36,--gamestate->nukes,2);
}

//===========================================================================

/*
===============
=
= GivePotion
=
===============
*/

void GivePotion (void)
{
	if (gamestate->potions == 99)
		return;

	SPA_PlaySound (GETPOTIONSND);
	DrawNum(17,54,++gamestate->potions,2);
}


/*
===============
=
= TakePotion
=
===============
*/

void TakePotion (void)
{
	SPA_PlaySound (USEPOTIONSND);
	DrawNum(17,54,--gamestate->potions,2);
}

//===========================================================================

/*
===============
=
= GiveKey
=
===============
*/

void GiveKey (int keytype)
{
	int	i,j,x;

	if (gamestate->keys[keytype] == 99)
		return;

	SPA_PlaySound (GETKEYSND);
	DrawNum(key_x[keytype],key_y[keytype],++gamestate->keys[keytype],2);
}


/*
===============
=
= TakeKey
=
===============
*/

void TakeKey (int keytype)
{
	int	i,j,x;
	char *key_colors[] = {"a RED key",
								 "a YELLOW key",
								 "a GREEN key",
								 "a BLUE key"};


	SPA_PlaySound (USEKEYSND);
	DrawNum(key_x[keytype],key_y[keytype],--gamestate->keys[keytype],2);
	Win_Create(&renderBufferText, 20,5);
	Win_CPrint("\nYou use\n");
	Win_CPrint(key_colors[keytype]);
	SPG_FlipBuffer();
	VW_WaitVBL(120);
}


//===========================================================================

/*
===============
=
= GiveGem
=
===============
*/

void GiveGem (int gemtype)
{
#if 0
	int	i,j,x;

	SPA_PlaySound (GETKEYSND);
	DrawNum(key_x[keytype],key_y[keytype],++gamestate->keys[keytype],2);
#endif
}


/*
===============
=
= TakeGem
=
===============
*/

void TakeGem (int gemtype)
{
#if 0
	int	i,j,x;

	SPA_PlaySound (USEKEYSND);
	DrawNum(key_x[keytype],key_y[keytype],--gamestate->keys[keytype],2);
#endif
}

/*
===============
=
= DrawGem
=
===============
*/

void DrawGems()
{
	short loop;

	redraw_gems = false;

//	LatchDrawPic (31,51,RADAR_BOTTOMPIC);
	SPG_DrawPic(&bottomHUDBuffer, grsegs[RADAR_BOTTOMPIC], 31*8,51);
	for (loop=0; loop<5; loop++)
		if (gamestate->gems[loop])
			SPG_DrawPic(&bottomHUDBuffer, grsegs[RADAR_RGEMPIC+loop], (32+loop)*8,53);
//			LatchDrawPic (32+loop,53,RADAR_RGEMPIC+loop);
}

//===========================================================================

/*
===============
=
= GiveScroll
=
===============
*/

void GiveScroll (int scrolltype,boolean show)
{
	int	i,j,x,y,scrollnum;

	SPA_PlaySound (GETSCROLLSND);
	gamestate->scrolls[scrolltype] = true;

	y = 30 + ((scrolltype > 3) * 10);
	x = 26 + (scrolltype % 4);
	DrawChar(x,y,SCROLLCHARS+scrolltype);

	if (show)
		ReadScroll(scrolltype);
}

/*
===============
=
= DrawScrolls
=
= Force draw of all scrolls
=
===============
*/
void DrawScrolls()
{
	int loop,x,y;

	SPG_Bar(&bottomHUDBuffer, 210,30,30,18,0xf);

	for (loop=0;loop<8;loop++)
		if (gamestate->scrolls[loop])
		{
			y = 30 + ((loop > 3) * 10);
			x = 26 + (loop % 4);
			DrawChar(x,y,SCROLLCHARS+loop);
		}
}


//===========================================================================

/*
===============
=
= DrawHealth
=
===============
*/
void DrawHealth()
{
	char picnum;
	int percentage;

	percentage = PERCENTAGE(100,MAXBODY,gamestate->body,9);

	DrawNum(9,57,percentage,3);

	if (percentage > 75)
		picnum = FACE1PIC;
	else
	if (percentage > 50)
		picnum = FACE2PIC;
	else
	if (percentage > 25)
		picnum = FACE3PIC;
	else
	if (percentage)
		picnum = FACE4PIC;
	else
	{
		picnum = FACE5PIC;
		SPD_LoadGrChunk (picnum);
	}

	if (!percentage)
	{
		SPG_DrawPic(&bottomHUDBuffer, grsegs[picnum], 8*8,14);
//		DrawPic(8,14,picnum);
	}
	else
		SPG_DrawPic(&bottomHUDBuffer, grsegs[picnum], 8*8,14);
//		LatchDrawPic(8,14,picnum);
}

//===========================================================================

/*
===============
=
= DrawNum
=
===============
*/
void DrawNum(short x,short y,short value,short maxdigits)
{
	char str[10],len,i;

	snprintf(str, 10, "%i", value);
	len=strlen(str);

	for (i=len; i<maxdigits; i++)
		DrawChar(x++,y,BLANKCHAR);

	for (i=0;i<len;i++)
		DrawChar(x++,y,NUMBERCHARS+str[i]-'0');
}

//===========================================================================

/*
===============
=
= GiveChest
=
===============
*/

void GiveChest(void)
{
	char i;

	for (i=0;i<RANDOM(4);i++)
	{
		GiveBolt();
		SPG_FlipBuffer();
		SPA_WaitUntilSoundIsDone();
	}

	for (i=0;i<RANDOM(3);i++)
	{
		GiveNuke();
		SPG_FlipBuffer();
		SPA_WaitUntilSoundIsDone();
	}

	for (i=0;i<RANDOM(2);i++)
	{
		GivePotion();
		SPG_FlipBuffer();
		SPA_WaitUntilSoundIsDone();
	}
}


//===========================================================================

/*
===============
=
= GiveGoal
=
===============
*/

void GiveGoal (void)
{
	SPA_PlaySound (GETPOINTSSND);
	playstate = ex_victorious;
}


/*
===============
=
= DrawText
=
===============
*/

void DrawText (void)
{
	unsigned	number;
	char		str[80];
	char 		*text;

	//
	// draw a new text description if needed
	//
//	number = *(byte*)(gamestate->mapsegs[0]+player->tiley*mapwidth+player->tilex)-NAMESTART;
	number = (int)GetMapSegs(0,player->tilex,player->tiley) - NAMESTART;

	if ( number>26 )
		number = 0;

	lasttext = number;
	text = curmap->texts[number];

	if (text == NULL) {
		memset(str, 0, 80);
	} else {
		memcpy (str,text,80);
	}

	DisplayMsg(str,NULL);
}

//===========================================================================

/*
===============
=
= DisplayMsg
=
===============
*/

char DisplayMsg(char *text,char *choices)
{
	char ch=true;
	short temp;

	PrintY = 1;
	WindowX = 20;
	WindowW = 270;

	Win_Clear(&bottomHUDBuffer, 20,2,270,8, STATUSCOLOR, TEXTCOLOR);
	Win_CPrintLine (text);
//	SPG_Bar(&bottomHUDBuffer, WindowX,2,WindowW,8,STATUSCOLOR);
//	temp = fontcolor;
//	fontcolor = TEXTCOLOR^STATUSCOLOR;
//	fontcolor = temp;

	if (choices)
	{
		SPG_FlipBuffer();
		ch=GetKeyChoice(choices,true);
//		LastScan = 0;
	}

	return(ch);
}

/*
===============
=
= DisplaySMsg
=
===============
*/
char DisplaySMsg(char *text,char *choices)
{
	char ch=true;
	short temp;

	PrintY = 69;
	WindowX = 98;
	WindowW = 115;

	Win_Clear(&bottomHUDBuffer, 98,70,115,8, STATUSCOLOR, TEXTCOLOR);
	Win_CPrintLine (text);
	if (text != status_text) {
		strcpy(status_text, text);
	}

	if (choices)
	{
		SPG_FlipBuffer();
		ch=GetKeyChoice(choices,true);
//		LastScan = 0;
	}

	return(ch);
}

//===========================================================================




/*
===============
=
= RadarBlip
=
= Displays a 'blip' (1 pixel wide X 2 pixel high) on the radar at
= an (X,Y) relative to (RADAR_X,RADAR_Y) (defined below...)
=
===============
*/

void RadarBlip (int X, int Y, int Color)
{
	SPG_Bar(&bottomHUDBuffer, X, Y, 1, 2, Color);
}

/*
===============
=
= DrawRadar
=
===============
*/

void DrawRadar (void)
{
	short objnum;

	objnum = 0;
	while (RadarXY[objnum][2] != -1)
	{
		RadarBlip(radar_xcenter+RadarXY[objnum][0],radar_ycenter+RadarXY[objnum][1],RadarXY[objnum][2]);
		objnum++;
	}
}

//===========================================================================


//--------------------------------------------------------------------------
// DrawNSEWIcons(void)
//--------------------------------------------------------------------------

void DrawRadarObj(short dx, short dy, unsigned sprnum,signed long psin,signed long pcos);

void DrawNSEWIcons()
{
	signed x,y;

	x = -FixedByFrac(RADAR_X_IRADIUS,costable[player->angle]);
	y = -FixedByFrac(RADAR_Y_IRADIUS,sintable[player->angle]);

//	VWB_DrawSprite(radar_xcenter+x-3,radar_ycenter+y-3,NORTHICONSPR);
	SPG_DrawPic(&bottomHUDBuffer, grsegs[NORTHICONSPR], (radar_xcenter+x-4),(radar_ycenter+y+1-3)); // changed coords to make it work...
}


/*
=============================================================================

							SHOTS

=============================================================================
*/

void T_Pshot (objtype *ob);


extern	statetype s_pshot1;
extern	statetype s_pshot2;

//extern	statetype s_bigpshot1;
//extern	statetype s_bigpshot2;


statetype s_pshot1 = {PSHOT1PIC,8,&T_Pshot,&s_pshot2};
statetype s_pshot2 = {PSHOT2PIC,8,&T_Pshot,&s_pshot1};


statetype s_pshot_exp1 = {PSHOT_EXP1PIC,7,NULL,&s_pshot_exp2};
statetype s_pshot_exp2 = {PSHOT_EXP2PIC,7,NULL,&s_pshot_exp3};
statetype s_pshot_exp3 = {PSHOT_EXP3PIC,7,NULL,NULL};


//statetype s_shotexplode = {PSHOT2PIC,8,NULL,NULL};

//statetype s_bigpshot1 = {BIGPSHOT1PIC,8,&T_Pshot,&s_bigpshot2};
//statetype s_bigpshot2 = {BIGPSHOT2PIC,8,&T_Pshot,&s_bigpshot1};


/*
===================
=
= SpawnPShot
=
===================
*/

void SpawnPShot (void)
{
	objtype *new;
	new = DSpawnNewObjFrac (player->x,player->y,&s_pshot1,PIXRADIUS*7);
	if (new != NULL) {
		new->obclass = pshotobj;
		new->speed = SHOTSPEED;
		new->angle = player->angle;
		new->active = always;
	}
}

/*
===================
=
= JimsShotClipMove
=
= Only checks corners, so the object better be less than one tile wide!
=
===================
*/
boolean JimsShotClipMove (objtype *ob, long xmove, long ymove)
{
	int			xl,yl,xh,yh,tx,ty,nt1,nt2,x,y;
	long		intersect,basex,basey,pointx,pointy;
	unsigned	inside,total,tile;
	objtype		*check;
	boolean		moveok;

//
// move player and check to see if any corners are in solid tiles
//
//	basex = ob->x;
//	basey = ob->y;

//	ob->x += xmove;
//	ob->y += ymove;

//	CalcBounds (ob);

	xl = ob->xl>>TILESHIFT;
	yl = ob->yl>>TILESHIFT;

	xh = ob->xh>>TILESHIFT;
	yh = ob->yh>>TILESHIFT;

	for (y=yl;y<=yh;y++)
		for (x=xl;x<=xh;x++)
		{
			check = GetActorAt(x,y);

			if ((!check) || (check == player) || (!(check->flags & of_shootable)))
				continue;

			ob->x -= xmove;
			ob->y -= ymove;

			if (check->obclass != solidobj)
			{
				SPA_PlaySound (SHOOTMONSTERSND);
				if (ob->obclass == bigpshotobj)
					ShootActor (check,BIGSHOTDAMAGE);
				else
					ShootActor (check,SHOTDAMAGE);
			}
			ob->state = &s_pshot_exp1;
			ob->ticcount = ob->state->tictime;
			return(true);
		}

	return(false);		// move is OK!

}


/*
===============
=
= T_Pshot
=
===============
*/
void T_Pshot (objtype *ob)
{
	objtype	*check;
	long	xmove,ymove,speed;
	int			xl,yl,xh,yh,tx,ty,nt1,nt2,x,y;
	long		intersect,basex,basey,pointx,pointy;
	unsigned	inside,total,tile;
	boolean		moveok;

//
// check current position for monsters having moved into it
//
	for (check = player->next; check; check=check->next)
		if ((check->flags & of_shootable)
		&& ob->xl <= check->xh
		&& ob->xh >= check->xl
		&& ob->yl <= check->yh
		&& ob->yh >= check->yl)
		{

			if (check->obclass != solidobj)
			{
				SPA_PlaySound (SHOOTMONSTERSND);
				if (ob->obclass == bigpshotobj)
					ShootActor (check,BIGSHOTDAMAGE);
				else
					ShootActor (check,SHOTDAMAGE);
			}

			ob->state = &s_pshot_exp1;
			ob->obclass = expobj;
			ob->ticcount = ob->state->tictime;
			return;
		}


//
// move ahead, possibly hitting a wall
//
	speed = ob->speed*tics;

	xmove = FixedByFrac(speed,costable[ob->angle]);
	ymove = -FixedByFrac(speed,sintable[ob->angle]);

	if (ShotClipMove(ob,xmove,ymove))
	{
		ob->state = &s_pshot_exp1;
		ob->obclass = expobj;
		ob->ticcount = ob->state->tictime;
		return;
	}

	ob->tilex = ob->x >> TILESHIFT;
	ob->tiley = ob->y >> TILESHIFT;

//
// check final position for monsters hit
//

#warning why did the following line use obj instead of ob, does it make a difference ?
//	JimsShotClipMove(obj,xmove,ymove);
	JimsShotClipMove(ob,xmove,ymove);

}


/*
=============================================================================

							PLAYER ACTIONS

=============================================================================
*/


//===========================================================================

/*
===============
=
= Shoot
=
===============
*/

void Shoot (void)
{
	gamestate->shotpower = 0;
	SPA_PlaySound (SHOOTSND);
	SpawnPShot ();
}

//===========================================================================

/*
===============
=
= CastBolt
=
===============
*/

void CastBolt (void)
{
	if (!gamestate->bolts)
	{
		SPA_PlaySound (NOITEMSND);
		return;
	}

	TakeBolt ();
	boltsleft = NUMBOLTS;
	bolttimer = BOLTTICS;
	Shoot ();
}


/*
===============
=
= ContinueBolt
=
===============
*/

void ContinueBolt (void)
{
	bolttimer-=realtics;
	if (bolttimer<0)
	{
		boltsleft--;
		bolttimer = BOLTTICS;
		Shoot ();
	}
}


//===========================================================================

/*
===============
=
= CastNuke
=
===============
*/

void CastNuke (void)
{
	int	angle;

	if (!gamestate->nukes)
	{
		SPA_PlaySound (NOITEMSND);
		return;
	}

	TakeNuke ();
	lastnuke = SP_TimeCount();

	for (angle = 0; angle < ANGLES; angle+= ANGLES/16)
	{
		objtype *new;
		new = DSpawnNewObjFrac (player->x,player->y,&s_pshot1,24*PIXRADIUS);
		if (new != NULL) {
			new->obclass = bigpshotobj;
			new->speed = SHOTSPEED;
			new->angle = angle;
			new->active = always;
		}
	}
}

//===========================================================================

/*
===============
=
= DrinkPotion
=
===============
*/

void DrinkPotion (void)
{
	unsigned	source,dest,topline;

	if (!gamestate->potions)
	{
		SPA_PlaySound (NOITEMSND);
		return;
	}

	DisplaySMsg("Curing", NULL);
	TakePotion ();
	gamestate->body = MAXBODY;
	VW_WaitVBL(30);
	status_flag    = S_NONE;
}



//===========================================================================

/*
===============
=
= ReadScroll
=
===============
*/

extern	boolean	tileneeded[NUMFLOORS];

void ReadScroll (int scroll)
{
	int	i;

	DisplaySMsg("Reading Scroll", NULL);

	if (status_flag != S_TIMESTOP)
		status_flag = S_NONE;

	SPD_LoadGrChunk (SCROLLTOPPIC);
	SPD_LoadGrChunk (SCROLL1PIC + scroll);
	SPD_LoadGrChunk (SCROLLBOTTOMPIC);

	SPG_Bar(&renderBufferText, 0,0,renderBufferText.Width,renderBufferText.Height, 0);
/*	DrawPic (10,0,SCROLLTOPPIC);
	DrawPic (10,32,SCROLL1PIC + scroll);
	DrawPic (10,88,SCROLLBOTTOMPIC);*/
	SPG_DrawPic(&renderBufferText, grsegs[SCROLLTOPPIC], 10,0);
	SPG_DrawPic(&renderBufferText, grsegs[SCROLL1PIC + scroll], 10,32);
	SPG_DrawPic(&renderBufferText, grsegs[SCROLLBOTTOMPIC], 10,88);

/*	MM_FreePtr (&grsegs[SCROLL1PIC + scroll]);
	MM_FreePtr (&grsegs[SCROLLTOPPIC]);
	MM_FreePtr (&grsegs[SCROLLBOTTOMPIC]);*/

	CacheScaleds();

	SPI_ClearKeysDown ();
// MDM begin
	lasttext = -1;
	DisplayMsg("Press ENTER or ESC to exit.",NULL);
	SPG_FlipBuffer();
	while ((!SPI_GetKeyDown(sc_Escape)) && (!SPI_GetKeyDown(sc_Enter)));
// MDM end
	SPI_ClearKeysDown ();

	if (status_flag == S_TIMESTOP)
		DisplaySMsg("Time Stopped:     ",NULL);
}


//===============
//
// StopTime()
//
//
//===============
void StopTime()
{
	FreezeTime = MAXFREEZETIME;
	SPA_PlaySound(FREEZETIMESND);
	DisplaySMsg("Time Stopped:     ",NULL);
	status_flag = S_TIMESTOP;
}


/*
===============
=
= TakeDamage
=
===============
*/

void TakeDamage (int points)
{
	unsigned	source,dest,topline;

	if (!gamestate->body || (bordertime && bcolor==FLASHCOLOR) || godmode)
		return;

	if (points >= gamestate->body)
	{
		points = gamestate->body;
		Flags |= FL_DEAD;
	}

	bordertime = FLASHTICS<<2;
	bcolor = FLASHCOLOR;
	SPG_SetBorderColor(FLASHCOLOR);

	DisplaySMsg("Damaging blows!", NULL);
	status_flag  = S_NONE;
	status_delay = 80;

	if (gamestate->body<MAXBODY/3)
		SPA_PlaySound (TAKEDMGHURTSND);
	else
		SPA_PlaySound (TAKEDAMAGESND);

	gamestate->body -= points;
}

/*
=============================================================================

							INTERACTION

=============================================================================
*/


/*
==================
=
= HitSpecialTile
=
= Returns true if the move is blocked
=
==================
*/

boolean HitSpecialTile (unsigned x, unsigned y, unsigned tile)
{
	objtype *check;
	short keyspot;
	unsigned	temp,spot,newlevel;
	char *key_colors[] = {"a RED key",
								 "a YELLOW key",
								 "a GREEN key",
								 "a BLUE key"};

	switch (tile)
	{
		case 28:
			playstate = ex_victorious;
		break;

		case 36:			// Water gate
		case 24:			// Steel gate (BLUE KEY REQ.)
		case 25:			// Steel gate (RED KEY REQ.)
		case 26:			// Steel gate (YELLOW KEY REQ.)
		case 27:			// HOT Steel gate (YELLOW KEY REQ.)
		case 18:			// Wooden Doorway
		case 19:			// Wooden doorway with a GLOW - Oh, THATS's what IT is!
		case 1:			// tile warp
		case 54:			// DOWN stairs
		case 56:			// UP stairs
		case 51:			// generic door
			if (!playstate && !FreezeTime)
			{

			// Is this an openable door? (Is "openable" a word?)
			//
//				spot = (*(gamestate->mapsegs[2]+y*mapwidth+x)) >> 8;
				spot = GetMapSegs(2,x,y) >> 8;
				if (spot == CANT_OPEN_CODE)	// CAN'T EVER OPEN (it's just for looks)
				{
					Win_Create(&renderBufferText, 20,4);
					Win_CPrint("\nThe door is blocked");
					SPG_FlipBuffer();
					SPI_ClearKeysDown();
					SPI_WaitForever();
					return;
				}

				// make sure player has key to get into door
				//

				if (TILE_FLAGS(tile) & tf_EMBEDDED_KEY_COLOR)
					keyspot = GATE_KEY_COLOR(tile);
				else
					keyspot = GetMapSegs(2,x,y+1)>>8;
//					keyspot = (*(gamestate->mapsegs[2]+(y+1)*mapwidth+x)) >> 8;

				if (keyspot--)
					if (!gamestate->keys[keyspot])
					{
						SPA_PlaySound(HIT_GATESND);
						Win_Create(&renderBufferText, 20,5);
						Win_CPrint("\nYou need\n");
						Win_CPrint(key_colors[keyspot]);
						SPG_FlipBuffer();
						SPI_ClearKeysDown();
						SPI_WaitForever();
						return;
					}

			//
			// deal with this gate (warp? simply open? whatever...)
			//
				switch (spot)
				{
					case NEXT_LEVEL_CODE:		// WARP TO NEXT LEVEL
						newlevel = gamestate->mapon+1;
						playstate = ex_warped;
					break;

					case REMOVE_DOOR_CODE:		// REMOVE DOOR
						SetTileMap(x,y,0);
						SetActorAt(x,y,0);
						SetMapSegs(0,x,y,0);
						SetMapSegs(2,x,y+1,0);
/*						CASTAT(unsigned,actorat[x][y]) =
							tilemap[x][y] =	
							*(gamestate->mapsegs[0]+y*mapwidth+x) = 0;
						*(gamestate->mapsegs[2]+(y+1)*mapwidth+x) = 0;	// key no longer needed*/
						if (keyspot>=0)
							TakeKey(keyspot);
					break;

					default:			// WARP TO A LEVEL
						newlevel = spot;
						playstate = ex_warped;
					break;
				}

				if (playstate == ex_warped)
				{
					SPA_PlaySound(HIT_GATESND);
//					levelinfo *li=&gamestate->levels[curmap];

//					OldAngle = FaceDoor(x,y);

					if (!VerifyGateExit())
					{
						SPI_ClearKeysDown ();
						playstate = ex_stillplaying;
						break;
					}

//					FaceAngle(OldAngle);

					if (keyspot>=0)
						TakeKey(keyspot);
					SetMapSegs(2,x,y+1,0);// key no longer needed
//					*(gamestate->mapsegs[2]+mapwidth*(y+1)+x) = 0;	

					gamestate->mapon = newlevel;
					SPA_PlaySound(WARPUPSND);
					SPI_ClearKeysDown ();

//					li->x = player->tilex;
//					li->y = player->tiley;
//					li->angle = player->angle+180;
//					if (li->angle > 360)
//						li->angle -= 360;
				}
			}
		break;
	}

	return true;
}

//-------------------------------------------------------------------------
// VerifyGateExit()
//-------------------------------------------------------------------------
boolean VerifyGateExit()
{
	char choices[] = {sc_Escape,sc_Y,sc_N,0},ch;

	ch=DisplayMsg("Pass this way?      Y/N",choices);
	DrawText();

	return(ch == sc_Y);
}


/*
==================
=
= TouchActor
=
= Returns true if the move is blocked
=
==================
*/

boolean TouchActor (objtype *ob, objtype *check)
{
	if (ob->xh < check->xl || ob->xl > check->xh ||
		ob->yh < check->yl || ob->yl > check->yh)
		return false;				// not quite touching

	switch (check->obclass)
	{
		case bonusobj:
			switch (check->temp1)
			{
				case B_BOLT:		GiveBolt ();		break;

				case B_NUKE:		GiveNuke ();		break;

				case B_POTION:		GivePotion ();		break;

				case B_RKEY2:		GiveKey(B_RKEY-B_RKEY);					break;

				case B_RKEY:
				case B_YKEY:
				case B_GKEY:
				case B_BKEY:		GiveKey (check->temp1-B_RKEY);		break;

				case B_SCROLL1:
				case B_SCROLL2:
				case B_SCROLL3:
				case B_SCROLL4:
				case B_SCROLL5:
				case B_SCROLL6:
				case B_SCROLL7:
				case B_SCROLL8:	GiveScroll (check->temp1-B_SCROLL1,true);	break;

				case B_CHEST:		GiveChest (); 		break;

				case B_RGEM:
				case B_YGEM:
				case B_GGEM:
				case B_BGEM:
				case B_PGEM:
					SPA_PlaySound(GETGEMSND);
					gamestate->gems[check->temp1-B_RGEM] = GEM_DELAY_TIME;
					redraw_gems = true;
				break;

				default:
					Quit("TouchActor(): INVALID BONUS");
				break;
			}

			SetActorAt(check->tilex,check->tiley,NULL);
			RemoveObj (check);

			return false;
		break;

		case freezeobj:
			StopTime();
			SetActorAt(check->tilex,check->tiley,NULL);
			RemoveObj(check);
			return(false);
		break;

	}

	return	true;
}


/*
==================
=
= CalcBounds
=
==================
*/

void CalcBounds (objtype *ob)
{
//
// calculate hit rect
//
  ob->xl = ob->x - ob->size;
  ob->xh = ob->x + ob->size;
  ob->yl = ob->y - ob->size;
  ob->yh = ob->y + ob->size;
}


/*
===================
=
= LocationInActor
=
===================
*/

boolean LocationInActor (objtype *ob)
{
	int	x,y,xmin,ymin,xmax,ymax;
	objtype *check;

	CalcBounds (ob);

	xmin = (ob->x >> TILESHIFT)-2;
	ymin = (ob->y >> TILESHIFT)-2;
	xmax = xmin+5;
	ymax = ymin+5;

	for (x=xmin;x<xmax;x++)
		for (y=ymin;y<ymax;y++)
		{
			check = GetActorAt(x,y);
			if (check>(objtype *)LASTTILE
				&& (check->flags & of_shootable)
				&& ob->xl-SIZE_TEST <= check->xh
				&& ob->xh+SIZE_TEST >= check->xl
				&& ob->yl-SIZE_TEST <= check->yh
				&& ob->yh+SIZE_TEST >= check->yl)
					return true;
		}

	return false;
}

/*
===================
=
= ClipXMove
=
= Only checks corners, so the object better be less than one tile wide!
=
===================
*/
void ClipXMove (objtype *ob, long xmove)
{
	int			xl,yl,xh,yh,tx,ty,nt1,nt2,x,y;
	long		intersect,basex,basey,pointx,pointy;
	unsigned	inside,total,tile;
	objtype		*check;
	boolean		moveok;

//
// move player and check to see if any corners are in solid tiles
//
	basex = ob->x;
	basey = ob->y;

	ob->x += xmove;

	CalcBounds (ob);

	xl = ob->xl>>TILESHIFT;
	yl = ob->yl>>TILESHIFT;

	xh = ob->xh>>TILESHIFT;
	yh = ob->yh>>TILESHIFT;

	for (y=yl;y<=yh;y++)
		for (x=xl;x<=xh;x++)
		{
			check = GetActorAt(x,y);

			if (!check)
				continue;		// blank floor, walk ok

			if ((unsigned)check <= LASTTILE)
			{
				if (TILE_FLAGS((unsigned)check) & tf_SPECIAL)
				{
					HitSpecialTile(x,y,(unsigned)check-SPECTILESTART);
					goto blockmove;
				}

				if (TILE_FLAGS((unsigned)check) & tf_SOLID)
				{
					goto blockmove;			// solid wall
				}
			}

			TouchActor(ob,check);		// pick up items
		}

//
// check nearby actors
//
	if (LocationInActor(ob))
	{
		ob->x -= xmove;
		if (LocationInActor(ob))
		{
			ob->x += xmove;
			if (LocationInActor(ob))
				ob->x -= xmove;
		}
	}
	return;		// move is OK!


blockmove:

//	if (!SPA_IsAnySoundPlaying())
//		SPA_PlaySound (HITWALLSND);

	moveok = false;

	do
	{
		xmove /= 2;
		if (moveok)
		{
			ob->x += xmove;
		}
		else
		{
			ob->x -= xmove;
		}
		CalcBounds (ob);
		xl = ob->xl>>TILESHIFT;
		yl = ob->yl>>TILESHIFT;
		xh = ob->xh>>TILESHIFT;
		yh = ob->yh>>TILESHIFT;
		if (GetTileMap(xl,yl) || GetTileMap(xh,yl)
		|| GetTileMap(xh,yh) || GetTileMap(xl,yh) )
		{
			moveok = false;
			if (xmove>=-2048 && xmove <=2048)
			{
				ob->x = basex;
				ob->y = basey;
				return;
			}
		}
		else
		{
			if (xmove>=-2048 && xmove <=2048)
				return;
			moveok = true;
		}
	} while (1);
}


/*
===================
=
= ClipYMove
=
= Only checks corners, so the object better be less than one tile wide!
=
===================
*/
void ClipYMove (objtype *ob, long ymove)
{
	int			xl,yl,xh,yh,tx,ty,nt1,nt2,x,y;
	long		intersect,basex,basey,pointx,pointy;
	unsigned	inside,total,tile;
	objtype		*check;
	boolean		moveok;

//
// move player and check to see if any corners are in solid tiles
//
	basex = ob->x;
	basey = ob->y;

	ob->y += ymove;

	CalcBounds (ob);

	xl = ob->xl>>TILESHIFT;
	yl = ob->yl>>TILESHIFT;

	xh = ob->xh>>TILESHIFT;
	yh = ob->yh>>TILESHIFT;

	for (y=yl;y<=yh;y++)
		for (x=xl;x<=xh;x++)
		{
			check = GetActorAt(x,y);
			if (!check)
				continue;		// blank floor, walk ok

			if ((unsigned)check <= LASTTILE)
			{
				if (TILE_FLAGS((unsigned)check) & tf_SPECIAL)		// <=LASTSPECIALTILE)
				{
					HitSpecialTile (x,y,(unsigned)check-SPECTILESTART);
					goto blockmove;
				}

				if (TILE_FLAGS((unsigned)check) & tf_SOLID)		// LASTWALLTILE)
				{
					goto blockmove;	// solid wall
				}
			}

			TouchActor(ob,check);		// pick up items
		}

//
// check nearby actors
//
	if (LocationInActor(ob))
	{
		if (LocationInActor(ob))
		{
			ob->y -= ymove;
		}
	}
	return;		// move is OK!


blockmove:

//	if (!SPA_IsAnySoundPlaying())
//		SPA_PlaySound (HITWALLSND);

	moveok = false;

	do
	{
		ymove /= 2;
		if (moveok)
		{
			ob->y += ymove;
		}
		else
		{
			ob->y -= ymove;
		}
		CalcBounds (ob);
		xl = ob->xl>>TILESHIFT;
		yl = ob->yl>>TILESHIFT;
		xh = ob->xh>>TILESHIFT;
		yh = ob->yh>>TILESHIFT;
		if (GetTileMap(xl,yl) || GetTileMap(xh,yl)
		|| GetTileMap(xh,yh) || GetTileMap(xl,yh) )
		{
			moveok = false;
			if (ymove>=-2048 && ymove <=2048)
			{
				ob->x = basex;
				ob->y = basey;
				return;
			}
		}
		else
		{
			if (ymove>=-2048 && ymove <=2048)
				return;
			moveok = true;
		}
	} while (1);
}


//==========================================================================


/*
===================
=
= ShotClipMove
=
= Only checks corners, so the object better be less than one tile wide!
=
===================
*/

boolean ShotClipMove (objtype *ob, long xmove, long ymove)
{
	int			xl,yl,xh,yh,tx,ty,nt1,nt2,x,y;
	long		intersect,basex,basey,pointx,pointy;
	unsigned	inside,total,spot,tile;
	objtype		*check;
	boolean		moveok;

//
// move shot and check to see if any corners are in solid tiles
//
	basex = ob->x;
	basey = ob->y;

	ob->x += xmove;
	ob->y += ymove;

	CalcBounds (ob);

	xl = ob->xl>>TILESHIFT;
	yl = ob->yl>>TILESHIFT;

	xh = ob->xh>>TILESHIFT;
	yh = ob->yh>>TILESHIFT;

	for (y=yl;y<=yh;y++)
		for (x=xl;x<=xh;x++)
		{
//			spot = (*(gamestate->mapsegs[2]+y*mapwidth+x)) >> 8;
			spot = GetMapSegs(2,x,y)>>8;
			if (spot == EXP_WALL_CODE)
				switch (ob->obclass)
				{
					case pshotobj:
					case bigpshotobj:
						ExplodeWall (x,y);
						goto blockmove;
					break;
				}

//			tile = *(gamestate->mapsegs[0]+y*mapwidth+x);
			tile = GetMapSegs(0,x,y);
			if (TILE_FLAGS(tile) & tf_SOLID)
				goto blockmove;
		}
	return false;		// move is OK!


blockmove:

	SPA_PlaySound (SHOOTWALLSND);

	moveok = false;

	do
	{
		xmove /= 2;
		ymove /= 2;
		if (moveok)
		{
			ob->x += xmove;
			ob->y += ymove;
		}
		else
		{
			ob->x -= xmove;
			ob->y -= ymove;
		}
		CalcBounds (ob);
		xl = ob->xl>>TILESHIFT;
		yl = ob->yl>>TILESHIFT;
		xh = ob->xh>>TILESHIFT;
		yh = ob->yh>>TILESHIFT;
		if (GetTileMap(xl,yl) || GetTileMap(xh,yl) || GetTileMap(xh,yh) || GetTileMap(xl,yh))
		{
			moveok = false;
			if (xmove>=-2048 && xmove <=2048 && ymove>=-2048 && ymove <=2048)
			{
				ob->x = basex;
				ob->y = basey;
				return true;
			}
		}
		else
		{
			if (xmove>=-2048 && xmove <=2048 && ymove>=-2048 && ymove <=2048)
				return true;
			moveok = true;
		}
	} while (1);
}



/*
=============================================================================

							PLAYER CONTROL

=============================================================================
*/



void	T_Player (objtype *ob);

statetype s_player = {0,0,&T_Player,&s_player};

/*
===============
=
= SpawnPlayer
=
===============
*/

void SpawnPlayer (int tilex, int tiley, int dir)
{
	player->obclass = playerobj;
	player->active = always;
	player->tilex = tilex;
	player->tiley = tiley;
	player->x = ((long)tilex<<TILESHIFT)+TILEGLOBAL/2;
	player->y = ((long)tiley<<TILESHIFT)+TILEGLOBAL/2;
	player->state = &s_player;
	player->size = renderSetup.MinDist;
	CalcBounds(player);
	player->angle = (1-dir)*90;
	if (player->angle<0)
		player->angle += ANGLES;
}


/*
===================
=
= Thrust
=
===================
*/

void Thrust (int angle, unsigned speed)
{
	long xmove,ymove;

	if (lasttimecount>>5 != ((lasttimecount-tics)>>5) )
	{
	//
	// walk sound
	//
		if (lasttimecount&32)
			SPA_PlaySound (WALK1SND);
		else
			SPA_PlaySound (WALK2SND);
	}

	xmove = FixedByFrac(speed,costable[angle]);
	ymove = -FixedByFrac(speed,sintable[angle]);

	ClipXMove(player,xmove);
	ClipYMove(player,ymove);
	player->tilex = player->x >> TILESHIFT;
	player->tiley = player->y >> TILESHIFT;
}



/*
=======================
=
= ControlMovement
=
=======================
*/
void ControlMovement (objtype *ob)
{
	int	angle=0;
	long	speed=0;
	static int mouseAngleRem=0;
	int mouseAngle;

	int mousexmove, mouseymove;
//	MouseDelta(&mousexmove, &mouseymove);
	mousexmove = control.x;
	mouseymove = control.y;

	mouseAngle = mousexmove+mouseAngleRem;
	mouseAngleRem = mouseAngle%10;
	mouseAngle = mouseAngle/10;

	if (SP_StrafeOn()) {
		//
		// side to side move
		//
		if (SP_StrafeOn()) { // keyboard is used for strafing, mouse changes angle
			ob->angle -= mouseAngle;

			if (ob->angle >= ANGLES)
				ob->angle -= ANGLES;
			if (ob->angle < 0)
				ob->angle += ANGLES;
		} else if (!mousexmove)
			speed = 0;
		else if (mousexmove<0)
			speed = -(long)mousexmove*300;
		else
			speed = -(long)mousexmove*300;

		if (control.xaxis == -1)
		{
			if (control.run)
				speed += RUNSPEED*tics;
			else
				speed += PLAYERSPEED*tics;
		}
		else if (control.xaxis == 1)
		{
			if (control.run)
				speed -= RUNSPEED*tics;
			else
				speed -= PLAYERSPEED*tics;
		}
		if (speed > 0)
		{
			if (speed >= TILEGLOBAL)
				speed = TILEGLOBAL-1;
			angle = ob->angle + ANGLES/4;
			if (angle >= ANGLES)
				angle -= ANGLES;
			Thrust (angle,speed);				// move to left
		}
		else if (speed < 0)
		{
			if (speed <= -TILEGLOBAL)
				speed = -TILEGLOBAL+1;
			angle = ob->angle - ANGLES/4;
			if (angle < 0)
				angle += ANGLES;
			Thrust (angle,-speed);				// move to right
		}

		//
		// forward/backwards move
		//
		speed = 0;
	} else {

		if (control.strafe)
		{
		//
		// strafing
		//
			//
			// side to side move
			//
			if (!mousexmove)
				speed = 0;
			else if (mousexmove<0)
				speed = -(long)mousexmove*300;
			else
				speed = -(long)mousexmove*300;

			if (control.xaxis == -1)
			{
				if (control.run)
					speed += RUNSPEED*tics;
				else
					speed += PLAYERSPEED*tics;
			}
			else if (control.xaxis == 1)
			{
				if (control.run)
					speed -= RUNSPEED*tics;
				else
					speed -= PLAYERSPEED*tics;
			}
			if (speed > 0)
			{
				if (speed >= TILEGLOBAL)
					speed = TILEGLOBAL-1;
				angle = ob->angle + ANGLES/4;
				if (angle >= ANGLES)
					angle -= ANGLES;
				Thrust (angle,speed);				// move to left
			}
			else if (speed < 0)
			{
				if (speed <= -TILEGLOBAL)
					speed = -TILEGLOBAL+1;
				angle = ob->angle - ANGLES/4;
				if (angle < 0)
					angle += ANGLES;
				Thrust (angle,-speed);				// move to right
			}
		}
		else
		{
		//
		// not strafing
		//

			//
			// turning
			//
			if (control.xaxis == 1)
			{
				ob->angle -= tics;
				if (control.run)				// fast turn
					ob->angle -= 2*tics;
			}
			else if (control.xaxis == -1)
			{
				ob->angle+= tics;
				if (control.run)				// fast turn
					ob->angle += 2*tics;
			}

			ob->angle -= mouseAngle;

			if (ob->angle >= ANGLES)
				ob->angle -= ANGLES;
			if (ob->angle < 0)
				ob->angle += ANGLES;

		}

		//
		// forward/backwards move
		//
		if (!mouseymove)
			speed = 0;
		else if (mouseymove<0)
			speed = -(long)mouseymove*500;
		else
			speed = -(long)mouseymove*200;
	}

	if (control.yaxis == -1)
	{
		if (control.run)
			speed += RUNSPEED*tics;
		else
			speed += PLAYERSPEED*tics;
	}
	else if (control.yaxis == 1)
	{
		if (control.run)
			speed -= RUNSPEED*tics;
		else
			speed -= PLAYERSPEED*tics;
	}

	if (speed > 0)
	{
		if (speed >= TILEGLOBAL)
			speed = TILEGLOBAL-1;
		Thrust (ob->angle,speed);			// move forwards
	}
	else if (speed < 0)
	{
		if (speed <= -TILEGLOBAL)
			speed = -TILEGLOBAL+1;
		angle = ob->angle + ANGLES/2;
		if (angle >= ANGLES)
			angle -= ANGLES;
		Thrust (angle,-speed);				// move backwards
	}

}
/*
===============
=
= T_Player
=
===============
*/

void	T_Player (objtype *ob)
{
	extern boolean autofire;

	int	angle,speed,scroll,loop;
	unsigned	text,tilex,tiley;
	long	lspeed;

//	boolean radar_moved=false;


	ControlMovement (ob);


	//
	// firing
	//
	if (boltsleft)
	{
		handheight+=(realtics<<2);
		if (handheight>MAXHANDHEIGHT)
			handheight = MAXHANDHEIGHT;

		ContinueBolt ();
		lasthand = lasttimecount;
	}
	else
	{
		if (control.fire)
		{
			handheight+=(realtics<<2);
			if (handheight>MAXHANDHEIGHT)
				handheight = MAXHANDHEIGHT;
			lasthand = lasttimecount;

			if (!button0down)
				Shoot();

			if (!autofire)
				button0down=true;
		}
		else
		{
			if (lasttimecount > lasthand+HANDPAUSE)
			{
				handheight-=(realtics<<1);
				if (handheight<0)
					handheight = 0;
			}

			button0down = false;
		}
}

	//
	// special actions
	//

	if (control.potion && gamestate->body != MAXBODY)
		DrinkPotion ();

	if (control.bolt && !boltsleft)
		CastBolt ();

	if (control.nuke && ((SP_TimeCount()-lastnuke > NUKETIME) || (autofire)))
		CastNuke ();

	scroll = SPI_GetLastKey()-2;
	if ( scroll>=0 && scroll<NUMSCROLLS && gamestate->scrolls[scroll])
		ReadScroll (scroll);


#if 0
// gems fade out over time...
//
	for (loop=0; loop<5; loop++)
		if (gamestate->gems[loop])
		{
			gamestate->gems[loop] -= realtics;
			if (gamestate->gems[loop] < 0)
			{
				gamestate->gems[loop] = 0;
				redraw_gems = true;
			}
		}
#endif
}


/*==========================================================================

								EXPLOSION SPAWNING ROUTINES

===========================================================================*/

statetype s_explode = {0,1,T_ExpThink,&s_explode};

//-------------------------------------------------------------------------
// SpawnExplosion()
//------------------------------------------------------------------------
void SpawnExplosion(fixed x, fixed y, short Delay)
{
	objtype *new;
	new = DSpawnNewObjFrac(x,y,&s_explode,PIXRADIUS*7);
	if (new != NULL) {
		new->obclass = expobj;
		new->active = always;
		new->temp1 = Delay;
	}
}


//---------------------------------------------------------------------------
// T_ExpThink()
//---------------------------------------------------------------------------
void T_ExpThink(objtype *obj)
{
	if (obj->temp1)
	{
		if ((obj->temp1-=realtics) <= 0)
			obj->temp1 = 0;
	}
	else
	{
		obj->state = &s_pshot_exp1;
		obj->ticcount = obj->state->tictime;
		SPA_PlaySound(BOOMSND);
	}
}



//-------------------------------------------------------------------------
// SpawnBigExplosion()
//------------------------------------------------------------------------
void SpawnBigExplosion(fixed x, fixed y, short Delay, fixed Range)
{
// I suspect that the original random function only returned short ints,
// which is why this code would result in items out of map. The following hack solves this.
	if (Range > 0xFFFF) {
		Range = 0xFFFF;
	}
	SpawnExplosion(x-RANDOM(Range),y+RANDOM(Range),RANDOM(Delay));
	SpawnExplosion(x+RANDOM(Range),y-RANDOM(Range),RANDOM(Delay));
	SpawnExplosion(x-RANDOM(Range),y-RANDOM(Range),RANDOM(Delay));
	SpawnExplosion(x+RANDOM(Range),y+RANDOM(Range),RANDOM(Delay));
}

