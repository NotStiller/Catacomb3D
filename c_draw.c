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

boolean fizzlein;
/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

long 	bytecount,endcount;		// for profiling
int		animframe;
int		*pixelangle;
int		finetangent[FINEANGLES+1];
int		fineviewangle;
unsigned	viewxpix,viewypix;

/*
============================================================================

			   3 - D  DEFINITIONS

============================================================================
*/


tilept	tile,		// tile of wall being followed
	focal,			// focal point in tiles
	right;		// rightmost tile in view

walltype	walls[MAXWALLS],*rightwall;


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


//==========================================================================

void	DrawWall (walltype *wallptr);
void	TraceRay (unsigned angle);
fixed	FixedByFrac (fixed a, fixed b);
fixed	FixedAdd (void);
fixed	TransformX (fixed gx, fixed gy);
int		BackTrace (int finish);
void	ForwardTrace (void);
void	FollowWall (void);

void	BuildTables (void);

void	DrawHand (void);


//==========================================================================

long		wallscalesource;
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

//	wallpicseg = walldirectory[wallpic-FIRSTWALLPIC];
	wallpicseg = grsegs[wallpic]+2*sizeof(uint32_t);
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
		wallheight[x] = height;
		zbuffer[x] = fracheight;


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

		// the above c code is different, so translate the assembly code that was here
		unsigned short int a = distance*slope;
//		unsigned short int b = (a&0xFF00)+((unsigned short)a>>10);
		unsigned short int b = (unsigned short)a>>10;
		unsigned short int c = (b+mapadd)&63;
		unsigned short int d = (63-c)<<6;
//		source = d;
		source = 63-c;

		if (source != lastsource)
		{
			if (lastpix != 0xFFFF)
			{
				wallpointer[lastpix] = (uint8_t*)wallpicseg+lastsource;
				wallwidth[lastpix] = lastwidth;
			}
			lastpix = x;
			lastsource = source;
			lastwidth = 1;
		}
		else
			lastwidth++;			// optimized draw, same map as last one
	}
	wallpointer[lastpix] = (uint8_t*)wallpicseg+lastsource;
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

	//
	// we assume viewx,viewy is not inside a solid tile, so go ahead one step
	//
	do	// until a solid tile is hit
	{
		otx = tile.x;
		oty = tile.y;
		if (!CheckTileCoords(otx,oty)) { return; }
		SetSpotVis(otx,oty,true);
		tracex += tracexstep;
		tracey -= traceystep;
		tile.x = tracex>>TILESHIFT;
		tile.y = tracey>>TILESHIFT;

		if (tile.x!=otx && tile.y!=oty && (SafeTilemap(otx,tile.y) || SafeTilemap(tile.x,oty)) )
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
	} while (!(tilecolor = SafeTilemap(tile.x,tile.y)) );

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

fixed FixedByFrac (fixed a, fixed b)
{
// This function did some serious work once, but on modern hardware it is rather trivial.
	long l = (long)((double)a * (double)b / 65536.0); // 64bit floats can handle 32bit ints easily
	return l;
}

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
=   renderSetup.CenterX
=   TILEGLOBAL		: size of one
=   renderSetup.FocalLength		: distance behind viewx/y for center of projection
=   renderSetup.ProjConst		: conversion from global value to screen value
=
= returns:
=   screenx,screenheight: projected edge location and size
=
========================
*/

void TransformPoint (fixed gx, fixed gy, int *screenx, unsigned *screenheight)
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
  nx = gxt+gyt;

//
// calculate newy
//
  gxt = FixedByFrac(gx,-viewsin);
  gyt = FixedByFrac(gy,viewcos);
  ny = gxt+gyt;

//
// calculate perspective ratio
//
  if (nx<0)
	nx = 0;

  ratio = nx*renderSetup.ProjConst/renderSetup.FocalLength;

  if (ratio<=MINRATIO)
	ratio = MINRATIO;

  *screenx = renderSetup.CenterX + ny/ratio;

  *screenheight = renderSetup.HeightScale/ratio;

}


//
// transform actor
//
boolean TransformActor (objtype *ob)
{
  int ratio;
  fixed gx,gy,gxt,gyt,nx,ny;
  boolean visible;

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
  nx = gxt+gyt-ob->size;

//
// calculate newy
//
  gxt = FixedByFrac(gx,-viewsin);
  gyt = FixedByFrac(gy,viewcos);
  ny = gxt+gyt;

  visible = nx>renderSetup.MinDist;
//
// calculate perspective ratio
//
  if (nx<0)
	nx = 0;

  ratio = nx*renderSetup.ProjConst/renderSetup.FocalLength;

  if (ratio<=MINRATIO)
	ratio = MINRATIO;

  ob->viewx = renderSetup.CenterX + ny/ratio;

  ob->viewheight = renderSetup.HeightScale/ratio;
 
  return visible;
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

  return gxt+gyt;
}

//==========================================================================

/*
==================
=
= BuildTables
=
= Calculates:
=
= renderSetup.ProjConst			projection constant
= sintable/costable	overlapping fractional tables
= firstangle/lastangle	angles from focalpoint to left/right view edges
= prestep		distance from focal point before checking for tiles
=
==================
*/

void BuildTables (void) {
	int 		i;
	long		intang;
	long		x;
	float 	angle,anglestep,radtoint;
	double	tang;
	fixed 	value;

	renderSetup.MinDist = 2*TILEGLOBAL/5;
	renderSetup.FocalLength = TILEGLOBAL;	
	renderSetup.HeightScale = 6*TILEGLOBAL/5; // 320x200 had pixels of dimension 5x6

	//
	// calculate renderSetup.ProjConst value so one tile at mindist allmost fills the view horizontally
	//
	renderSetup.ProjConst = (TILEGLOBAL/renderSetup.Width)*renderSetup.FocalLength/(renderSetup.FocalLength+renderSetup.MinDist);

	//
	// calculate the angle offset from view angle of each pixel's ray
	//
	radtoint = (float)FINEANGLES/2/PI;
	pixelangle = malloc(sizeof(int)*renderSetup.Width);
	for (i=0;i<renderSetup.Width/2;i++)
	{
		// start 1/2 pixel over, so viewangle bisects two middle pixels
		x = (TILEGLOBAL*i+TILEGLOBAL/2)/renderSetup.Width;
		tang = (float)x/(renderSetup.FocalLength+renderSetup.MinDist);
		angle = atan(tang);
		intang = angle*radtoint;
		pixelangle[renderSetup.Width/2-1-i] = intang;
		pixelangle[renderSetup.Width/2+i] = -intang;
	}

	//
	// calculate fine tangents
	// 1 sign bit, 5 units (clipped to), 10 fracs
	//

	for (i=0;i<FINEANGLES/4;i++)
	{
		intang = tan(i/radtoint)*(1l<<10);

		//
		// if the tangent is not reprentable in this many bits, bound the
		// units part ONLY
		//
		if (intang>MAXINT)
			intang = 0x8f00 | (intang & 0xff);
		else if (intang<-MAXINT)
			intang = 0xff00 | (intang & 0xff);

		finetangent[i] = intang;
		//		finetangent[FINEANGLES/2+i] = intang;
		//		finetangent[FINEANGLES/2-i-1] = -intang;
		finetangent[FINEANGLES-i-1] = -intang;
	}

	//
	// costable overlays sintable with a quarter phase shift
	// ANGLES is assumed to be divisable by four
	//

	angle = 0;
	anglestep = PI/2/ANGLEQUAD;
	for (i=0;i<=ANGLEQUAD;i++)
	{
		value=TILEGLOBAL*sin(angle);
		sintable[i] = sintable[i+ANGLES] = sintable[ANGLES/2-i] = value;
		assert(value != 0x80000000);
		sintable[ANGLES-i] = sintable[ANGLES/2+i] = -value;
		angle += anglestep;
	}

	//
	// figure trace angles for first and last pixel on screen
	//
	angle = atan((float)renderSetup.Width/2*renderSetup.ProjConst/renderSetup.FocalLength);
	angle *= ANGLES/(PI*2);

	intang = (int)angle+1;
	firstangle = intang;
	lastangle = -intang;

	prestep = TILEGLOBAL*((float)renderSetup.FocalLength/costable[firstangle]);

	//
	// misc stuff
	//
	walls[0].x2 = -1;
	walls[0].height2 = 32000;
}


//==========================================================================


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

	for (i=0;i < renderSetup.Width;i++) {
		wallwidth[i] = 0;
	}
	SPG_DrawFloors (8, 0);

	rightwall->x1 = renderSetup.Width;
	rightwall->height1 = 32000;
	(rightwall+1)->x1 = 32000;

	leftx = -1;

	for (wall=&walls[1];wall<rightwall && leftx<renderSetup.Width ;wall++)
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

	  if (rightclip>=renderSetup.Width)
		rightclip=renderSetup.Width-1;

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
		if (!CheckTileCoords(obj->tilex,obj->tiley)) { return; }
		//
		// could be in any of the nine surrounding tiles
		//
		int visible=GetSpotVis(obj->tilex,obj->tiley);
		int x,y;
		for (x = -1; x <= 1 && !visible; x++) {
			for (y = -1; y <= 1 && !visible; y++) {
				if (GetSpotVis(obj->tilex+x,obj->tiley+y) && !GetTileMap(obj->tilex+x,obj->tiley+y)) {
					visible = true;
					break;
				}
			}
		}
		if (visible)
		{
			if ((obj->active == noalways) || (obj->active == always))
				obj->active = always;
			else
				obj->active = yes;
			if (!TransformActor (obj))
				continue;
			if (!obj->state->shapenum)
				continue;
			if (!obj->viewheight)
				continue;			// too far away

			*vislist++ = obj;
			numvisable++;
		}
		else if (EnableC4Features) {
			if ((obj->active != always) && (obj->active != noalways))
				obj->active = no;
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
		SPG_DrawScaleShape(farthest->viewx,farthest->viewheight, grsegs[farthest->state->shapenum], 5);
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
	long	newtime;
//
// calculate tics since last refresh for adaptive timing
//
	if (lasttimecount > SP_TimeCount()) {
		printf("lasttimecount > TimeCount !\n");
		lasttimecount = SP_TimeCount();	// if the game was paused a LONG time
	}

	newtime = SP_TimeCount();
	realtics = tics = newtime-lasttimecount;
	lasttimecount = newtime;

	if (tics>MAXTICS)
	{
// the sourceport needs not mirror behaviour on too slow computers, also did the following ever make sense ?
//		TimeCount -= tics-MAXTICS;
// It would be invalidated at the next CalcTics anyway.
		tics = MAXTICS;
	}

	if (realtics>MAXREALTICS) {
		realtics = MAXREALTICS;
	}
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
	restarttrace = false;
	reallyabsolutelypositivelyaborttrace = false;

//
// clear out the traced array
//
	ClearSpotVis();

//
// set up variables for this view
//

	viewangle = player->angle;
	fineviewangle = viewangle*(FINEANGLES/ANGLES);
	viewsin = -sintable[viewangle];
	viewcos = costable[viewangle];
	viewx = player->x - FixedByFrac(renderSetup.FocalLength,viewcos);
	viewy = player->y - FixedByFrac(renderSetup.FocalLength,viewsin);
	viewx &= 0xfffffc00;		// stop on a pixel boundary
	viewy &= 0xfffffc00;
	viewx += 0x180;
	viewy += 0x180;
	if (0) {// there be an error here !
		viewangle = 197;
		fineviewangle = viewangle*(FINEANGLES/ANGLES);
		viewsin = -sintable[viewangle];
		viewcos = costable[viewangle];
		viewx = 886144;	viewy = 104832; 
	}
	viewxpix = viewx>>10;
	viewypix = viewy>>10;

	focal.x = viewx>>TILESHIFT;
	focal.y = viewy>>TILESHIFT;

	if (0)
	{
		globpt viewpos, viewdir;
		viewpos.x = viewx;
		viewpos.y = viewy;
		viewdir.x = viewcos;
		viewdir.y = viewsin;
	void FindLeftmostWall(globpt ViewPos, globpt ViewDir);
		FindLeftmostWall(viewpos, viewdir);
	}

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

//
// find the leftmost visable tile in view
//
	tracedir = viewangle + firstangle;
	if (tracedir<0)
	  tracedir+=ANGLES;
	else if (tracedir>=ANGLES)
	  tracedir-=ANGLES;
	TraceRay( tracedir );

//
// follow the walls from there to the right
//
	rightwall = &walls[1];
	FollowWalls ();

	if (restarttrace && !reallyabsolutelypositivelyaborttrace) {
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
//		FizzleFade(renderSetup.Width,renderSetup.Height,true);
		FizzleFade();
		lasttimecount = SP_TimeCount();
		MouseDelta(NULL, NULL);	// Clear accumulated mouse movement
	}

	FlipBuffer();
	CalcTics ();


}


