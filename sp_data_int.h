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


extern uint8_t *mapData, *grData, *auData;
extern long mapDataSize, grDataSize, auDataSize;

extern huffnode *grhuffman, *audiohuffman;
extern int32_t *GrChunksPos, *GrChunksSize, *AudioChunksPos, *AudioChunksSize;

uint8_t *SPD_ReadAndHuffExpand(uint8_t *Data, huffnode Dictionary[], long *Expanded);

void loadMapHeader(uint8_t *Data, int HeaderOffset, int Map, uint16_t RLEWTag);
void loadMapTexts(int Chunk, int Level);
void loadFont(int Chunk);
void loadTile(int Chunk, int NumTilesInChunk, int Width, int Height, int Masked);
void loadPic(int Chunk, int PicNum);
void loadMaskedPic(int Chunk, int PicNum);
void loadSprite(int Chunk, int SpriteNum);
pictabletype *loadPicTable(uint8_t *Data, long DataSize, int NumElements);
spritetabletype *loadSpriteTable(uint8_t *Data, long DataSize, int NumElements);

void SPD_CombinedLoader(int Chunk);


void SPD_DumpHexChar(const uint8_t *Buffer, long Size);
void SPD_DumpU8Hex(const uint8_t *Buffer, long Size);
uint8_t *SPD_ParseObj(const char *Filename, char ObjName[256], long *ObjSize);
void SPD_DumpDict(const char *Name, uint8_t *Buf, long Size);
void SPD_DumpLongArray(const char *Prefix, uint8_t *Buf, long Size, int LongSize);
void SPD_DumpMapfiletype(const char *Prefix, uint8_t *Buf, long Size);

#endif

