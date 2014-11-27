#include "id_heads.h"

static uint8_t spotvis[MAPSIZE][MAPSIZE];
static uint8_t tilemap[MAPSIZE][MAPSIZE];
static objtype *actorat[MAPSIZE][MAPSIZE];

ControlInfo	control;
boolean		running;

gametype *gamestate;
objtype objlist[MAXACTORS], *lastobj, *objfreelist;
objtype *player;
int	objectcount;

static int goodCoords(int X, int Y) {
	return !(curmap == NULL || X < 0 || Y < 0 || X >= curmap->width || Y >= curmap->height);
}

void ClearTileMap() {
	memset(tilemap, 0, sizeof(tilemap));
}

uint8_t GetTileMap(int X, int Y) {
	if (!goodCoords(X,Y)) {
		return 0;
	}
	return tilemap[X][Y];
}

void SetTileMap(int X, int Y, uint8_t Value) {
	if (!goodCoords(X,Y)) {
		printf("SetTileMap(%i,%i,%i) ignored because coords are out of bounds !\n", X,Y,(int)Value);
		return;
	}
	tilemap[X][Y] = Value;
}

void ClearActorAt() {
	memset(actorat, 0, sizeof(actorat));
}

objtype *GetActorAt(int X, int Y) {
	if (!goodCoords(X,Y)) {
		return NULL;
	}
	return actorat[X][Y];
}

void SetActorAt(int X, int Y, objtype *Value) {
	if (!goodCoords(X,Y)) {
		printf("SetActorAt(%i,%i,%p) ignored because coords are out of bounds !\n", X,Y,Value);
		return;
	}
	actorat[X][Y] = Value;
}

intptr_t GetActorAtInt(int X, int Y) {
	if (!goodCoords(X,Y)) {
		return 0;
	}
	return *(intptr_t*)&actorat[X][Y];
}

void SetActorAtInt(int X, int Y, intptr_t Value) {
	if (!goodCoords(X,Y)) {
		printf("SetActorAtInt(%i,%i,%i) ignored because coords are out of bounds !\n", X,Y,(int)Value);
		return;
	}
	*(intptr_t*)&actorat[X][Y] = Value;
}

uint16_t GetMapSegs(int Plane, int X, int Y) {
	if (!goodCoords(X,Y)) {
		return 0;
	}
	if (Plane < 0 || Plane > 2) {
		return 0;
	}

	uint16_t *p = gamestate->mapsegs[Plane];
	uint16_t v = p[Y*curmap->width+X];
	return v;
}

void SetMapSegs(int Plane, int X, int Y, uint16_t Value) {
	if (!goodCoords(X,Y)) {
		printf("SetMapSegs(%i,%i,%i) ignored because coords are out of bounds !\n", X,Y,(int)Value);
		return;
	}
	assert(Plane >= 0 && Plane <= 2);
	gamestate->mapsegs[Plane][Y*curmap->width+X] = Value;
}

void ClearSpotVis() {
	assert(sizeof(spotvis) == MAPSIZE*MAPSIZE);
	memset(spotvis, 0, MAPSIZE*MAPSIZE);
}

boolean GetSpotVis(int X, int Y) {
	if (!goodCoords(X,Y)) {
		return false;
	}
	return spotvis[X][Y];
}

void SetSpotVis(int X, int Y, boolean Value) {
	if (!goodCoords(X,Y)) {
		printf("SetSpotVis(%i,%i,%i) ignored because coords are out of bounds !\n", X,Y,(int)Value);
		return;
	}
	spotvis[X][Y] = Value;
}


/*
#############################################################################

				  The objlist data structure

#############################################################################

objlist containt structures for every actor currently playing.  The structure
is accessed as a linked list starting at *player, ending when ob->next ==
NULL.  GetNewObj inserts a new object at the end of the list, meaning that
if an actor spawn another actor, the new one WILL get to think and react the
same frame.  RemoveObj unlinks the given object and returns it to the free
list, but does not damage the objects ->next pointer, so if the current object
removes itself, a linked list following loop can still safely get to the
next element.

<backwardly linked free list>

#############################################################################
*/


/*
=========================
=
= InitObjList
=
= Call to clear out the entire object list, returning them all to the free
= list.  Allocates a special spot for the player.
=
=========================
*/

void InitObjList (void)
{
	int	i;

	for (i=0;i<MAXACTORS;i++)
	{
		objlist[i].prev = &objlist[i+1];
		objlist[i].next = NULL;
	}

	objlist[MAXACTORS-1].prev = NULL;

	objfreelist = &objlist[0];
	lastobj = NULL;

	objectcount = 0;

//
// give the player and score the first free spots
//
	player = GetNewObj (false);
}

//===========================================================================

/*
=========================
=
= GetNewObj
=
= Sets the global variable new to point to a free spot in objlist.
= The free spot is inserted at the end of the liked list
=
= When the object list is full, the caller can either have it bomb out or
= return NULL.
=
=========================
*/
objtype *GetNewObj (boolean nullokay)
{
	if (!objfreelist)
	{
		if (nullokay)
		{
			return NULL;
		}
		Quit ("GetNewObj: No free spots in objlist!");
	}

	objtype *new;
	new = objfreelist;
	objfreelist = new->prev;
	memset (new,0,sizeof(*new));
	new->next = NULL;

	if (lastobj)
		lastobj->next = new;
	new->prev = lastobj;	// new->next is allready NULL from memset

	new->active = false;
	lastobj = new;

	objectcount++;

	return new;
}

//===========================================================================

/*
=========================
=
= RemoveObj
=
= Add the given object back into the free list, and unlink it from it's
= neighbors
=
=========================
*/

void RemoveObj (objtype *gone)
{

	objtype **spotat;

	if (gone == player)
		Quit ("RemoveObj: Tried to remove the player!");

//
// fix the next object's back link
//
	if (gone == lastobj)
		lastobj = (objtype *)gone->prev;
	else
		gone->next->prev = gone->prev;

//
// fix the previous object's forward link
//
	gone->prev->next = gone->next;

//
// add it back in to the free list
//
	gone->prev = objfreelist;
	objfreelist = gone;
}


