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

// ID_GLOB.H

#ifndef ID_HEADS_H
#define ID_HEADS_H

#include "srcport.h"

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define __ID_GLOB__

//--------------------------------------------------------------------------

extern const char *GamespecificExtension; // C3D, ABS etc.
extern int EnableC4Features;

//#define	EXTENSION	"C3D"
//#define	EXTENSION	"ABS"
//#define	EXT	"ABS"

//#define CAT3D

#define	EGAGR	2
#define	VGAGR	3

#define  EGA320GR	10					// MDM (GAMERS EDGE)
#define  EGA640GR	11					// MDM (GAMERS EDGE)

#define MAPSIZE		64		// maps are 64*64 max
#define MAXACTORS	150		// max number of tanks, etc / map

#define NORTH	0
#define EAST	1
#define SOUTH	2
#define WEST	3

#define SIGN(x) ((x)>0?1:-1)
#define ABS(x) ((int)(x)>0?(x):-(x))
#define LABS(x) ((long)(x)>0?(x):-(x))


#define TILEGLOBAL	(1l<<16)
#define TILESHIFT	16l
#define PIXRADIUS		512



#define	__TYPES__

// typedef	enum	{false,true}	boolean;
#define false 0
#define true 1
typedef	uint8_t	boolean;
typedef	uint8_t	byte;
typedef	uint16_t			word;
typedef	uint32_t		longword;
typedef	byte *					Ptr;

typedef	struct { int x,y; } Point;
typedef	struct { Point ul,lr; } Rect;

typedef long fixed;

typedef struct { int x,y; } tilept;
typedef struct { fixed x,y; } globpt;

typedef struct {
	int x1,x2,leftclip,rightclip;// first pixel of wall (may not be visable)
	unsigned height1,height2,color,walllength,side;
	long planecoord;
} walltype;

enum {north,east,south,west,northeast,southeast,southwest,northwest,nodir};		

typedef int dirtype;

typedef struct	statestruct
{
	int		shapenum;
	int		tictime;
	void	(*think) ();
	struct	statestruct	*next;
} statetype;

typedef struct objstruct
{
  enum {no,noalways,yes,always}	active;
  short int		ticcount;
  int	obclass;
  statetype	*state;

  unsigned char flags;
  boolean	shootable;
  boolean	tileobject;		// true if entirely inside one tile

  long		distance;
  dirtype	dir;
  fixed 	x,y;
  unsigned short	tilex,tiley;
  short int	 viewx;
  unsigned short	viewheight;

  short int 		angle;
  short int		hitpoints;
  long		speed;

  long	size;			// global radius for hit rect calculation
  fixed		xl,xh,yl,yh;	// hit rectangle

  short int		temp1,temp2;
  struct	objstruct	*next,*prev;
} objtype;

#include "id_vw.h"
#include "id_in.h"
#include "id_sd.h"
#include "id_us.h"


typedef struct {
	uint8_t *BufferStart;
	int Width, Height, Pitch;
	int CenterX, CenterY;

	fixed MinDist;
	fixed FocalLength;
	fixed HeightScale; // used to correct aspect ratio for heights
	fixed ProjConst;
} RenderSetup3D;

extern RenderSetup3D renderSetup;

void GameWindowResizeHook(int NewWidth, int NewHeight);


void	Quit (char *error);		// defined in user program

//
// replacing refresh manager with custom routines
//

#define	MAXTICS				6
#define MAXREALTICS (2*60)
#define ANGLES		360		// must be divisable by 4

extern	fixed sintable[ANGLES+ANGLES/4],*costable;
extern	fixed	viewx,viewy;			// the focal point
extern	tilept	tile,focal,right;
extern	fixed *zbuffer;
extern	walltype	walls[],*rightwall;

extern	unsigned	tics,realtics;
extern	long lasttimecount;
extern	boolean		fizzlein;


extern	int	walllight1[];
extern	int	walldark1[];
extern	int	walllight2[];
extern	int	walldark2[];

extern unsigned topcolor,bottomcolor;


#define MAXWALLS	50
#define DANGERHIGH	45



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
boolean CheckTileCoords(int x, int y);
byte SafeTilemap(int x, int y);

extern	boolean	restarttrace, reallyabsolutelypositivelyaborttrace;

/*
=============================================================================

						 C3_DRAW DEFINITIONS

=============================================================================
*/

void	DrawWall (walltype *wallptr);
void	TraceRay (unsigned angle);
fixed	FixedByFrac (fixed a, fixed b);
void	TransformPoint (fixed gx, fixed gy, int *screenx, unsigned *screenheight);
fixed	TransformX (fixed gx, fixed gy);
int		FollowTrace (fixed tracex, fixed tracey, long deltax, long deltay, int max);
void	ForwardTrace (void);
int		FinishWall (void);
int		TurnCounterClockwise (void);
void	FollowWall (void);

void	BuildTables (void);


/*
=============================================================================

						 C3_ASM DEFINITIONS

=============================================================================
*/

extern	unsigned short	*wallheight;
extern	unsigned short	*wallwidth;
extern	uint8_t*		*wallpointer;

void	ScaleWalls (void);


// c_game.c

typedef	struct
{
	int		difficulty;
	int		mapon;
	int		bolts,nukes,potions,keys[4],scrolls[8];

	int		gems[5];				// "int allgems[5]" is used for 1:1 comparison
										// in play loop for radar... CHANGE IT, TOO!

	long	score;
	int		body,shotpower;

	uint16_t	*mapsegs[3];
} gametype;

typedef	enum	{ex_stillplaying,ex_died,ex_warped,ex_resetgame,ex_loadedgame,ex_victorious,ex_abort} exittype;

extern	ControlInfo	control;
extern	boolean		running;

extern	gametype	gamestate;
extern	objtype 	objlist[MAXACTORS],*player;

void ClearTileMap();
uint8_t GetTileMap(int X, int Y);
void SetTileMap(int X, int Y, uint8_t Value);

void ClearActorAt();
objtype *GetActorAt(int X, int Y);
intptr_t GetActorAtInt(int X, int Y);
void SetActorAt(int X, int Y, objtype *Value);
void SetActorAtInt(int X, int Y, intptr_t Value);

uint16_t GetMapSegs(int Plane, int X, int Y);
void SetMapSegs(int Plane, int X, int Y, uint16_t Value);

void ClearSpotVis();
boolean GetSpotVis(int X, int Y);
void SetSpotVis(int X, int Y, boolean Value);

void InitObjList (void);
objtype *GetNewObj (boolean usedummy);
void RemoveObj (objtype *gone);


#endif

