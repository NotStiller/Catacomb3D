/* Catacomb 3-D SDL Port
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

#ifndef SP_DATA_INT_H
#define SP_DATA_INT_H

#include "id_heads.h"

//////////////
// for internal use only
/////

typedef struct
{
  uint16_t bit0,bit1;	// 0-255 is a character, > is a pointer to a node
} huffnode;

typedef struct {
	huffnode *Dict;
	uint16_t RLEWTag;
	long Size;
	uint8_t *Data;
} DataFile;

typedef struct ChunkDesc_s ChunkDesc;

typedef void (*ResLoader)(ChunkDesc *Chunk);

typedef struct ChunkDesc_s {
	int Index;
	long Pos, Size;
	DataFile *File;
	ResLoader Loader;
	long LoaderParm;
} ChunkDesc;


uint8_t *SPD_ReadAndHuffExpand(ChunkDesc *Chunk, long *Expanded);

void SPD_LoadALSamples(ChunkDesc *Chunks, int Start, int NumSounds);
void SPD_LoadALMusic(ChunkDesc *Chunks, int Start, int NumMusic);

void loadMapHeader(uint8_t *Data, int HeaderOffset, int Map, uint16_t RLEWTag);
void loadMapTexts(ChunkDesc *Chunk, int Level);
void SPD_LoadFont(ChunkDesc *Chunk);

void SPD_RegisterTiles(ChunkDesc *Chunks, int Size, int Masked, int Start, int Num);

pictabletype *SPD_RegisterPics(ChunkDesc *Chunks, int TableChunk, int FirstChunk, int NumElements, int Masked);
spritetabletype *SPD_RegisterSprites(ChunkDesc *Chunks, int TableChunk, int FirstChunk, int NumElements);

void SPD_CombinedLoader(int Chunk);


void SPD_DumpHexChar(const uint8_t *Buffer, long Size);
void SPD_DumpU8Hex(const uint8_t *Buffer, long Size);
uint8_t *SPD_ParseObj(const char *Filename, char ObjName[256], long *ObjSize);
void SPD_DumpDict(const char *Name, uint8_t *Buf, long Size);
void SPD_DumpLongArray(const char *Prefix, uint8_t *Buf, long Size, int LongSize);
void SPD_DumpMapfiletype(const char *Prefix, uint8_t *Buf, long Size);

#endif

