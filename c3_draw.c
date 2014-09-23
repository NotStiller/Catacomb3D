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

// C3_DRAW.C

#include "c3_def.h"
#pragma hdrstop

unsigned	highest;
unsigned	mostwalls,numwalls;

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

#define PI	3.141592657
#define ANGLEQUAD	(ANGLES/4)

unsigned	oldend;

#define FINEANGLES	3600

#define MINRATIO	16


const	unsigned	MAXSCALEHEIGHT	= (VIEWWIDTH/2);
const	unsigned	MAXVISHEIGHT	= (VIEWHEIGHT/2);
const	unsigned	BASESCALE		= 32;

/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

//
// calculate location of screens in video memory so they have the
// maximum possible distance seperating them (for scaling overflow)
//

unsigned screenloc[3]= {0x900,0x2000,0x3700};
unsigned freelatch = 0x4e00;

boolean		fizzlein;

long	scaleshapecalll;
long	scaletablecall;

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

long 	bytecount,endcount;		// for profiling
int		animframe;
int		pixelangle[VIEWWIDTH];
int		finetangent[FINEANGLES+1];
int		fineviewangle;
unsigned	viewxpix,viewypix;

/*
============================================================================

			   3 - D  DEFINITIONS

============================================================================
*/

fixed	tileglobal	= TILEGLOBAL;
fixed	focallength	= FOCALLENGTH;
fixed	mindist		= MINDIST;
int		viewheight	= VIEWHEIGHT;
fixed scale;


tilept	tile,lasttile,		// tile of wall being followed
	focal,			// focal point in tiles
	left,mid,right;		// rightmost tile in view

globpt edge,view;

int	segstart[VIEWHEIGHT],	// addline tracks line segment and draws
	segend[VIEWHEIGHT],
	segcolor[VIEWHEIGHT];	// only when the color changes


walltype	walls[MAXWALLS],*leftwall,*rightwall;


//==========================================================================

//
// refresh stuff
//

long lasttimecount;

//
// rendering stuff
//

int firstangle,lastangle;

fixed prestep;

fixed sintable[ANGLES+ANGLES/4],*costable = sintable+(ANGLES/4);

fixed	viewx,viewy;			// the focal point
int	viewangle;
fixed	viewsin,viewcos;

int	zbuffer[VIEWXH+1];	// holds the height of the wall at that point

//==========================================================================

void	DrawWall (walltype *wallptr);
void	TraceRay (unsigned angle);
fixed	FixedByFrac (fixed a, fixed b);
fixed	FixedAdd (void);
fixed	TransformX (fixed gx, fixed gy);
int		BackTrace (int finish);
void	ForwardTrace (void);
int		TurnClockwise (void);
int		TurnCounterClockwise (void);
void	FollowWall (void);

void	NewScene (void);
void	BuildTables (void);

//==========================================================================



long		wallscalesource;

int	walllight1[NUMFLOORS] = {0,
	WALL1LPIC,WALL2LPIC,WALL3LPIC,WALL4LPIC,WALL5LPIC,WALL6LPIC,WALL7LPIC,
	WALL1LPIC,WALL2LPIC,WALL3LPIC,WALL4LPIC,WALL5LPIC,WALL6LPIC,WALL7LPIC,
	EXPWALL1PIC,EXPWALL2PIC,EXPWALL3PIC,
	RDOOR1PIC,RDOOR2PIC,RDOOR1PIC,RDOOR2PIC,
	YDOOR1PIC,YDOOR2PIC,YDOOR1PIC,YDOOR2PIC,
	GDOOR1PIC,GDOOR2PIC,GDOOR1PIC,GDOOR2PIC,
	BDOOR1PIC,BDOOR2PIC,BDOOR1PIC,BDOOR2PIC};

int	walldark1[NUMFLOORS] = {0,
	WALL1DPIC,WALL2DPIC,WALL3DPIC,WALL4DPIC,WALL5DPIC,WALL6DPIC,WALL7DPIC,
	WALL1DPIC,WALL2DPIC,WALL3DPIC,WALL4DPIC,WALL5DPIC,WALL6DPIC,WALL7DPIC,
	EXPWALL1PIC,EXPWALL2PIC,EXPWALL3PIC,
	RDOOR1PIC,RDOOR2PIC,RDOOR1PIC,RDOOR2PIC,
	YDOOR1PIC,YDOOR2PIC,YDOOR1PIC,YDOOR2PIC,
	GDOOR1PIC,GDOOR2PIC,GDOOR1PIC,GDOOR2PIC,
	BDOOR1PIC,BDOOR2PIC,BDOOR1PIC,BDOOR2PIC};

int	walllight2[NUMFLOORS] = {0,
	WALL1LPIC,WALL2LPIC,WALL3LPIC,WALL4LPIC,WALL5LPIC,WALL6LPIC,WALL7LPIC,
	WALL1LPIC,WALL2LPIC,WALL3LPIC,WALL4LPIC,WALL5LPIC,WALL6LPIC,WALL7LPIC,
	EXPWALL1PIC,EXPWALL2PIC,EXPWALL3PIC,
	RDOOR2PIC,RDOOR1PIC,RDOOR2PIC,RDOOR1PIC,
	YDOOR2PIC,YDOOR1PIC,YDOOR2PIC,YDOOR1PIC,
	GDOOR2PIC,GDOOR1PIC,GDOOR2PIC,GDOOR1PIC,
	BDOOR2PIC,BDOOR1PIC,BDOOR2PIC,BDOOR1PIC};

int	walldark2[NUMFLOORS] = {0,
	WALL1DPIC,WALL2DPIC,WALL3DPIC,WALL4DPIC,WALL5DPIC,WALL6DPIC,WALL7DPIC,
	WALL1DPIC,WALL2DPIC,WALL3DPIC,WALL4DPIC,WALL5DPIC,WALL6DPIC,WALL7DPIC,
	EXPWALL1PIC,EXPWALL2PIC,EXPWALL3PIC,
	RDOOR2PIC,RDOOR1PIC,RDOOR2PIC,RDOOR1PIC,
	YDOOR2PIC,YDOOR1PIC,YDOOR2PIC,YDOOR1PIC,
	GDOOR2PIC,GDOOR1PIC,GDOOR2PIC,GDOOR1PIC,
	BDOOR2PIC,BDOOR1PIC,BDOOR2PIC,BDOOR1PIC};

/*
=====================
=
= DrawVWall
=
= Draws a wall by vertical segments, for texture mapping!
=
= wallptr->side is true for east/west walls (constant x)
=
= fracheight and fracstep are 16.16 bit fractions
=
=====================
*/

void DrawVWall (walltype *wallptr)
{
	int	 		x,i;
	unsigned	source;
	unsigned	width,sourceint;
	unsigned	wallpic;
	memptr 		wallpicseg;
	unsigned	skip;
	long		fracheight,fracstep,longheightchange;
	unsigned	height;
	int			heightchange;
	unsigned	slope,distance;
	int			traceangle,angle;
	int			mapadd;
	unsigned short	lastpix,lastsource,lastwidth;


	if (wallptr->rightclip < wallptr->leftclip)
		Quit ("DrawVWall: Right < Left");

//
// setup for height calculation
//
	wallptr->height1 >>= 1;
	wallptr->height2 >>= 1;
	wallptr->planecoord>>=10;			// remove non significant bits

	width = wallptr->x2 - wallptr->x1;
	if (width)
	{
		heightchange = wallptr->height2 - wallptr->height1;
		longheightchange = heightchange<<16;
		if ((longheightchange < 0 && heightchange > 0) || (longheightchange > 0 && heightchange < 0)) {
			assert(0);
		}
		assert(heightchange == 0 || longheightchange != 0);
		assert(heightchange != 0 || longheightchange == 0);
		fracstep = longheightchange/(long)width;
	}

	fracheight = ((long)wallptr->height1<<16)+0x8000;
	skip = wallptr->leftclip - wallptr->x1;
	if (skip)
		fracheight += fracstep*skip;

//
// setup for texture mapping
//
// mapadd is 64*64 (to keep source positive) + the origin wall intercept
// distance has 6 unit bits, and 6 frac bits
// traceangle is the center view angle in FINEANGLES, moved to be in
// the +-90 degree range (to thew right of origin)
//
	traceangle = fineviewangle;
	//
	// find wall picture to map from
	//
	if (wallptr->side)
	{	// east or west wall
		if (animframe)
			wallpic = walllight2[wallptr->color];
		else
			wallpic = walllight1[wallptr->color];

		if (wallptr->planecoord < viewxpix)
		{
			distance = viewxpix-wallptr->planecoord;
			traceangle -= FINEANGLES/2;
			mapadd = (64-viewypix&63);		// the pixel spot of the origin
		}
		else
		{
			distance = wallptr->planecoord-viewxpix;
			// traceangle is correct
			mapadd = viewypix&63;		// the pixel spot of the origin
		}
	}
	else
	{	// north or south wall
		if (animframe)
			wallpic = walldark2[wallptr->color];
		else
			wallpic = walldark1[wallptr->color];

		if (wallptr->planecoord < viewypix)
		{
			distance = viewypix-wallptr->planecoord;
			traceangle -= FINEANGLES/4;
			mapadd = viewxpix&63;		// the pixel spot of the origin
		}
		else
		{
			distance = wallptr->planecoord-viewypix;
			traceangle -= FINEANGLES*3/4;
			mapadd = (64-viewxpix&63);		// the pixel spot of the origin
		}
	}

	mapadd = 64*64-mapadd;				// make sure it stays positive

	wallpicseg = walldirectory[wallpic-FIRSTWALLPIC];
	if (traceangle > FINEANGLES/2)
		traceangle -= FINEANGLES;

//
// calculate everything
//
// IMPORTANT!  This loop is executed around 5000 times / second!
//
	lastpix = lastsource = 0xFFFF;

	for (x = wallptr->leftclip ; x <= wallptr->rightclip ; x++)
	{
		//
		// height
		//
		height = fracheight>>16;
		fracheight += fracstep;
		assert(height >= 0);
//		assert(fracheight >= 0);
		if (height > MAXSCALEHEIGHT)
			height = MAXSCALEHEIGHT;
		wallheight[x] = zbuffer[x] = height;


		//
		// texture map
		//
		angle = pixelangle[x]+traceangle;
		if (angle<0)
			angle+=FINEANGLES;

		slope = finetangent[angle];

//
// distance is an unsigned 6.6 bit number (12 pixel bits)
// slope is a signed 5.10 bit number
// result is a signed 11.16 bit number
//

		// the above c code is different, so translate this assembly code here
		unsigned short int a = distance*slope;
		unsigned short int b = (a&0xFF00)+((unsigned short)a>>10);
		unsigned short int c = (b+mapadd)&63;
		unsigned short int d = (63-c)<<6;
		source = d;

		if (source != lastsource)
		{
			if (lastpix != 0xFFFF)
			{
				wallseg[lastpix] = wallpicseg;
				wallofs[lastpix] = lastsource;
				wallwidth[lastpix] = lastwidth;
			}
			lastpix = x;
			lastsource = source;
			lastwidth = 1;
		}
		else
			lastwidth++;			// optimized draw, same map as last one
	}
	wallseg[lastpix] = wallpicseg;
	wallofs[lastpix] = lastsource;
	wallwidth[lastpix] = lastwidth;
}


//==========================================================================


/*
=================
=
= TraceRay
=
= Used to find the left and rightmost tile in the view area to be traced from
= Follows a ray of the given angle from viewx,viewy in the global map until
= it hits a solid tile
= sets:
=   tile.x,tile.y	: tile coordinates of contacted tile
=   tilecolor	: solid tile's color
=
==================
*/

int tilecolor;

void TraceRay (unsigned angle)
{
	long tracex,tracey,tracexstep,traceystep,searchx,searchy;
	fixed fixtemp;
	int otx,oty,searchsteps;

	tracexstep = costable[angle];
	traceystep = sintable[angle];

	//
	// advance point so it is even with the view plane before we start checking
	//
	fixtemp = FixedByFrac(prestep,tracexstep);
	tracex = viewx+fixtemp;
	fixtemp = FixedByFrac(prestep,traceystep);
	tracey = viewy-fixtemp;

	tile.x = tracex>>TILESHIFT;	// starting point in tiles
	tile.y = tracey>>TILESHIFT;

	if (tracexstep<0)			// use 2's complement, not signed magnitude
		tracexstep = -(tracexstep&0x7fffffff);

	if (traceystep<0)			// use 2's complement, not signed magnitude
		traceystep = -(traceystep&0x7fffffff);

	//
	// we assume viewx,viewy is not inside a solid tile, so go ahead one step
	//
	do	// until a solid tile is hit
	{
		otx = tile.x;
		oty = tile.y;
		spotvis[otx][oty] = true;
		tracex += tracexstep;
		tracey -= traceystep;
		tile.x = tracex>>TILESHIFT;
		tile.y = tracey>>TILESHIFT;

		if (tile.x!=otx && tile.y!=oty && (tilemap[otx][tile.y] || tilemap[tile.x][oty]) )
		{
			//
			// trace crossed two solid tiles, so do a binary search along the line
			// to find a spot where only one tile edge is crossed
			//
			searchsteps = 0;
			searchx = tracexstep;
			searchy = traceystep;
			do
			{
				searchx/=2;
				searchy/=2;
				if (tile.x!=otx && tile.y!=oty)
				{
					// still too far
					tracex -= searchx;
					tracey += searchy;
				}
				else
				{
					// not far enough, no tiles crossed
					tracex += searchx;
					tracey -= searchy;
				}

				//
				// if it is REAL close, go for the most clockwise intersection
				//
				if (++searchsteps == 16)
				{
					tracex = (long)otx<<TILESHIFT;
					tracey = (long)oty<<TILESHIFT;
					if (tracexstep>0)
					{
						if (traceystep<0)
						{
							tracex += TILEGLOBAL-1;
							tracey += TILEGLOBAL;
						}
						else
						{
							tracex += TILEGLOBAL;
						}
					}
					else
					{
						if (traceystep<0)
						{
							tracex --;
							tracey += TILEGLOBAL-1;
						}
						else
						{
							tracey --;
						}
					}
				}

				tile.x = tracex>>TILESHIFT;
				tile.y = tracey>>TILESHIFT;

			} while (( tile.x!=otx && tile.y!=oty) || (tile.x==otx && tile.y==oty) );
		}
	} while (!(tilecolor = tilemap[tile.x][tile.y]) );

}

//==========================================================================


/*
========================
=
= FixedByFrac
=
= multiply a 16/16 bit, 2's complement fixed point number by a 16 bit
= fraction, passed as a signed magnitude 32 bit number
=
========================
*/

//#pragma warn -rvl			// I stick the return value in with ASMs

fixed FixedByFrac (fixed a, fixed b)
{
	int sign=1;
	if (a < 0) {
		a = -a;
		sign=-sign;
	}
	if (b < 0) {
		b = b^0x80000000;
		assert(b < 65536);
		sign=-sign;
	}
	long l = sign*(long)((double)a * (double)b / 65536.0);
	return l;
}

//#pragma warn +rvl

//==========================================================================


/*
========================
=
= TransformPoint
=
= Takes paramaters:
=   gx,gy		: globalx/globaly of point
=
= globals:
=   viewx,viewy		: point of view
=   viewcos,viewsin	: sin/cos of viewangle
=
=
= defines:
=   CENTERX		: pixel location of center of view window
=   TILEGLOBAL		: size of one
=   FOCALLENGTH		: distance behind viewx/y for center of projection
=   scale		: conversion from global value to screen value
=
= returns:
=   screenx,screenheight: projected edge location and size
=
========================
*/

void TransformPoint (fixed gx, fixed gy, short int *screenx, unsigned short *screenheight)
{
  int ratio;
  fixed gxt,gyt,nx,ny;

//
// translate point to view centered coordinates
//
  gx = gx-viewx;
  gy = gy-viewy;

//
// calculate newx
//
  gxt = FixedByFrac(gx,viewcos);
  gyt = FixedByFrac(gy,viewsin);
  nx = gxt-gyt;

//
// calculate newy
//
  gxt = FixedByFrac(gx,viewsin);
  gyt = FixedByFrac(gy,viewcos);
  ny = gyt+gxt;

//
// calculate perspective ratio
//
  if (nx<0)
	nx = 0;

  ratio = nx*scale/FOCALLENGTH;

  if (ratio<=MINRATIO)
	ratio = MINRATIO;

  *screenx = CENTERX + ny/ratio;

  *screenheight = TILEGLOBAL/ratio;

}


//
// transform actor
//
void TransformActor (objtype *ob)
{
  int ratio;
  fixed gx,gy,gxt,gyt,nx,ny;

//
// translate point to view centered coordinates
//
  gx = ob->x-viewx;
  gy = ob->y-viewy;

//
// calculate newx
//
  gxt = FixedByFrac(gx,viewcos);
  gyt = FixedByFrac(gy,viewsin);
  nx = gxt-gyt-ob->size;

//
// calculate newy
//
  gxt = FixedByFrac(gx,viewsin);
  gyt = FixedByFrac(gy,viewcos);
  ny = gyt+gxt;

//
// calculate perspective ratio
//
  if (nx<0)
	nx = 0;

  ratio = nx*scale/FOCALLENGTH;

  if (ratio<=MINRATIO)
	ratio = MINRATIO;

  ob->viewx = CENTERX + ny/ratio;

  ob->viewheight = TILEGLOBAL/ratio;
}

//==========================================================================

fixed TransformX (fixed gx, fixed gy)
{
  int ratio;
  fixed gxt,gyt,nx,ny;

//
// translate point to view centered coordinates
//
  gx = gx-viewx;
  gy = gy-viewy;

//
// calculate newx
//
  gxt = FixedByFrac(gx,viewcos);
  gyt = FixedByFrac(gy,viewsin);

  return gxt-gyt;
}

//==========================================================================

/*
==================
=
= BuildTables
=
= Calculates:
=
= scale			projection constant
= sintable/costable	overlapping fractional tables
= firstangle/lastangle	angles from focalpoint to left/right view edges
= prestep		distance from focal point before checking for tiles
=
==================
*/

void BuildTables (void)
{
  int 		i;
  long		intang;
  long		x;
  float 	angle,anglestep,radtoint;
  double	tang;
  fixed 	value;

//
// calculate the angle offset from view angle of each pixel's ray
//
	radtoint = (float)FINEANGLES/2/PI;
	for (i=0;i<VIEWWIDTH/2;i++)
	{
	// start 1/2 pixel over, so viewangle bisects two middle pixels
		x = (TILEGLOBAL*i+TILEGLOBAL/2)/VIEWWIDTH;
		tang = (float)x/(FOCALLENGTH+MINDIST);
		angle = atan(tang);
		intang = angle*radtoint;
		pixelangle[VIEWWIDTH/2-1-i] = intang;
		pixelangle[VIEWWIDTH/2+i] = -intang;
	}

//
// calculate fine tangents
// 1 sign bit, 5 units (clipped to), 10 fracs
//
#define MYMININT	(-MAXINT)

	for (i=0;i<FINEANGLES/4;i++)
	{
		intang = tan(i/radtoint)*(1l<<10);

		//
		// if the tangent is not reprentable in this many bits, bound the
		// units part ONLY
		//
		if (intang>MAXINT)
			intang = 0x8f00 | (intang & 0xff);
		else if (intang<MYMININT)
			intang = 0xff00 | (intang & 0xff);

		finetangent[i] = intang;
//		finetangent[FINEANGLES/2+i] = intang;
//		finetangent[FINEANGLES/2-i-1] = -intang;
		finetangent[FINEANGLES-i-1] = -intang;
	}

//
// calculate scale value so one tile at mindist allmost fills the view horizontally
//
  scale = GLOBAL1/VIEWWIDTH;
  scale *= focallength;
  scale /= (focallength+mindist);

//
// costable overlays sintable with a quarter phase shift
// ANGLES is assumed to be divisable by four
//
// The low word of the value is the fraction, the high bit is the sign bit,
// bits 16-30 should be 0
//

  angle = 0;
  anglestep = PI/2/ANGLEQUAD;
  for (i=0;i<=ANGLEQUAD;i++)
  {
	value=GLOBAL1*sin(angle);
	sintable[i] = sintable[i+ANGLES] = sintable[ANGLES/2-i] = value;
	sintable[ANGLES-i] = sintable[ANGLES/2+i] = value | 0x80000000l;
	angle += anglestep;
  }

//
// figure trace angles for first and last pixel on screen
//
  angle = atan((float)VIEWWIDTH/2*scale/FOCALLENGTH);
  angle *= ANGLES/(PI*2);

  intang = (int)angle+1;
  firstangle = intang;
  lastangle = -intang;

  prestep = GLOBAL1*((float)FOCALLENGTH/costable[firstangle]);

//
// misc stuff
//
  walls[0].x2 = VIEWX-1;
  walls[0].height2 = 32000;
}


//==========================================================================

/*
=====================
=
= ClearScreen
=
=====================
*/

void ClearScreen (void)
{
	VW_Bar(0,0,VIEWWIDTH, CENTERY+1, 0);
	VW_Bar(0,CENTERY+1,VIEWWIDTH, CENTERY+1, 8);
}

//==========================================================================

/*
=====================
=
= DrawWallList
=
= Clips and draws all the walls traced this refresh
=
=====================
*/

void DrawWallList (void)
{
	int i,leftx,newleft,rightclip;
	walltype *wall, *check;

	for (i=0;i < VIEWWIDTH;i++) {
		wallwidth[i] = 0;
	}
	ClearScreen ();

	rightwall->x1 = VIEWXH+1;
	rightwall->height1 = 32000;
	(rightwall+1)->x1 = 32000;

	leftx = -1;

	for (wall=&walls[1];wall<rightwall && leftx<=VIEWXH ;wall++)
	{	
	  if (leftx >= wall->x2)
		continue;

	  rightclip = wall->x2;

	  check = wall+1;
	  while (check->x1 <= rightclip && check->height1 >= wall->height2)
	  {
		rightclip = check->x1-1;
		check++;
	  }

	  if (rightclip>VIEWXH)
		rightclip=VIEWXH;

	  if (leftx < wall->x1 - 1)
		newleft = wall->x1-1;		// there was black space between walls
	  else
		newleft = leftx;

	  if (rightclip > newleft)
	  {
		wall->leftclip = newleft+1;
		wall->rightclip = rightclip;
		DrawVWall (wall);
		leftx = rightclip;
	  }
	}

	ScaleWalls ();					// draw all the walls
}

//==========================================================================

/*
=====================
=
= DrawScaleds
=
= Draws all objects that are visable
=
=====================
*/

objtype *depthsort[MAXACTORS];

void DrawScaleds (void)
{

	int 		i,j,least,numvisable,height;
	objtype 	*obj,**vislist,*farthest;
	memptr		shape;
	byte		*tilespot,*visspot;

	numvisable = 0;

//
// calculate base positions of all objects
//
	vislist = &depthsort[0];

	for (obj = player->next;obj;obj=obj->next)
	{
		if (!obj->state->shapenum)
			continue;

		tilespot = &tilemap[0][0]+(obj->tilex<<6)+obj->tiley;
		visspot = &spotvis[0][0]+(obj->tilex<<6)+obj->tiley;
		//
		// could be in any of the nine surrounding tiles
		//
		if (*visspot
		|| ( *(visspot-1) && !*(tilespot-1) )
		|| ( *(visspot+1) && !*(tilespot+1) )
		|| ( *(visspot-65) && !*(tilespot-65) )
		|| ( *(visspot-64) && !*(tilespot-64) )
		|| ( *(visspot-63) && !*(tilespot-63) )
		|| ( *(visspot+65) && !*(tilespot+65) )
		|| ( *(visspot+64) && !*(tilespot+64) )
		|| ( *(visspot+63) && !*(tilespot+63) ) )
		{
			obj->active = true;
			TransformActor (obj);
			if (!obj->viewheight || obj->viewheight > VIEWWIDTH)
				continue;			// too close or far away

			*vislist++ = obj;
			numvisable++;
		}
	}

	if (vislist == &depthsort[0])
		return;						// no visable objects

//
// draw from back to front
//
	for (i = 0; i<numvisable; i++)
	{
		least = 32000;
		for (j=0;j<numvisable;j++)
		{
			height = depthsort[j]->viewheight;
			if (height < least)
			{
				least = height;
				farthest = depthsort[j];
			}
		}
		//
		// draw farthest
		//
		shape = shapedirectory[farthest->state->shapenum-FIRSTSCALEPIC];
		ScaleShape(farthest->viewx,shape,farthest->viewheight);
		farthest->viewheight = 32000;
	}
}

//==========================================================================


/*
=====================
=
= CalcTics
=
=====================
*/

void CalcTics (void)
{
	long	newtime,oldtimecount;


//
// calculate tics since last refresh for adaptive timing
//
	if (lasttimecount > SP_TimeCount())
		SP_SetTimeCount(lasttimecount);		// if the game was paused a LONG time

	if (DemoMode)					// demo recording and playback needs
	{								// to be constant
//
// take DEMOTICS or more tics, and modify Timecount to reflect time taken
//
		oldtimecount = lasttimecount;
		while (SP_TimeCount()<oldtimecount+DEMOTICS*2)
		;
		lasttimecount = oldtimecount + DEMOTICS;
		SP_SetTimeCount(lasttimecount+DEMOTICS);
		tics = DEMOTICS;
	}
	else
	{
//
// non demo, so report actual time
//
		newtime = SP_TimeCount();
		tics = newtime-lasttimecount;
		lasttimecount = newtime;

		if (tics>MAXTICS)
		{
			SP_SetTimeCount(tics-MAXTICS);
			tics = MAXTICS;
		}
	}
}


//==========================================================================


/*
========================
=
= DrawHand
=
========================
*/

void	DrawHand (void)
{
	int	picnum;

	picnum = HAND1PICM;
	if (gamestate.shotpower || boltsleft)
		picnum += (((unsigned)SP_TimeCount()>>3)&1);

	SPG_DrawMaskedPicSkip(grsegs[picnum], 12*8, VIEWHEIGHT-handheight, 0, handheight);
}

//==========================================================================


/*
========================
=
= ThreeDRefresh
=
========================
*/

void	ThreeDRefresh (void)
{
	int tracedir;

restart:
	aborttrace = false;

//
// clear out the traced array
//
	memset(spotvis, 0, mapwidth*32*2);

//
// set up variables for this view
//

	viewangle = player->angle;
	fineviewangle = viewangle*(FINEANGLES/ANGLES);
	viewsin = sintable[viewangle];
	viewcos = costable[viewangle];
	viewx = player->x - FixedByFrac(FOCALLENGTH,viewcos);
	viewy = player->y + FixedByFrac(FOCALLENGTH,viewsin);
	viewx &= 0xfffffc00;		// stop on a pixel boundary
	viewy &= 0xfffffc00;
	viewx += 0x180;
	viewy += 0x180;
//viewx = 886144;	viewy = 104832; // there be an error here !
	viewxpix = viewx>>10;
	viewypix = viewy>>10;

	focal.x = viewx>>TILESHIFT;
	focal.y = viewy>>TILESHIFT;

//
// find the rightmost visable tile in view
//
	tracedir = viewangle + lastangle;
	if (tracedir<0)
	  tracedir+=ANGLES;
	else if (tracedir>=ANGLES)
	  tracedir-=ANGLES;
	TraceRay( tracedir );
	right.x = tile.x;
	right.y = tile.y;
	assert(tile.x < 64);

//
// find the leftmost visable tile in view
//
	tracedir = viewangle + firstangle;
	if (tracedir<0)
	  tracedir+=ANGLES;
	else if (tracedir>=ANGLES)
	  tracedir-=ANGLES;
	TraceRay( tracedir );
	assert(tile.x < 64);

//
// follow the walls from there to the right
//
	rightwall = &walls[1];
	FollowWalls ();

	if (aborttrace) {
		goto restart;
	}

//
// actually draw stuff
//

//
// draw the wall list saved be FollowWalls ()
//
	animframe = (SP_TimeCount()&8)>>3;

//
// draw all the scaled images
//

	DrawWallList();
	DrawScaleds();

//
// draw hand
//
	if (handheight)
		DrawHand ();

//
// show screen and time last cycle
//
	if (fizzlein)
	{
		fizzlein = false;
		FizzleFade(VIEWWIDTH,VIEWHEIGHT,true);
		lasttimecount = SP_TimeCount();
		MouseDelta(NULL, NULL);	// Clear accumulated mouse movement
	}

	SPG_FlipBuffer();
	CalcTics ();


}

