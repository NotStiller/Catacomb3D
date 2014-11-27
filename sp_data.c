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

#include <stdlib.h>
#include "sp_data_int.h"
#include "id_heads.h"

maptype *curmap = NULL;
maptype mapheaderseg[30];

static int rawSoundsNum=0;
static SoundSample **rawSounds = NULL;
static int rawMusicNum=0;
static MusicSample **rawMusic = NULL;

// end of extracted data

uint8_t SPD_ReadU8(uint8_t **Buffer) {
	return *(*Buffer)++;
}

int8_t SPD_ReadS8(uint8_t **Buffer) {
	uint8_t v = *(*Buffer)++;
	return *(int8_t*)&v;
}

uint16_t SPD_ReadU16(uint8_t **Buffer) {
	uint16_t v = SPD_ReadU8(Buffer)+0x100*SPD_ReadU8(Buffer);
	return v;
}

int16_t SPD_ReadS16(uint8_t **Buffer) {
	uint16_t v = SPD_ReadU16(Buffer);
	return *(int16_t*)&v;
}

uint32_t SPD_ReadU32(uint8_t **Buffer) {
	return SPD_ReadU16(Buffer)+0x10000*SPD_ReadU16(Buffer);
/*	uint16_t lower = SPD_ReadU16(Buffer), upper = SPD_ReadU16(Buffer);
	int16_t slower = *(int16_t*)&lower, supper = *(int16_t*)&upper;
	uint32_t v1 = lower+0x10000*upper;
	uint32_t v2 = slower+0x10000*supper;
	assert(v1 == v2);
	return v1;*/
}

int32_t SPD_ReadS32(uint8_t **Buffer) {
	uint32_t v = SPD_ReadU32(Buffer);
	return *(int32_t*)&v;
}

void SPD_WriteU8(uint8_t **Buffer, uint8_t Value) {
	*(*Buffer)++ = Value;
}

void SPD_WriteS8(uint8_t **Buffer, int8_t Value) {
	SPD_WriteU8(Buffer, *(uint8_t*)&Value);
}

void SPD_WriteU16(uint8_t **Buffer, uint16_t Value) {
	SPD_WriteU8(Buffer, Value&0xFF);
	SPD_WriteU8(Buffer, (Value>>8)&0xFF);
}

void SPD_WriteS16(uint8_t **Buffer, int16_t Value) {
	SPD_WriteU16(Buffer, *(uint16_t*)&Value);
}

void SPD_WriteU32(uint8_t **Buffer, uint32_t Value) {
	SPD_WriteU16(Buffer, Value&0xFFFF);
	SPD_WriteU16(Buffer, (Value>>16)&0xFFFF);
}

void SPD_WriteS32(uint8_t **Buffer, int32_t Value) {
	SPD_WriteU32(Buffer, *(uint32_t*)&Value);
}

static void expandHuffman(uint8_t *Source, uint8_t *Dest,long length,huffnode *Hufftable) {
// rewrite of CAL_HuffExpand
	huffnode *headptr;
	headptr = Hufftable+254;	// head node is allways node 254

	huffnode *curhuffnode = headptr;
	int i, j;
	int written = 0;
	i = 0;
	while (written < length) {
		uint8_t srcbyte = Source[i++];
		for (j = 0; j < 8; j++) {
			unsigned short b = curhuffnode->bit0;
			if (srcbyte&1) {
				b = curhuffnode->bit1;
			}
			srcbyte = srcbyte>>1;
			if (b < 256) {
				Dest[written++] = b;
				curhuffnode = headptr;
				if (written == length) {
					break;
				}
			} else {
				assert(b-256 >= 0);
				curhuffnode = &Hufftable[b-256];
			}
		}
	}
}

static void expandCarmack (uint16_t *Source, uint16_t *Dest, long Length) {
// rewrite of CAL_CarmackExpand
	const uint8_t NEARTAG = 0xA7, FARTAG = 0xA8;
	uint16_t	ch,chhigh,count,offset;
	uint16_t	*copyptr, *inptr, *outptr;

	Length/=2;

	inptr = Source;
	outptr = Dest;

	while (Length>0)
	{
		ch = *inptr++;
		chhigh = ch>>8;
		if (chhigh == NEARTAG)
		{
			count = ch&0xff;
			if (!count) { // insert tag, otherwise data could not contain NEARTAG
				ch |= *(uint8_t *)inptr;
				inptr = (uint16_t*) ((uint8_t*)inptr+1);
				*outptr++ = ch;
				Length--;
			} else {
				offset = *(uint8_t*)inptr;
				inptr = (uint16_t*)((uint8_t*)inptr+1);
				copyptr = outptr - offset;
				Length -= count;
				while (count--) {
					*outptr++ = *copyptr++;
				}
			}
		} else if (chhigh == FARTAG) {
			count = ch&0xff; 
			if (!count) { // insert tag, otherwise data could not contain FARTAG
				ch |= *(uint8_t*)inptr;
				inptr = (uint16_t*)((uint8_t*)inptr+1);
				*outptr++ = ch;
				Length --;
			} else {
				offset = *inptr++;
				copyptr = Dest + offset;
				Length -= count;
				while (count--)
					*outptr++ = *copyptr++;
			}
		} else {
			*outptr++ = ch;
			Length --;
		}
	}
	assert(Length >= 0);
}

static void expandRLEW (uint16_t *Source, uint16_t *Dest,long Length, uint16_t RLEWTag) {
// rewrite of CA_RLEWexpand
	uint16_t *end = Dest + (Length)/2;
	uint16_t value, count, i;
	do
	{
		value = *Source++;
		if (value != RLEWTag)
			*Dest++=value;
		else
		{
			count = *Source++;
			value = *Source++;
			for (i=1;i<=count;i++) {
				*Dest++ = value;
			}
		}
	} while (Dest<end);
}


void SPD_ReadFromFile(FILE *Handle, uint8_t *Dest, long Length) {
	assert(fread(Dest, 1, Length, Handle) == Length);
}

void SPD_WriteToFile(FILE *Handle, uint8_t *Source, long Length) {
	assert(fwrite(Source, 1, Length, Handle) == Length);
}

uint8_t *SPD_ReadFile(const char *Name, long *Size) {
	assert(Size != NULL);
	FILE *handle;
	handle = fopen(Name, "r");
	assert(handle != NULL);
	uint8_t *data = SPD_ReadFile2(handle, Size);
	fclose(handle);

	return data;
}

uint8_t *SPD_ReadFile2(FILE *File, long *Size) {
	assert(Size != NULL);
	assert(File != NULL);
	size_t pos = ftell(File);
	fseek(File, 0, SEEK_END);
	*Size = ftell(File)-pos;
	uint8_t *data = malloc(*Size);
	fseek(File, pos, SEEK_SET);
	fread(data, 1, *Size, File);
	return data;
}

uint8_t *SPD_ReadAndHuffExpand(ChunkDesc *Chunk, long *Expanded) {	
	uint8_t *data = Chunk->File->Data+Chunk->Pos;
	assert(data != NULL);
	int32_t explen = SPD_ReadS32(&data);
	if (Expanded) {
		*Expanded = explen;
	}
	memptr buffer = malloc(explen);
	assert(buffer != NULL);
	expandHuffman (data, buffer,explen,Chunk->File->Dict);
	return buffer;
}

static void deplaneRaw (memptr Source, int Width, int Height, uint8_t *Buffer) {
	uint8_t *plane0,*plane1,*plane2,*plane3;
	plane0 = Source;
	plane1 = plane0 + Width*Height;
	plane2 = plane1 + Width*Height;
	plane3 = plane2 + Width*Height;

	int x,y;
	for (y=0;y<Height;y++)
	{
		for (x=0;x<Width;x++)
		{
			uint8_t by0,by1,by2,by3;
			by0 = *plane0++;
			by1 = *plane1++;
			by2 = *plane2++;
			by3 = *plane3++;

			int b;
			for (b=0;b<8;b++)
			{
				Buffer[7-b] = ((by3&1)<<3) + ((by2&1)<<2) + ((by1&1)<<1) + (by0&1);
				by3 >>= 1;
				by2 >>= 1;
				by1 >>= 1;
				by0 >>= 1;
			}
			Buffer += 8;
		}
	}
}

static void deplaneMaskedRaw(memptr Source, int Width, int Height, uint8_t *Buffer) {
	uint8_t *plane0,*plane1,*plane2,*plane3,*planeM;
	planeM = Source;
	plane0 = planeM + Width*Height;
	plane1 = plane0 + Width*Height;
	plane2 = plane1 + Width*Height;
	plane3 = plane2 + Width*Height;

	int x,y;
	for (y=0;y<Height;y++)
	{
		for (x=0;x<Width;x++)
		{
			uint8_t byM,by0,by1,by2,by3;
			byM = *planeM++;
			by0 = *plane0++;
			by1 = *plane1++;
			by2 = *plane2++;
			by3 = *plane3++;

			int b;
			for (b=0;b<8;b++)
			{
				if (byM&1) {
					Buffer[7-b] = 0xFF;
				} else {
					Buffer[7-b] = ((by3&1)<<3) + ((by2&1)<<2) + ((by1&1)<<1) + (by0&1);
				}				
				by3 >>= 1;
				by2 >>= 1;
				by1 >>= 1;
				by0 >>= 1;
				byM >>= 1;
			}
			Buffer += 8;
		}
	}
}


SoundSample *SPD_GetSound(int SampleName) {
	assert(SampleName >= 0);
	assert(SampleName < rawSoundsNum);
	return rawSounds[SampleName];
}

MusicSample *SPD_GetMusic(int SampleName) {
	assert(SampleName >= 0);
	assert(SampleName < rawMusicNum);
	return rawMusic[SampleName];
}


SoundSample *spd_LoadALSound(uint8_t *Data, long Size) {
	uint8_t *p = Data;
	uint32_t len = SPD_ReadU32(&p);
	assert(Size == 1+23+len);
	SoundSample *sample = (SoundSample*)malloc(sizeof(SoundSample)+len);
	sample->Length = len;
	sample->Priority = SPD_ReadS16(&p);
	printf("priority %i\n", (int)sample->Priority);
	sample->mChar = SPD_ReadU8(&p);
	sample->cChar = SPD_ReadU8(&p);
	sample->mScale = SPD_ReadU8(&p);
	sample->cScale = SPD_ReadU8(&p);
	sample->mAttack = SPD_ReadU8(&p);
	sample->cAttack = SPD_ReadU8(&p);
	sample->mSus = SPD_ReadU8(&p);
	sample->cSus = SPD_ReadU8(&p);
	sample->mWave = SPD_ReadU8(&p);
	sample->cWave = SPD_ReadU8(&p);
	sample->nConn = SPD_ReadU8(&p);
	sample->Unused[0] = SPD_ReadU8(&p);
	sample->Unused[1] = SPD_ReadU8(&p);
	sample->Unused[2] = SPD_ReadU8(&p);
	sample->Unused[3] = SPD_ReadU8(&p);
	sample->Unused[4] = SPD_ReadU8(&p);
	sample->Block = SPD_ReadU8(&p);
	sample->Data = (uint8_t*)sample+sizeof(SoundSample);
	int i;
	for (i = 0; i < sample->Length; i++) {
		sample->Data[i] = SPD_ReadU8(&p);
	}
	return sample;
}



MusicSample *spd_LoadALMusic(uint8_t *Data, long DataSize) {
	uint8_t *p = Data;
	
	int count = SPD_ReadU16(&p);
	assert(DataSize == 2+count);
	assert((count&3) == 0);
	count /= 4;

	MusicSample *sample = (MusicSample*)malloc(sizeof(MusicSample)+sizeof(uint16_t)*2*count);
	sample->Count = count;
	sample->Data = (uint16_t*)((uint8_t*)sample+sizeof(MusicSample));

	int i;
	for (i = 0; i < sample->Count; i++) {
		sample->Data[2*i+0] = SPD_ReadU16(&p);
		sample->Data[2*i+1] = SPD_ReadU16(&p);
	}
	return sample;
}


void SPD_LoadALSamples(ChunkDesc *Chunks, int Start, int NumSounds) {
	rawSoundsNum = NumSounds;
	rawSounds = malloc(sizeof(SoundSample*)*NumSounds);

	int i;
	for (i = 0; i < NumSounds; i++) {
		long size;
		memptr buffer;
		buffer = SPD_ReadAndHuffExpand(&Chunks[Start+i], &size);
		assert(buffer != NULL);
		rawSounds[i] = spd_LoadALSound(buffer, size);
		free(buffer);
	}
}

void SPD_LoadALMusic(ChunkDesc *Chunks, int Start, int NumMusic) {
	rawMusicNum = NumMusic;
	rawMusic = malloc(sizeof(MusicSample*)*NumMusic);

	int i;
	for (i = 0; i < NumMusic; i++) {
		long size;
		memptr buffer = SPD_ReadAndHuffExpand(&Chunks[Start+i], &size);
		rawMusic[i] = spd_LoadALMusic(buffer, size);
		free(buffer);
	}
}



static void deplanePic (memptr Source, int Width, int Height, boolean Masked, memptr *Destination)
{
	MM_GetPtr(Destination, 8*Width*Height+2*sizeof(uint32_t));
	((uint32_t*)*Destination)[0] = 8*Width;
	((uint32_t*)*Destination)[1] = Height;
	if (Masked) {
		deplaneMaskedRaw(Source, Width, Height, *Destination+2*sizeof(uint32_t));
	} else {
		deplaneRaw(Source, Width, Height, *Destination+2*sizeof(uint32_t));
	}
}

void loadPic(ChunkDesc *Chunk) {
	memptr buffer = SPD_ReadAndHuffExpand(Chunk, NULL);
	deplanePic(buffer, pictable[Chunk->LoaderParm].width, pictable[Chunk->LoaderParm].height, 0, &grsegs[Chunk->Index]);
	free(buffer);
}

void loadMaskedPic(ChunkDesc *Chunk) {
	memptr buffer = SPD_ReadAndHuffExpand(Chunk, NULL);
	deplanePic(buffer, picmtable[Chunk->LoaderParm].width, picmtable[Chunk->LoaderParm].height, 1, &grsegs[Chunk->Index]);
	free(buffer);
}

pictabletype *SPD_RegisterPics(ChunkDesc *Chunks, int TableChunk, int FirstChunk, int NumElements, int Masked) {
	memptr buffer = SPD_ReadAndHuffExpand(&Chunks[TableChunk], NULL);
	pictabletype *table = malloc(sizeof(pictabletype)*NumElements);
	int i;
	uint8_t *p = buffer;
	for (i = 0; i < NumElements; i++) {
		table[i].width = SPD_ReadS16(&p);
		table[i].height = SPD_ReadS16(&p);

		assert(Chunks[FirstChunk+i].Loader == NULL);
		Chunks[FirstChunk+i].Loader = Masked?loadMaskedPic:loadPic;
		Chunks[FirstChunk+i].LoaderParm = i;
	}
	free(buffer);
	return table;
}

void loadSprite(ChunkDesc *Chunk) {
// This is a crippled version of the sprite loading function, but we need no bit
// shifting functionality anyway. Now sprites are just replaced by masked pics,
// which they really are.
	long size;
	memptr buffer = SPD_ReadAndHuffExpand(Chunk, &size);
	spritetabletype *spr = &spritetable[Chunk->LoaderParm];
	if (spr->orgx != 0 || spr->orgy != 0) {
		printf("Warning: ignoring sprite parameters !\n");
	}
	assert(spr->shifts == 4);
	deplanePic(buffer, spr->width, spr->height, 1, &grsegs[Chunk->Index]);
	free(buffer);
}

spritetabletype *SPD_RegisterSprites(ChunkDesc *Chunks, int TableChunk, int FirstChunk, int NumElements) {
	memptr buffer = SPD_ReadAndHuffExpand(&Chunks[TableChunk], NULL);

	spritetabletype *table = malloc(sizeof(spritetabletype)*NumElements);
	int i;
	uint8_t *p = buffer;
	for (i = 0; i < NumElements; i++) {
		table[i].width = SPD_ReadS16(&p);
		table[i].height = SPD_ReadS16(&p);
		table[i].orgx = SPD_ReadS16(&p);
		table[i].orgy = SPD_ReadS16(&p);
		table[i].xl = SPD_ReadS16(&p);
		table[i].yl = SPD_ReadS16(&p);
		table[i].xh = SPD_ReadS16(&p);
		table[i].yh = SPD_ReadS16(&p);
		table[i].shifts = SPD_ReadS16(&p);

		assert(Chunks[FirstChunk+i].Loader == NULL);
		Chunks[FirstChunk+i].Loader = loadSprite;
		Chunks[FirstChunk+i].LoaderParm = i;
	}
	free(buffer);
	return table;
}

void loadTile(ChunkDesc *Chunk, int NumTilesInChunk, int Width, int Masked) {
	assert(grsegs[Chunk->Index] == NULL);
	
	long pos = Chunk->Pos;
	assert(pos >= 0);
	
	int Height = 8*Width;
	int srcSpriteSize = (Masked?5:4)*Width*Height,
		dstSpriteSize = 8*Width*Height;
	uint8_t *buffer = malloc(NumTilesInChunk*srcSpriteSize);
	expandHuffman(Chunk->File->Data+pos,buffer,NumTilesInChunk*srcSpriteSize,Chunk->File->Dict);
	MM_GetPtr (&grsegs[Chunk->Index],NumTilesInChunk*dstSpriteSize);
	int i;
	for (i = 0; i < NumTilesInChunk; i++) {
		if (!Masked) {
			deplaneRaw(buffer+i*srcSpriteSize, Width, Height, (uint8_t*)grsegs[Chunk->Index]+i*dstSpriteSize);
		} else {
			deplaneMaskedRaw(buffer+i*srcSpriteSize, Width, Height, (uint8_t*)grsegs[Chunk->Index]+i*dstSpriteSize);
		}
	}
	free(buffer);
}

static void loadAnyTile(ChunkDesc *Chunk) {
	int parm = Chunk->LoaderParm;
	loadTile(Chunk, 1, (parm&6), parm&1);
}

void SPD_RegisterTiles(ChunkDesc *Chunks, int Size, int Masked, int Start, int Num) {
	if (Size == 8) {
		loadTile(Chunks+Start, Num, 1, Masked);
	} else if (Size == 16 || Size == 32) {
		int i;
		for (i = 0; i < Num; i++) {
			assert(Chunks[Start+i].Loader == NULL);
			Chunks[Start+i].Loader = loadAnyTile;
			Chunks[Start+i].LoaderParm = (Masked?1:0)+Size/8;
		}
	} else {
		assert(false);
	}
}


void SPD_LoadFont(ChunkDesc *Chunk) {
	long explen;
	memptr buffer = SPD_ReadAndHuffExpand(Chunk, &explen);
	int datalen = explen-3*256-2;
	uint8_t *p = (uint8_t*)buffer;
	MM_GetPtr(&grsegs[Chunk->Index], datalen+sizeof(fontstruct));
	fontstruct *font = grsegs[Chunk->Index];
	font->height = SPD_ReadS16(&p);
	int i;
	for (i = 0; i < 256; i++) {
		font->location[i] = SPD_ReadS16(&p)-3*256-2+sizeof(fontstruct);
	}
	for (i = 0; i < 256; i++) {
		uint8_t w = SPD_ReadU8(&p);
		assert(w < 128);
		font->width[i] = (char)w;
	}
	memcpy((uint8_t*)font+sizeof(fontstruct), p, datalen);
	free(buffer);
}

void loadMapTexts(ChunkDesc *Chunk, int Level) {
	long len=0;
	memptr buffer = SPD_ReadAndHuffExpand(Chunk, &len);
	MM_GetPtr(&grsegs[Chunk->Index], len); 
	memcpy(grsegs[Chunk->Index], buffer, len);
	free(buffer);

	int     i;
	char *p = grsegs[Chunk->Index];
	mapheaderseg[Level].texts[0] = NULL;
	for (i=1;i<27;i++)
	{
		while (*p != '\n')
		{
			if (*p == '\r')
				*p = 0;
			p++;
		}
		p++;
		mapheaderseg[Level].texts[i] = p;
	}
}


void loadMapHeader(uint8_t *Data, int HeaderOffset, int Map, uint16_t RLEWTag) {
	int mapnum = Map;

// load map header
	{
		maptype *map = &mapheaderseg[mapnum];
		uint8_t *p = Data+HeaderOffset;
		map->planestart[0] = SPD_ReadS32(&p);
		map->planestart[1] = SPD_ReadS32(&p);
		map->planestart[2] = SPD_ReadS32(&p);
		map->planelength[0] = SPD_ReadU16(&p);
		map->planelength[1] = SPD_ReadU16(&p);
		map->planelength[2] = SPD_ReadU16(&p);
		map->width = SPD_ReadU16(&p);
		map->height = SPD_ReadU16(&p);
		int i;
		for (i = 0; i < 16; i++) {
			map->name[i] = SPD_ReadS8(&p);
		}
	}

	int size = mapheaderseg[mapnum].width * mapheaderseg[mapnum].height * 2;

	int plane;
	for (plane = 0; plane<MAPPLANES; plane++)
	{
		mapheaderseg[mapnum].rawplaneslength[plane] = 0;
		long pos = mapheaderseg[mapnum].planestart[plane];
		long compressed = mapheaderseg[mapnum].planelength[plane];		
		if (!compressed)
			continue;		// the plane is not used in this game

		memptr *dest = (memptr*)&mapheaderseg[mapnum].rawplanes[plane];
		MM_GetPtr(dest,size);
		mapheaderseg[mapnum].rawplaneslength[plane] = size;

		int16_t *source = (int16_t*)(Data+pos);
		long expanded = *source;
		source++;
		char *buffer = (char*)malloc(expanded);

		expandCarmack (source, (uint16_t*)buffer,expanded);
		expandRLEW ((uint16_t*)buffer+1,*dest,size,RLEWTag);
		free(buffer);
	}
	if (0) { // hack to find out if freeze time exists. It does !
		uint16_t *mapseg = (uint16_t*)mapheaderseg[mapnum].rawplanes[2];
		int i;
		int width = mapheaderseg[mapnum].width;
		int height = mapheaderseg[mapnum].height;
		printf("map %i plane 2 length %i width %i height %i prod %i\n", mapnum, size, width, height, width*height*2-size);
		int size = mapheaderseg[mapnum].rawplaneslength[2];
		for (i = 0; i < size/2; i++) {
			int tile = mapseg[i]&0xFF;
//			if (tile) printf("%4i", tile);
			if (tile == 57) {
				printf("FOUND SMTH at %i,%i !\n", i%width, i/width);
			}
		}
		printf("\n");
	}
	int i;
	for (i = 0; i < 27; i++) {
		mapheaderseg[mapnum].texts[i] = NULL;
	}
}

void SPD_LoadGrChunk(int Chunk) {
	if (grsegs[Chunk]) {
		return;
	}
	SPD_CombinedLoader(Chunk);
}

void SPD_SetupScaleWall(int Chunk, int PicNum, int ScaleWallNum) {
	SPD_LoadGrChunk(Chunk);
#warning IS THIS EVEN NECESSARY ?
//	loadPic(Chunk, PicNum);
}

void SPD_SetupScalePic(int Chunk, int PicNum, int ScalePicNum) {
	SPD_LoadGrChunk(Chunk);
#warning IS THIS EVEN NECESSARY ?
//	loadPic(Chunk, PicNum);
}


void SPD_LoadMap(int Chunk, int MapNum) {
	curmap = &mapheaderseg[MapNum];
}

void SPD_DumpHexChar(const uint8_t *Buffer, long Size) {
	long i;
	for (i = 0; i < Size; i += 8) {
		int j;
		for (j = 0; j < 8 && i+j < Size; j++) {
			printf("%02x, ", Buffer[i+j]);			
		}
		for (; j < 8; j++) {
			printf("    ");
		}
		printf("  //  ");
		for (j = 0; j < 8 && i+j < Size; j++) {
			uint8_t c = Buffer[i+j];
			printf("%c", isprint(c)?*(char*)&c:'.');
		}
		printf("\n");
	}
}

void SPD_DumpU8Hex(const uint8_t *Buffer, long Size) {
	long i;
	for (i = 0; i < Size; i++) {
		printf("0x%02x, ", Buffer[i]);
		if (!((i+1)&7)) {
			printf("\n");
		}
	}
}



