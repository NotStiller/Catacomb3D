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

// ID_CA.C

/*
=============================================================================

Id Software Caching Manager
---------------------------

Must be started BEFORE the memory manager, because it needs to get the headers
loaded into the data segment

=============================================================================
*/

#include "id_heads.h"
#include "id_strs.h"

/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

uint16_t    map_RLEWtag;
int32_t		*map_headeroffsets;
int			loadedmap;

uint16_t	 		*mapsegs[3];
maptype				*mapheaderseg[NUMMAPS];
byte				*audiosegs[NUMSNDCHUNKS];
void				*grsegs[NUMCHUNKS];

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

extern	byte	_maphead[]; // in data.c

FILE		*grhandle;		// handle to EGAGRAPH
FILE		*maphandle;		// handle to MAPTEMP / GAMEMAPS
FILE		*audiohandle;	// handle to AUDIOT / AUDIO

long		chunkcomplen,chunkexplen;




void	CAL_DialogDraw (char *title,unsigned numcache);
void	CAL_DialogUpdate (void);
void	CAL_DialogFinish (void);
void	CAL_CarmackExpand (uint16_t *source, uint16_t *dest,
		long length);


/*
=============================================================================

					   LOW LEVEL ROUTINES

=============================================================================
*/


/*
============================
=
= CAL_GetGrChunkLength
=
= Gets the length of an explicit length chunk (not tiles)
= The file pointer is positioned so the compressed data can be read in next.
=
============================
*/

void CAL_GetGrChunkLength (int chunk)
{
	long pos = GrChunksPos[chunk];
	assert(fseek(grhandle,pos,SEEK_SET) == 0);
	assert(fread(&chunkexplen,1,sizeof(chunkexplen),grhandle) == sizeof(chunkexplen));
	chunkcomplen = GrChunksSize[chunk]-4;
}

/*
==========================
=
= CA_FarRead
=
= Read from a file to a far pointer
=
==========================
*/

boolean CA_FarRead (FILE *handle, byte *dest, long length)
{
	unsigned long read = fread(dest, 1, length, handle);
	if (read != length)
		Quit("CA_FarRead failed!");
	return true;
}

/*
==========================
=
= CA_FarWrite
=
= Write from a file to a far pointer
=
==========================
*/

boolean CA_FarWrite (FILE *handle, byte *source, long length)
{
	if (fwrite(source, 1, length, handle) != length)
		Quit("CA_FarWrite failed!");
	return true;
}


/*
============================================================================

		COMPRESSION routines, see JHUFF.C for more

============================================================================
*/

/*
======================
=
= CAL_HuffExpand
=
= Length is the length of the EXPANDED data
=
======================
*/


int CAL_HuffExpand2 (byte *source, byte *dest,long length,huffnode *hufftable)
{
  huffnode *headptr;
  headptr = hufftable+254;	// head node is allways node 254

	huffnode *curhuffnode = headptr;
	int i, j;
	int written = 0;
	i = 0;
	while (written < length) {
		byte srcbyte = source[i++];
		for (j = 0; j < 8; j++) {
			unsigned short b = curhuffnode->bit0;

			if (srcbyte&1) {
				b = curhuffnode->bit1;
			}
			srcbyte = srcbyte>>1;
			if (b < 256) {
				dest[written++] = b;
				curhuffnode = headptr;
				if (written == length) {
					break;
				}
			} else {
			assert(b-256 >= 0);
				curhuffnode = &hufftable[b-256];
			}
		}
	}
	return i;
}

void CAL_HuffExpand (byte *source, byte *dest,long length,huffnode *hufftable)
{
  huffnode *headptr;
  headptr = hufftable+254;	// head node is allways node 254

	huffnode *curhuffnode = headptr;
	int i, j;
	int written = 0;
	i = 0;
	while (written < length) {
		byte srcbyte = source[i++];
		for (j = 0; j < 8; j++) {
			unsigned short b = curhuffnode->bit0;
			if (srcbyte&1) {
				b = curhuffnode->bit1;
			}
			srcbyte = srcbyte>>1;
			if (b < 256) {
				dest[written++] = b;
				curhuffnode = headptr;
				if (written == length) {
					break;
				}
			} else {
			assert(b-256 >= 0);
				curhuffnode = &hufftable[b-256];
			}
		}
	}
}


/*
======================
=
= CAL_CarmackExpand
=
= Length is the length of the EXPANDED data
=
======================
*/

#define NEARTAG	0xa7
#define FARTAG	0xa8

void CAL_CarmackExpand (uint16_t *source, uint16_t *dest, long length)
{
	uint16_t	ch,chhigh,count,offset;
	uint16_t	*copyptr, *inptr, *outptr;

	length/=2;

	inptr = source;
	outptr = dest;

	while (length)
	{
		ch = *inptr++;
		chhigh = ch>>8;
		if (chhigh == NEARTAG)
		{
			count = ch&0xff;
			if (!count)
			{				// have to insert a word containing the tag byte
//				ch |= *((unsigned char *)inptr)++;
				ch |= *(uint8_t *)inptr;
				inptr = (uint16_t*) ((uint8_t*)inptr+1);
				*outptr++ = ch;
				length--;
			}
			else
			{
				offset = *(uint8_t*)inptr;
				inptr = (uint16_t*)((uint8_t*)inptr+1);
				copyptr = outptr - offset;
				length -= count;
				while (count--)
					*outptr++ = *copyptr++;
			}
		}
		else if (chhigh == FARTAG)
		{
			count = ch&0xff;
			if (!count)
			{				// have to insert a word containing the tag byte
//				ch |= *((unsigned char *)inptr)++;
				ch |= *(uint8_t*)inptr;
				inptr = (uint16_t*)((uint8_t*)inptr+1);
				*outptr++ = ch;
				length --;
			}
			else
			{
				offset = *inptr++;
				copyptr = dest + offset;
				length -= count;
				while (count--)
					*outptr++ = *copyptr++;
			}
		}
		else
		{
			*outptr++ = ch;
			length --;
		}
	}
}



/*
======================
=
= CA_RLEWcompress
=
======================
*/

long CA_RLEWCompress (uint16_t *source, long length, uint16_t *dest,
  uint16_t rlewtag)
{
  long complength;
  uint16_t value,count,i;
  uint16_t *start,*end;

  start = dest;

  end = source + (length+1)/2;

//
// compress it
//
  do
  {
	count = 1;
	value = *source++;
	while (*source == value && source<end)
	{
	  count++;
	  source++;
	}
	if (count>3 || value == rlewtag)
	{
    //
    // send a tag / count / value string
    //
      *dest++ = rlewtag;
      *dest++ = count;
      *dest++ = value;
    }
    else
    {
    //
    // send word without compressing
    //
      for (i=1;i<=count;i++)
	*dest++ = value;
	}

  } while (source<end);

  complength = 2*(dest-start);
  return complength;
}


/*
======================
=
= CA_RLEWexpand
= length is EXPANDED length
=
======================
*/

void CA_RLEWexpand (uint16_t *source, uint16_t *dest,long length,
  uint16_t rlewtag)
{
  uint16_t *end = dest + (length)/2;
  uint16_t value, count, i;
  do
  {
	value = *source++;
	if (value != rlewtag)
	//
	// uncompressed
	//
	  *dest++=value;
	else
	{
	//
	// compressed string
	//
	  count = *source++;
	  value = *source++;
	  for (i=1;i<=count;i++){
		*dest++ = value;
      }
	}
  } while (dest<end);

}



/*
=============================================================================

					 CACHE MANAGER ROUTINES

=============================================================================
*/

void CAL_ReadAndHuffExpand(int Pic, memptr* PtrVar, int Size)
{
	MM_GetPtr(PtrVar,Size);
	CAL_GetGrChunkLength(Pic);
	memptr compseg;
	MM_GetPtr(&compseg,chunkcomplen);
	CA_FarRead (grhandle,compseg,chunkcomplen);
	CAL_HuffExpand (compseg, (byte*)*PtrVar,Size,grhuffman);
	MM_FreePtr(&compseg);
}

/*
======================
=
= CAL_SetupGrFile
=
======================
*/

void CAL_SetupGrFile (void)
{
	FILE *handle;
	memptr compseg;

	
//
// Open the graphics file, leaving it open until the game is finished
//
	grhandle = fopen(GREXT"GRAPH."EXTENSION, "r");
	if (grhandle == NULL)
		Quit ("Cannot open "GREXT"GRAPH."EXTENSION"!");


//
// load the pic and sprite headers into the arrays in the data segment
//
	assert(sizeof(pictabletype) == 4);
	CAL_ReadAndHuffExpand(STRUCTPIC, (memptr*)&pictable, NUMPICS*sizeof(pictabletype));
	CAL_ReadAndHuffExpand(STRUCTPICM, (memptr*)&picmtable, NUMPICM*sizeof(pictabletype));

	assert(sizeof(spritetabletype) == 18);
	CAL_ReadAndHuffExpand(STRUCTSPRITE, (memptr*)&spritetable, NUMSPRITES*sizeof(spritetabletype));

}

//==========================================================================


/*
======================
=
= CAL_SetupMapFile
=
======================
*/

void CAL_SetupMapFile (void)
{
	FILE *handle;
	long length;

/*
typedef struct
{
	uint16_t	RLEWtag;
	int8_t		headeroffsets[4*100]; // actually an array of int32 !
	int8_t		tileinfo[];
} mapfiletype;
*/
	map_RLEWtag = *(uint16_t*)_maphead;
	map_headeroffsets = (int32_t*)(_maphead+2);
	
	if ((maphandle = fopen("GAMEMAPS."EXTENSION, "r")) == NULL)
		Quit ("Can't open GAMEMAPS."EXTENSION"!");
}

//==========================================================================


/*
======================
=
= CAL_SetupAudioFile
=
======================
*/

void CAL_SetupAudioFile (void)
{
	int handle;
	long length;

//
// open the data file
//
	if ((audiohandle = fopen("AUDIO."EXTENSION, "r")) == NULL)
		Quit ("Can't open AUDIO."EXTENSION"!");
}

//==========================================================================


/*
======================
=
= CA_Startup
=
= Open all files and load in headers
=
======================
*/

void CA_Startup (void)
{
	CAL_SetupMapFile ();
	CAL_SetupGrFile ();
	CAL_SetupAudioFile ();

// Ignore this please...
	int accountedfor=0;
	int i;
	for (i = 0; i < NUMCHUNKS; i++) {
		if (i >= STARTTILE16 && i < STARTTILE16+NUMTILE16) {
			accountedfor++;
		} else if (i >= STARTTILE16M && i < STARTTILE16M+NUMTILE16M) {
			accountedfor++;
		} else if (i >= STARTTILE8 && i < STARTTILE8+NUMTILE8) {
			accountedfor++;
		} else if (i >= STARTTILE8M && i < STARTTILE8M+NUMTILE8M) {
			accountedfor++;
		} else if (i >= STARTPICS && i < STARTPICS+NUMPICS) {
			accountedfor++;
		} else if (i >= STARTPICM && i < STARTPICM+NUMPICM) {
			accountedfor++;
		} else if (i >= STARTFONT && i < STARTFONT+NUMFONT) {
			accountedfor++;
		} else if (i >= STARTFONTM && i < STARTFONTM+NUMFONTM) {
			accountedfor++;
		} else if (i >= STARTSPRITES && i < STARTSPRITES+NUMSPRITES) {
			accountedfor++;
		} else if (i >= STARTEXTERNS && i < STARTEXTERNS+NUMEXTERNS) {
			accountedfor++;
		}
		CA_CacheGrChunk(i);
	}
	printf("Chunks accounted for %i/%i\n", accountedfor, NUMCHUNKS);

	loadedmap = -1;
}

//==========================================================================


/*
======================
=
= CA_Shutdown
=
= Closes all files
=
======================
*/

void CA_Shutdown (void)
{
	if (maphandle != NULL) {
		fclose (maphandle);
	}
	if (grhandle != NULL) {
		fclose (grhandle);
	}
	if (audiohandle != NULL) {
		fclose (audiohandle);
	}
}

//===========================================================================

/*
======================
=
= CA_CacheAudioChunk
=
======================
*/

void CA_CacheAudioChunk (int chunk)
{
	long	pos,compressed;
	long	expanded;
	memptr	bigbufferseg;
	byte	*source;

	if (audiosegs[chunk])
	{
		return;							// allready in memory
	}

	pos = AudioChunksPos[chunk];
	compressed = AudioChunksSize[chunk];

	fseek(audiohandle,pos,SEEK_SET);

	MM_GetPtr(&bigbufferseg,compressed);
	source = bigbufferseg;
	CA_FarRead(audiohandle,source,compressed);
	expanded = *(int32_t*)source;
	source += 4;			// skip over length
	MM_GetPtr ((memptr*)&(audiosegs[chunk]),expanded);
	int bytesread = CAL_HuffExpand2 (source,audiosegs[chunk],expanded,audiohuffman);
	assert(compressed >= bytesread);

done:
	MM_FreePtr(&bigbufferseg);
}

//===========================================================================

/*
======================
=
= CA_LoadAllSounds
=
= Purges all sounds, then loads all new ones (mode switch)
=
======================
*/

void CA_LoadAllSounds (void)
{
	if (SoundMode == sdm_AdLib)
	{
		int i;
		for (i=0;i<NUMSOUNDS;i++)
			CA_CacheAudioChunk (STARTADLIBSOUNDS+i);
	}
}


//===========================================================================

/*
======================
=
= CAL_CacheSprite
=
= Generate shifts and set up sprite structure for a given sprite
=
======================
*/

void CAL_CacheSprite (int chunk, byte *compressed)
{
	int i;
	unsigned expanded;
	spritetabletype *spr;
	spritetype *dest;

//
// calculate sizes
//

	spr = &spritetable[chunk-STARTSPRITES];
	expanded = MAXSHIFTS*6+5*spr->width*spr->height;
	MM_GetPtr ((memptr*)&(grsegs[chunk]),expanded);
	dest = (spritetype*)grsegs[chunk];

	for (i=0;i<MAXSHIFTS;i++)
	{
		dest->sourceoffset[i] = MAXSHIFTS*6;
		dest->planesize[i] = spr->width*spr->height;
		dest->width[i] = spr->width;
	}
//
// expand the unshifted shape
//
	CAL_HuffExpand (compressed, &dest->data[0],5*spr->width*spr->height,grhuffman);
}

//===========================================================================


/*
======================
=
= CAL_ExpandGrChunk
=
= Does whatever is needed with a pointer to a compressed chunk
=
======================
*/

void CAL_ExpandGrChunk (int chunk, byte *source)
{
	long	expanded;


	if (chunk >= STARTTILE8 && chunk < STARTEXTERNS)
	{
	//
	// expanded sizes of tile8/16/32 are implicit
	//

#define BLOCK		32
#define MASKBLOCK	40

		if (chunk<STARTTILE8M)			// tile 8s are all in one chunk!
			expanded = BLOCK*NUMTILE8;
		else if (chunk<STARTTILE16)
			expanded = MASKBLOCK*NUMTILE8M;
		else if (chunk<STARTTILE16M)	// all other tiles are one/chunk
			expanded = BLOCK*4;
		else if (chunk<STARTTILE32)
			expanded = MASKBLOCK*4;
		else if (chunk<STARTTILE32M)
			expanded = BLOCK*16;
		else
			expanded = MASKBLOCK*16;
	}
	else
	{
	//
	// everything else has an explicit size longword
	//
		expanded = *(long*)source;
		source += 4;			// skip over length
	}

//
// allocate final space, decompress it, and free bigbuffer
// Sprites need to have shifts made and various other junk
//
	if (chunk>=STARTSPRITES && chunk< STARTTILE8)
		CAL_CacheSprite(chunk,source);
	else
	{
		MM_GetPtr ((memptr*)&(grsegs[chunk]),expanded);
		CAL_HuffExpand (source,grsegs[chunk],expanded,grhuffman);
	}
}


/*
======================
=
= CA_CacheGrChunk
=
= Makes sure a given chunk is in memory, loadiing it if needed
=
======================
*/

void CA_CacheGrChunk (int chunk)
{
	long	pos,compressed;
	memptr	bigbufferseg;
	int		next;

	if (grsegs[chunk])
	{
		return;							// allready in memory
	}

	pos = GrChunksPos[chunk];
	compressed = GrChunksSize[chunk];
	if (pos < 0)
	{
		return;
	}

	fseek(grhandle,pos,SEEK_SET);

	MM_GetPtr(&bigbufferseg,compressed);
	CA_FarRead(grhandle,bigbufferseg,compressed);
	CAL_ExpandGrChunk (chunk,bigbufferseg);
	MM_FreePtr(&bigbufferseg);
}



//==========================================================================

/*
======================
=
= CA_CacheMap
=
======================
*/

void CA_CacheMap (int mapnum)
{
	int32_t	pos,compressed;
	int		plane;
	memptr	*dest;
	memptr bigbufferseg;
	uint16_t	size;
	int16_t *source;
	memptr	buffer2seg;
	long	expanded;

printf("CA_CacheMap(%i)\n", mapnum);
//
// free up memory from last map
//
	for (plane=0;plane<MAPPLANES;plane++)
		if (mapsegs[plane])
			MM_FreePtr ((memptr*)&mapsegs[plane]);

	loadedmap = mapnum;


//
// load map header
// The header will be cached if it is still around
//
	if (!mapheaderseg[mapnum])
	{
		pos = map_headeroffsets[mapnum];
		if (pos<0)						// $FFFFFFFF start is a sparse map
		  Quit ("CA_CacheMap: Tried to load a non existent map!");

		MM_GetPtr((memptr*)&mapheaderseg[mapnum],sizeof(maptype));
		maptype *map = mapheaderseg[mapnum];
		byte *buffer;
		MM_GetPtr(&buffer,12+6+4+16);
		fseek(maphandle,pos,SEEK_SET);
		CA_FarRead (maphandle,buffer,sizeof(maptype));
/* I am afraid of packing issues, so let's unpack this manually here:
typedef	struct {
	int32_t			planestart[3];
	uint16_t		planelength[3];
	uint16_t		width,height;
	char			name[16];
} maptype;
*/
		map->planestart[0] = *(int32_t*)(buffer+0);
		map->planestart[1] = *(int32_t*)(buffer+4);
		map->planestart[2] = *(int32_t*)(buffer+8);
		map->planelength[0] = *(uint16_t*)(buffer+12);
		map->planelength[1] = *(uint16_t*)(buffer+14);
		map->planelength[2] = *(uint16_t*)(buffer+16);
		map->width = *(uint16_t*)(buffer+18);
		map->height = *(uint16_t*)(buffer+20);
		int i;
		for (i = 0; i < 16; i++)
		{
			map->name[i] = buffer[22+i];
		}
		MM_FreePtr(&buffer);
	}

//
// load the planes in
// If a plane's pointer still exists it will be overwritten (levels are
// allways reloaded, never cached)
//

	size = mapheaderseg[mapnum]->width * mapheaderseg[mapnum]->height * 2;

	for (plane = 0; plane<MAPPLANES; plane++)
	{
		pos = mapheaderseg[mapnum]->planestart[plane];
		compressed = mapheaderseg[mapnum]->planelength[plane];

		if (!compressed)
			continue;		// the plane is not used in this game

		dest = (memptr*)&mapsegs[plane];
		MM_GetPtr(dest,size);

		fseek(maphandle,pos,SEEK_SET);
		MM_GetPtr(&bigbufferseg,compressed);
		CA_FarRead(maphandle,(byte *)bigbufferseg,compressed);
		//
		// unhuffman, then unRLEW
		// The huffman'd chunk has a two byte expanded length first
		// The resulting RLEW chunk also does, even though it's not really
		// needed
		//
		source = bigbufferseg;
		expanded = *source;
		source++;
		MM_GetPtr (&buffer2seg,expanded);
		CAL_CarmackExpand (source, (uint16_t*)buffer2seg,expanded);
		CA_RLEWexpand ((uint16_t*)buffer2seg+1,*dest,size,map_RLEWtag);
		MM_FreePtr (&buffer2seg);
		MM_FreePtr(&bigbufferseg);
	}
printf("done CA_CacheMap(%i)\n", mapnum);
}

//===========================================================================
/*
======================
=
= CA_SetAllPurge
=
= Make everything possible purgable
=
======================
*/

void CA_SetAllPurge (void)
{
	int i;

	for (i=0;i<3;i++)
		if (mapsegs[i])
			MM_FreePtr ((memptr*)&(mapsegs[i]));
}

//===========================================================================


