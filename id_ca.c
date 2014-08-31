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
#pragma hdrstop
#include "id_strs.h"

#pragma warn -pro
#pragma warn -use

#define THREEBYTEGRSTARTS


static long filelength(FILE *handle) {
	long pos = ftell(handle);
	fseek(handle, 0, SEEK_END);
	long pos2 = ftell(handle);
	fseek(handle, pos, SEEK_SET);
	return pos2;
}

/*
=============================================================================

						 LOCAL CONSTANTS

=============================================================================
*/

typedef struct
{
  unsigned short bit0,bit1;	// 0-255 is a character, > is a pointer to a node
} huffnode;


typedef struct
{
	unsigned short	RLEWtag;
	byte		headeroffsets[4*100];
	byte		tileinfo[];
} mapfiletype;


/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/

byte 		_seg	*tinf;
int			mapon;

unsigned short 		*mapsegs[3];
maptype				*mapheaderseg[NUMMAPS];
byte				*audiosegs[NUMSNDCHUNKS];
void				*grsegs[NUMCHUNKS];

byte		far	grneeded[NUMCHUNKS];
byte		ca_levelbit,ca_levelnum;

FILE		*profilehandle,*debughandle;

void	(*drawcachebox)		(char *title, unsigned numcache);
void	(*updatecachebox)	(void);
void	(*finishcachebox)	(void);

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/

extern	byte	EGAhead[];
extern	byte	EGAdict[];
extern	byte	_maphead[];
extern	byte	audiohead[];
extern	byte	audiodict[];


long		_seg *grstarts;	// array of offsets in egagraph, -1 for sparse
long		_seg *audiostarts;	// array of offsets in audio / audiot

#ifdef GRHEADERLINKED
huffnode	*grhuffman;
#else
huffnode	grhuffman[255];
#endif

#ifdef AUDIOHEADERLINKED
huffnode	*audiohuffman;
#else
huffnode	audiohuffman[255];
#endif


FILE		*grhandle;		// handle to EGAGRAPH
FILE		*maphandle;		// handle to MAPTEMP / GAMEMAPS
FILE		*audiohandle;	// handle to AUDIOT / AUDIO

long		chunkcomplen,chunkexplen;

SDMode		oldsoundmode;



void	CAL_DialogDraw (char *title,unsigned numcache);
void	CAL_DialogUpdate (void);
void	CAL_DialogFinish (void);
void	CAL_CarmackExpand (unsigned short *source, unsigned short *dest,
		unsigned short length);


#ifdef THREEBYTEGRSTARTS
#define FILEPOSSIZE	3
//#define	GRFILEPOS(c) (*(long far *)(((byte far *)grstarts)+(c)*3)&0xffffff)
long GRFILEPOS(int c)
{
	long value;
	int	offset;

	offset = c*3;
	value = *(long*)((unsigned long)grstarts+offset);

	value &= 0x00ffffffl;

	if (value == 0xffffffl)
		value = -1;

	return value;
};
#else
#define FILEPOSSIZE	4
#define	GRFILEPOS(c) (grstarts[c])
#endif

/*
=============================================================================

					   LOW LEVEL ROUTINES

=============================================================================
*/

/*
============================
=
= CA_OpenDebug / CA_CloseDebug
=
= Opens a binary file with the handle "debughandle"
=
============================
*/

void CA_OpenDebug (void)
{
	unlink ("DEBUG.TXT");
	debughandle = fopen("DEBUG.TXT", "w");
}

void CA_CloseDebug (void)
{
	fclose (debughandle);
}



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
	long pos = GRFILEPOS(chunk);
	assert(fseek(grhandle,pos,SEEK_SET) == 0);
	assert(fread(&chunkexplen,1,sizeof(chunkexplen),grhandle) == sizeof(chunkexplen));
	chunkcomplen = GRFILEPOS(chunk+1)-GRFILEPOS(chunk)-4;
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
/*
asm		push	ds
asm		mov	bx,[handle]
asm		mov	cx,[WORD PTR length]
asm		mov	dx,[WORD PTR dest]
asm		mov	ds,[WORD PTR dest+2]
asm		mov	ah,0x3f				// READ w/handle
asm		int	21h
asm		pop	ds
asm		jnc	good
	errno = _AX;
	return	false;
good:
asm		cmp	ax,[WORD PTR length]
asm		je	done
	errno = EINVFMT;			// user manager knows this is bad read
	return	false;
done:
	return	true;*/
}


/*
==========================
=
= CA_SegWrite
=
= Write from a file to a far pointer
=
==========================
*/

boolean CA_FarWrite (FILE *handle, byte far *source, long length)
{
	if (length>0xffffl)
		Quit ("CA_FarWrite doesn't support 64K reads yet!");

	if (fwrite(source, 1, length, handle) != length)
		Quit("CA_FarWrite failed!");
	return true;
}



/*
==========================
=
= CA_ReadFile
=
= Reads a file into an allready allocated buffer
=
==========================
*/

boolean CA_ReadFile (char *filename, memptr *ptr)
{
	FILE *handle;
	long size;

	if ((handle = fopen(filename,"r")) == NULL)
		return false;

	size = filelength (handle);
	if (!CA_FarRead (handle,*ptr,size))
	{
		fclose (handle);
		return false;
	}
	fclose (handle);
	return true;
}



/*
==========================
=
= CA_LoadFile
=
= Allocate space for and load a file
=
==========================
*/

boolean CA_LoadFile (char *filename, memptr *ptr)
{
	FILE *handle;
	long size;

	if ((handle = fopen(filename,"r")) == NULL)
		return false;

	size = filelength (handle);
	MM_GetPtr (ptr,size);
	if (!CA_FarRead (handle,*ptr,size))
	{
		fclose (handle);
		return false;
	}
	fclose (handle);
	return true;
}

/*
============================================================================

		COMPRESSION routines, see JHUFF.C for more

============================================================================
*/



/*
===============
=
= CAL_OptimizeNodes
=
= Goes through a huffman table and changes the 256-511 node numbers to the
= actular address of the node.  Must be called before CAL_HuffExpand
=
===============
*/

void CAL_OptimizeNodes (huffnode *table)
{
}



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

/*
	if (length <0xfff0)
	{
//--------------------------
// expand less than 64k of data
//--------------------------

asm mov	bx,[headptr]

asm	mov	si,[sourceoff]
asm	mov	di,[destoff]
asm	mov	es,[destseg]
asm	mov	ds,[sourceseg]
asm	mov	ax,[endoff]

asm	mov	ch,[si]				// load first byte
asm	inc	si
asm	mov	cl,1

expandshort:
asm	test	ch,cl			// bit set?
asm	jnz	bit1short
asm	mov	dx,[ss:bx]			// take bit0 path from node
asm	shl	cl,1				// advance to next bit position
asm	jc	newbyteshort
asm	jnc	sourceupshort

bit1short:
asm	mov	dx,[ss:bx+2]		// take bit1 path
asm	shl	cl,1				// advance to next bit position
asm	jnc	sourceupshort

newbyteshort:
asm	mov	ch,[si]				// load next byte
asm	inc	si
asm	mov	cl,1				// back to first bit

sourceupshort:
asm	or	dh,dh				// if dx<256 its a byte, else move node
asm	jz	storebyteshort
asm	mov	bx,dx				// next node = (huffnode *)code
asm	jmp	expandshort

storebyteshort:
asm	mov	[es:di],dl
asm	inc	di					// write a decopmpressed byte out
asm	mov	bx,[headptr]		// back to the head node for next bit

asm	cmp	di,ax				// done?
asm	jne	expandshort
	}
	else
	{
//--------------------------
// expand more than 64k of data
//--------------------------

  length--;

asm mov	bx,[headptr]
asm	mov	cl,1

asm	mov	si,[sourceoff]
asm	mov	di,[destoff]
asm	mov	es,[destseg]
asm	mov	ds,[sourceseg]

asm	lodsb			// load first byte

expand:
asm	test	al,cl		// bit set?
asm	jnz	bit1
asm	mov	dx,[ss:bx]	// take bit0 path from node
asm	jmp	gotcode
bit1:
asm	mov	dx,[ss:bx+2]	// take bit1 path

gotcode:
asm	shl	cl,1		// advance to next bit position
asm	jnc	sourceup
asm	lodsb
asm	cmp	si,0x10		// normalize ds:si
asm  	jb	sinorm
asm	mov	cx,ds
asm	inc	cx
asm	mov	ds,cx
asm	xor	si,si
sinorm:
asm	mov	cl,1		// back to first bit

sourceup:
asm	or	dh,dh		// if dx<256 its a byte, else move node
asm	jz	storebyte
asm	mov	bx,dx		// next node = (huffnode *)code
asm	jmp	expand

storebyte:
asm	mov	[es:di],dl
asm	inc	di		// write a decopmpressed byte out
asm	mov	bx,[headptr]	// back to the head node for next bit

asm	cmp	di,0x10		// normalize es:di
asm  	jb	dinorm
asm	mov	dx,es
asm	inc	dx
asm	mov	es,dx
asm	xor	di,di
dinorm:

asm	sub	[WORD PTR ss:length],1
asm	jnc	expand
asm  	dec	[WORD PTR ss:length+2]
asm	jns	expand		// when length = ffff ffff, done
	}
asm	mov	ax,ss
asm	mov	ds,ax
*/
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

void CAL_CarmackExpand (unsigned short *source, unsigned short *dest, unsigned short length)
{
	unsigned short	ch,chhigh,count,offset;
	unsigned short	*copyptr, *inptr, *outptr;

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
//				ch |= *((unsigned char far *)inptr)++;
				ch |= *(unsigned char *)inptr;
				inptr = (unsigned short*) ((unsigned char*)inptr+1);
				*outptr++ = ch;
				length--;
			}
			else
			{
				offset = *(unsigned char far *)inptr;
				inptr = (unsigned short*)((unsigned char*)inptr+1);
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
//				ch |= *((unsigned char far *)inptr)++;
				ch |= *(unsigned char*)inptr;
				inptr = (unsigned short*)((unsigned char*)inptr+1);
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

long CA_RLEWCompress (unsigned short *source, long length, unsigned short *dest,
  unsigned short rlewtag)
{
  long complength;
  unsigned short value,count,i;
  unsigned short *start,*end;

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

void CA_RLEWexpand (unsigned short *source, unsigned short *dest,long length,
  unsigned rlewtag)
{
  unsigned short *end = dest + (length)/2;
  unsigned short value, count, i;
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

	
#ifdef GRHEADERLINKED

	grhuffman = (huffnode *)EGAdict;
	grstarts = (long*)EGAhead;

	CAL_OptimizeNodes (grhuffman);

#else

//
// load ???dict.ext (huffman dictionary for graphics files)
//

	if ((handle = fopen(GREXT"DICT."EXTENSION, "r")) == NULL)
		Quit ("Can't open "GREXT"DICT."EXTENSION"!");

	fread(&grhuffman, 1, sizeof(grhuffman), handle);
	fclose(handle);
	CAL_OptimizeNodes (grhuffman);
//
// load the data offsets from ???head.ext
//
	MM_GetPtr ((memptr*)&grstarts,(NUMCHUNKS+1)*FILEPOSSIZE);

	if ((handle = fopen(GREXT"HEAD."EXTENSION, "r")) == NULL)
		Quit ("Can't open "GREXT"HEAD."EXTENSION"!");

	CA_FarRead(handle, (memptr)grstarts, (NUMCHUNKS+1)*FILEPOSSIZE);

	fclose(handle);


#endif

//
// Open the graphics file, leaving it open until the game is finished
//
	grhandle = fopen(GREXT"GRAPH."EXTENSION, "r");
	if (grhandle == NULL)
		Quit ("Cannot open "GREXT"GRAPH."EXTENSION"!");


//
// load the pic and sprite headers into the arrays in the data segment
//
#if NUMPICS>0
	MM_GetPtr((memptr*)&pictable,NUMPICS*sizeof(pictabletype));
	CAL_GetGrChunkLength(STRUCTPIC);		// position file pointer
	MM_GetPtr(&compseg,chunkcomplen);
	CA_FarRead (grhandle,compseg,chunkcomplen);
	CAL_HuffExpand (compseg, (byte*)pictable,NUMPICS*sizeof(pictabletype),grhuffman);
	MM_FreePtr(&compseg);
#endif

#if NUMPICM>0
	MM_GetPtr((memptr*)&picmtable,NUMPICM*sizeof(pictabletype));
	CAL_GetGrChunkLength(STRUCTPICM);		// position file pointer
	MM_GetPtr(&compseg,chunkcomplen);
	CA_FarRead (grhandle,compseg,chunkcomplen);
	CAL_HuffExpand (compseg, (byte*)picmtable,NUMPICM*sizeof(pictabletype),grhuffman);
	MM_FreePtr(&compseg);
#endif

#if NUMSPRITES>0
	MM_GetPtr((memptr*)&spritetable,NUMSPRITES*sizeof(spritetabletype));
	CAL_GetGrChunkLength(STRUCTSPRITE);	// position file pointer
	MM_GetPtr(&compseg,chunkcomplen);
	CA_FarRead (grhandle,compseg,chunkcomplen);
	assert(NUMSPRITES*sizeof(spritetabletype) == chunkexplen);
	CAL_HuffExpand (compseg, (byte*)spritetable,NUMSPRITES*sizeof(spritetabletype),grhuffman);
	MM_FreePtr(&compseg);
#endif

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

//
// load maphead.ext (offsets and tileinfo for map file)
//
#ifndef MAPHEADERLINKED
	if ((handle = fopen("MAPHEAD."EXTENSION, "r")) == NULL)
		Quit ("Can't open MAPHEAD."EXTENSION"!");
	length = filelength(handle);
	MM_GetPtr ((memptr*)&tinf,length);
	CA_FarRead(handle, tinf, length);
	fclose(handle);
#else

	tinf = (byte*)_maphead;

#endif

//
// open the data file
//
#ifdef MAPHEADERLINKED
	if ((maphandle = fopen("GAMEMAPS."EXTENSION, "r")) == NULL)
		Quit ("Can't open GAMEMAPS."EXTENSION"!");
#else
	if ((maphandle = fopen("MAPTEMP."EXTENSION, "r")) == NULL)
		Quit ("Can't open MAPTEMP."EXTENSION"!");
#endif
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
// load maphead.ext (offsets and tileinfo for map file)
//
#ifndef AUDIOHEADERLINKED
	if ((handle = open("AUDIOHED."EXTENSION,
		 O_RDONLY | O_BINARY, S_IREAD)) == -1)
		Quit ("Can't open AUDIOHED."EXTENSION"!");
	length = filelength(handle);
	MM_GetPtr ((memptr*)&audiostarts,length);
	CA_FarRead(handle, (byte far *)audiostarts, length);
	fclose(handle);
#else
	audiohuffman = (huffnode *)audiodict;
	CAL_OptimizeNodes (audiohuffman);
	audiostarts = (long*)audiohead;
#endif


//
// open the data file
//
#ifndef AUDIOHEADERLINKED
	if ((audiohandle = fopen("AUDIOT."EXTENSION, "r")) == NULL)
		Quit ("Can't open AUDIOT."EXTENSION"!");
#else
	if ((audiohandle = fopen("AUDIO."EXTENSION, "r")) == NULL)
		Quit ("Can't open AUDIO."EXTENSION"!");
#endif
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
#ifndef NOMAPS
	CAL_SetupMapFile ();
#endif
#ifndef NOGRAPHICS
	CAL_SetupGrFile ();
#endif
#ifndef NOAUDIO
	CAL_SetupAudioFile ();
#endif

	mapon = -1;
	ca_levelbit = 1;
	ca_levelnum = 0;

	drawcachebox	= CAL_DialogDraw;
	updatecachebox  = CAL_DialogUpdate;
	finishcachebox	= CAL_DialogFinish;
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
#ifdef PROFILE
	fclose (profilehandle);
#endif

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
#ifdef AUDIOHEADERLINKED
	long	expanded;
	memptr	bigbufferseg;
	byte	far *source;
#endif

	if (audiosegs[chunk])
	{
		MM_SetPurge ((memptr*)&audiosegs[chunk],0);
		return;							// allready in memory
	}

//
// load the chunk into a buffer, either the miscbuffer if it fits, or allocate
// a larger buffer
//
	pos = audiostarts[chunk];
	compressed = audiostarts[chunk+1]-pos;

	fseek(audiohandle,pos,SEEK_SET);

#ifndef AUDIOHEADERLINKED

	MM_GetPtr ((memptr*)&audiosegs[chunk],compressed);
	if (mmerror)
		return;

	CA_FarRead(audiohandle,audiosegs[chunk],compressed);

#else

	if (compressed<=BUFFERSIZE)
	{
		CA_FarRead(audiohandle,bufferseg,compressed);
		source = bufferseg;
	}
	else
	{
		MM_GetPtr(&bigbufferseg,compressed);
		if (mmerror)
			return;
		MM_SetLock (&bigbufferseg,true);
		CA_FarRead(audiohandle,bigbufferseg,compressed);
		source = bigbufferseg;
	}

	expanded = *(long far *)source;
	source += 4;			// skip over length
	MM_GetPtr ((memptr*)&(audiosegs[chunk]),expanded);
	if (mmerror)
		goto done;
	int bytesread = CAL_HuffExpand2 (source,audiosegs[chunk],expanded,audiohuffman);
	assert(compressed >= bytesread);

done:
	if (compressed>BUFFERSIZE)
		MM_FreePtr(&bigbufferseg);
#endif
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
	unsigned	start,i;

	switch (oldsoundmode)
	{
	case sdm_Off:
		goto cachein;
	case sdm_PC:
		start = STARTPCSOUNDS;
		break;
	case sdm_AdLib:
		start = STARTADLIBSOUNDS;
		break;
	}

	for (i=0;i<NUMSOUNDS;i++,start++)
		if (audiosegs[start])
			MM_SetPurge ((memptr*)(audiosegs[start]),3);		// make purgable

cachein:

	switch (SoundMode)
	{
	case sdm_Off:
		return;
	case sdm_PC:
		start = STARTPCSOUNDS;
		break;
	case sdm_AdLib:
		start = STARTADLIBSOUNDS;
		break;
	}
	assert(SoundMode == sdm_AdLib);

	for (i=0;i<NUMSOUNDS;i++,start++)
		CA_CacheAudioChunk (start);

	oldsoundmode = SoundMode;
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

void CAL_CacheSprite (int chunk, byte far *compressed)
{
	int i;
	unsigned expanded;
	spritetabletype far *spr;
	spritetype _seg *dest;

//
// calculate sizes
//

	spr = &spritetable[chunk-STARTSPRITES];
	expanded = MAXSHIFTS*6+5*spr->width*spr->height;
	MM_GetPtr ((memptr*)&(grsegs[chunk]),expanded);
	if (mmerror)
		return;
	dest = (spritetype _seg *)grsegs[chunk];

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
		if (mmerror)
			return;
		CAL_HuffExpand (source,grsegs[chunk],expanded,grhuffman);
	}
}


/*
======================
=
= CAL_ReadGrChunk
=
= Gets a chunk off disk, optimizing reads to general buffer
=
======================
*/

void CAL_ReadGrChunk (int chunk)
{
	long	pos,compressed;
	memptr	bigbufferseg;
	byte	far *source;
	int		next;

//
// load the chunk into a buffer, either the miscbuffer if it fits, or allocate
// a larger buffer
//
	pos = GRFILEPOS(chunk);
	if (pos<0)							// $FFFFFFFF start is a sparse tile
	  return;

	next = chunk +1;
	while (GRFILEPOS(next) == -1)		// skip past any sparse tiles
		next++;

	compressed = GRFILEPOS(next)-pos;

	fseek(grhandle,pos,SEEK_SET);

	if (compressed<=BUFFERSIZE)
	{
		CA_FarRead(grhandle,bufferseg,compressed);
		source = bufferseg;
	}
	else
	{
		MM_GetPtr(&bigbufferseg,compressed);
		if (mmerror)
			return;
		MM_SetLock (&bigbufferseg,true);
		CA_FarRead(grhandle,bigbufferseg,compressed);
		source = bigbufferseg;
	}

	CAL_ExpandGrChunk (chunk,source);

	if (compressed>BUFFERSIZE)
		MM_FreePtr(&bigbufferseg);
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
	byte	far *source;
	int		next;

	grneeded[chunk] |= ca_levelbit;		// make sure it doesn't get removed
	if (grsegs[chunk])
	{
		MM_SetPurge ((memptr*)&(grsegs[chunk]),0);
		return;							// allready in memory
	}

//
// load the chunk into a buffer, either the miscbuffer if it fits, or allocate
// a larger buffer
//
	pos = GRFILEPOS(chunk);
	if (pos<0)							// $FFFFFFFF start is a sparse tile
	  return;

	next = chunk +1;
	while (GRFILEPOS(next) == -1)		// skip past any sparse tiles
		next++;

	compressed = GRFILEPOS(next)-pos;

	fseek(grhandle,pos,SEEK_SET);

	if (compressed<=BUFFERSIZE)
	{
		CA_FarRead(grhandle,bufferseg,compressed);
		source = bufferseg;
	}
	else
	{
		MM_GetPtr(&bigbufferseg,compressed);
		MM_SetLock (&bigbufferseg,true);
		CA_FarRead(grhandle,bigbufferseg,compressed);
		source = bigbufferseg;
	}

	CAL_ExpandGrChunk (chunk,source);

	if (compressed>BUFFERSIZE)
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
	long	pos,compressed;
	int		plane;
	memptr	*dest;
	memptr bigbufferseg;
	unsigned	size;
	unsigned short *source;
#ifdef MAPHEADERLINKED
	memptr	buffer2seg;
	long	expanded;
#endif


//
// free up memory from last map
//
	if (mapon>-1 && mapheaderseg[mapon])
		MM_SetPurge ((memptr*)&(mapheaderseg[mapon]),3);
	for (plane=0;plane<MAPPLANES;plane++)
		if (mapsegs[plane])
			MM_FreePtr ((memptr*)&mapsegs[plane]);

	mapon = mapnum;


//
// load map header
// The header will be cached if it is still around
//
	if (!mapheaderseg[mapnum])
	{
		pos = ((long*)&((mapfiletype*)tinf)->headeroffsets[0])[mapnum];
		if (pos<0)						// $FFFFFFFF start is a sparse map
		  Quit ("CA_CacheMap: Tried to load a non existent map!");

		MM_GetPtr((memptr*)&mapheaderseg[mapnum],sizeof(maptype));
		fseek(maphandle,pos,SEEK_SET);
		CA_FarRead (maphandle,(memptr)mapheaderseg[mapnum],sizeof(maptype));
	}
	else
		MM_SetPurge ((memptr*)&(mapheaderseg[mapnum]),0);

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
		if (compressed<=BUFFERSIZE)
			source = bufferseg;
		else
		{
			MM_GetPtr(&bigbufferseg,compressed);
			MM_SetLock (&bigbufferseg,true);
			source = bigbufferseg;
		}

		CA_FarRead(maphandle,(byte far *)source,compressed);
#ifdef MAPHEADERLINKED
		//
		// unhuffman, then unRLEW
		// The huffman'd chunk has a two byte expanded length first
		// The resulting RLEW chunk also does, even though it's not really
		// needed
		//
		expanded = *source;
		source++;
		MM_GetPtr (&buffer2seg,expanded);
		CAL_CarmackExpand (source, (unsigned short *)buffer2seg,expanded);
		CA_RLEWexpand ((unsigned short *)buffer2seg+1,*dest,size,((mapfiletype _seg *)tinf)->RLEWtag);
		MM_FreePtr (&buffer2seg);

#else
		//
		// unRLEW, skipping expanded length
		//
		CA_RLEWexpand (source+1, *dest,size,
		((mapfiletype _seg *)tinf)->RLEWtag);
#endif

		if (compressed>BUFFERSIZE)
			MM_FreePtr(&bigbufferseg);
	}
}

//===========================================================================

/*
======================
=
= CA_UpLevel
=
= Goes up a bit level in the needed lists and clears it out.
= Everything is made purgable
=
======================
*/

void CA_UpLevel (void)
{
	if (ca_levelnum==7)
		Quit ("CA_UpLevel: Up past level 7!");

	ca_levelbit<<=1;
	ca_levelnum++;
}

//===========================================================================

/*
======================
=
= CA_DownLevel
=
= Goes down a bit level in the needed lists and recaches
= everything from the lower level
=
======================
*/

void CA_DownLevel (void)
{
	if (!ca_levelnum)
		Quit ("CA_DownLevel: Down past level 0!");
	ca_levelbit>>=1;
	ca_levelnum--;
	CA_CacheMarks(NULL);
}

//===========================================================================

/*
======================
=
= CA_ClearMarks
=
= Clears out all the marks at the current level
=
======================
*/

void CA_ClearMarks (void)
{
	int i;

	for (i=0;i<NUMCHUNKS;i++)
		grneeded[i]&=~ca_levelbit;
}


//===========================================================================

/*
======================
=
= CA_ClearAllMarks
=
= Clears out all the marks on all the levels
=
======================
*/

void CA_ClearAllMarks (void)
{
	memset (grneeded,0,sizeof(grneeded));
	ca_levelbit = 1;
	ca_levelnum = 0;
}


//===========================================================================

/*
======================
=
= CA_FreeGraphics
=
======================
*/

void CA_FreeGraphics (void)
{
	int	i;

	for (i=0;i<NUMCHUNKS;i++)
		if (grsegs[i])
			MM_SetPurge ((memptr*)&(grsegs[i]),3);
}


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

	CA_ClearMarks ();

//
// free cursor sprite and background save
//
	VW_FreeCursor ();

//
// free map headers and map planes
//
	for (i=0;i<NUMMAPS;i++)
		if (mapheaderseg[i])
			MM_SetPurge ((memptr*)&(mapheaderseg[i]),3);

	for (i=0;i<3;i++)
		if (mapsegs[i])
			MM_FreePtr ((memptr*)&(mapsegs[i]));

//
// free sounds
//
	for (i=0;i<NUMSNDCHUNKS;i++)
		if (audiosegs[i])
			MM_SetPurge ((memptr*)&(audiosegs[i]),3);

//
// free graphics
//
	CA_FreeGraphics ();
}


void CA_SetGrPurge (void)
{
	int i;

//
// free graphics
//
	for (i=0;i<NUMCHUNKS;i++)
		if (grsegs[i])
			MM_SetPurge ((memptr*)&(grsegs[i]),3);
}


//===========================================================================


/*
======================
=
= CAL_DialogDraw
=
======================
*/

#define NUMBARS	(17l*8)
#define BARSTEP	8

unsigned	thx,thy,lastx;
long		barx,barstep;

void	CAL_DialogDraw (char *title,unsigned numcache)
{
	unsigned	homex,homey,x;

	barstep = (NUMBARS<<16)/numcache;

//
// draw dialog window (masked tiles 12 - 20 are window borders)
//
	US_CenterWindow (20,8);
	homex = PrintX;
	homey = PrintY;

	US_CPrint ("Loading");
	fontcolor = F_SECONDCOLOR;
	US_CPrint (title);
	fontcolor = F_BLACK;

//
// draw thermometer bar
//
	thx = homex + 8;
	thy = homey + 32;
	VWB_DrawTile8(thx,thy,0);		// CAT3D numbers
	VWB_DrawTile8(thx,thy+8,3);
	VWB_DrawTile8(thx,thy+16,6);
	VWB_DrawTile8(thx+17*8,thy,2);
	VWB_DrawTile8(thx+17*8,thy+8,5);
	VWB_DrawTile8(thx+17*8,thy+16,8);
	for (x=thx+8;x<thx+17*8;x+=8)
	{
		VWB_DrawTile8(x,thy,1);
		VWB_DrawTile8(x,thy+8,4);
		VWB_DrawTile8(x,thy+16,7);
	}

	thx += 4;		// first line location
	thy += 5;
	barx = (long)thx<<16;
	lastx = thx;

	VW_UpdateScreen();
}


/*
======================
=
= CAL_DialogUpdate
=
======================
*/

void	CAL_DialogUpdate (void)
{
	unsigned	x,xh;

	barx+=barstep;
	xh = barx>>16;
	if (xh - lastx > BARSTEP)
	{
		for (x=lastx;x<=xh;x++)
			VWB_Vlin (thy,thy+13,x,14);
		lastx = xh;
		VW_UpdateScreen();
	}
}

/*
======================
=
= CAL_DialogFinish
=
======================
*/

void	CAL_DialogFinish (void)
{
	unsigned	x,xh;

	xh = thx + NUMBARS;
	for (x=lastx;x<=xh;x++)
		VWB_Vlin (thy,thy+13,x,14);
	VW_UpdateScreen();

}

//===========================================================================

/*
======================
=
= CA_CacheMarks
=
======================
*/
#define MAXEMPTYREAD	1024

void CA_CacheMarks (char *title)
{
	boolean dialog;
	int 	i,next,numcache;
	long	pos,endpos,nextpos,nextendpos,compressed;
	long	bufferstart,bufferend;	// file position of general buffer
	byte	far *source;
	memptr	bigbufferseg;

	dialog = (title!=NULL);

	numcache = 0;
//
// go through and make everything not needed purgable
//
	for (i=0;i<NUMCHUNKS;i++)
		if (grneeded[i]&ca_levelbit)
		{
			if (grsegs[i])					// its allready in memory, make
				MM_SetPurge((memptr*)&grsegs[i],0);	// sure it stays there!
			else
				numcache++;
		}
		else
		{
			if (grsegs[i])					// not needed, so make it purgeable
				MM_SetPurge((memptr*)&grsegs[i],3);
		}

	if (!numcache)			// nothing to cache!
		return;

	if (dialog)
	{
#ifdef PROFILE
		write(profilehandle,title,strlen(title));
		write(profilehandle,"\n",1);
#endif
		if (drawcachebox)
			drawcachebox(title,numcache);
	}

//
// go through and load in anything still needed
//
	bufferstart = bufferend = 0;		// nothing good in buffer now

	for (i=0;i<NUMCHUNKS;i++)
		if ( (grneeded[i]&ca_levelbit) && !grsegs[i])
		{
//
// update thermometer
//
			if (dialog && updatecachebox)
				updatecachebox ();

			pos = GRFILEPOS(i);
			if (pos<0)
				continue;

			next = i +1;
			while (GRFILEPOS(next) == -1)		// skip past any sparse tiles
				next++;

			compressed = GRFILEPOS(next)-pos;
			endpos = pos+compressed;

			if (compressed<=BUFFERSIZE)
			{
				if (bufferstart<=pos && bufferend>= endpos)
				{
				// data is allready in buffer
					source = (byte _seg *)bufferseg+(pos-bufferstart);
				}
				else
				{
				// load buffer with a new block from disk
				// try to get as many of the needed blocks in as possible
					while ( next < NUMCHUNKS )
					{
						while (next < NUMCHUNKS &&
						!(grneeded[next]&ca_levelbit && !grsegs[next]))
							next++;
						if (next == NUMCHUNKS)
							continue;

						nextpos = GRFILEPOS(next);
						while (GRFILEPOS(++next) == -1)	// skip past any sparse tiles
							;
						nextendpos = GRFILEPOS(next);
						if (nextpos - endpos <= MAXEMPTYREAD
						&& nextendpos-pos <= BUFFERSIZE)
							endpos = nextendpos;
						else
							next = NUMCHUNKS;			// read pos to posend
					}

					fseek(grhandle,pos,SEEK_SET);
					CA_FarRead(grhandle,bufferseg,endpos-pos);
					bufferstart = pos;
					bufferend = endpos;
					source = bufferseg;
				}
			}
			else
			{
			// big chunk, allocate temporary buffer
				MM_GetPtr(&bigbufferseg,compressed);
				if (mmerror)
					return;
				MM_SetLock (&bigbufferseg,true);
				fseek(grhandle,pos,SEEK_SET);
				CA_FarRead(grhandle,bigbufferseg,compressed);
				source = bigbufferseg;
			}

			CAL_ExpandGrChunk (i,source);
			if (mmerror)
				return;

			if (compressed>BUFFERSIZE)
				MM_FreePtr(&bigbufferseg);

		}

//
// finish up any thermometer remnants
//
		if (dialog && finishcachebox)
			finishcachebox();
}

