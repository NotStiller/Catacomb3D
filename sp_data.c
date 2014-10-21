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
#include "srcport.h"
#include "sp_data_int.h"
#include "id_heads.h"

void **grsegs;

int loadedmap=-1;
maptype mapheaderseg[30];

uint8_t *mapData=NULL;
long mapDataSize=0;

uint8_t *grData=NULL;
long grDataSize=0;

uint8_t *auData=NULL;
long auDataSize=0;


huffnode *grhuffman = NULL, *audiohuffman = NULL;
int32_t *GrChunksPos;
int32_t *GrChunksSize;
int32_t *AudioChunksPos;
int32_t *AudioChunksSize;

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

uint16_t SPD_ReadU8or16(uint8_t **Buffer) { // these are not needed for the Catacomb files, but for .OBJ files
	uint16_t v = SPD_ReadU8(Buffer);
	if (v&0x80) {
		int8_t b=SPD_ReadU8(Buffer);
		v = (v&0x7F)<<8;
		v += b;
	}
	return v;
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

uint8_t *SPD_ReadAndHuffExpand(uint8_t *Data, huffnode Dictionary[], long *Expanded) {	
	int32_t explen = SPD_ReadS32(&Data);
	if (Expanded) {
		*Expanded = explen;
	}
	memptr buffer = malloc(explen);
	expandHuffman (Data, buffer,explen,Dictionary);
	assert(buffer != NULL);
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



pictabletype *loadPicTable(uint8_t *Data, long DataSize, int NumElements) {
	memptr buffer = SPD_ReadAndHuffExpand(Data, grhuffman, NULL);
	pictabletype *table = malloc(sizeof(pictabletype)*NumElements);
	int i;
	uint8_t *p = buffer;
	for (i = 0; i < NumElements; i++) {
		table[i].width = SPD_ReadS16(&p);
		table[i].height = SPD_ReadS16(&p);
	}
	free(buffer);
	return table;
}


spritetabletype *loadSpriteTable(uint8_t *Data, long DataSize, int NumElements) {
	memptr buffer = SPD_ReadAndHuffExpand(Data, grhuffman, NULL);

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
	}
	free(buffer);
	return table;
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

void loadPic(int Chunk, int PicNum) {
	memptr buffer = SPD_ReadAndHuffExpand(grData+GrChunksPos[Chunk], grhuffman, NULL);
	deplanePic(buffer, pictable[PicNum].width, pictable[PicNum].height, 0, &grsegs[Chunk]);
	free(buffer);
}

void loadMaskedPic(int Chunk, int PicNum) {
	memptr buffer = SPD_ReadAndHuffExpand(grData+GrChunksPos[Chunk], grhuffman, NULL);
	deplanePic(buffer, picmtable[PicNum].width, picmtable[PicNum].height, 1, &grsegs[Chunk]);
	free(buffer);
}

void loadSprite(int Chunk, int SpriteNum) {
// This is a crippled version of the sprite loading function, but we need no bit
// shifting functionality anyway. Now sprites are just replaced by masked pics,
// which they really are.
	memptr buffer = SPD_ReadAndHuffExpand(grData+GrChunksPos[Chunk], grhuffman, NULL);
	spritetabletype *spr = &spritetable[SpriteNum];
	assert(spr->orgx == 0);
	assert(spr->orgy == 0);
	assert(spr->shifts == 4);
	deplanePic(buffer, spr->width, spr->height, 1, &grsegs[Chunk]);
	free(buffer);
}

void loadTile(int Chunk, int NumTilesInChunk, int Width, int Height, int Masked) {
	assert(grsegs[Chunk] == NULL);
	
	long pos = GrChunksPos[Chunk];
	assert(pos >= 0);
	
	int srcSpriteSize = (Masked?5:4)*Width*Height,
		dstSpriteSize = 8*Width*Height;
	uint8_t *buffer = malloc(NumTilesInChunk*srcSpriteSize);
	expandHuffman(grData+pos,buffer,NumTilesInChunk*srcSpriteSize,grhuffman);
	MM_GetPtr (&grsegs[Chunk],NumTilesInChunk*dstSpriteSize);
	int i;
	for (i = 0; i < NumTilesInChunk; i++) {
		if (!Masked) {
			deplaneRaw(buffer+i*srcSpriteSize, Width, Height, (uint8_t*)grsegs[Chunk]+i*dstSpriteSize);
		} else {
			deplaneMaskedRaw(buffer+i*srcSpriteSize, Width, Height, (uint8_t*)grsegs[Chunk]+i*dstSpriteSize);
		}
	}
	free(buffer);
}

void loadFont(int Chunk) {
	long explen;
	memptr buffer = SPD_ReadAndHuffExpand(grData+GrChunksPos[Chunk], grhuffman, &explen);
	int datalen = explen-3*256-2;
	uint8_t *p = (uint8_t*)buffer;
	MM_GetPtr(&grsegs[Chunk], datalen+sizeof(fontstruct));
	fontstruct *font = grsegs[Chunk];
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

void loadMapTexts(int Chunk, int Level) {
	long len=0;
	memptr buffer = SPD_ReadAndHuffExpand(grData+GrChunksPos[Chunk], grhuffman, &len);
	MM_GetPtr(&grsegs[Chunk], len); 
	memcpy(grsegs[Chunk], buffer, len);
	free(buffer);

	int     i;
	char *p = grsegs[Chunk];
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
	int i;
	for (i = 0; i < 27; i++) {
		mapheaderseg[mapnum].texts[i] = NULL;
	}
}

void SPD_SetupScaleWall(int Chunk, int PicNum, int ScaleWallNum) {
	loadPic(Chunk, PicNum);
}

void SPD_SetupScalePic(int Chunk, int PicNum, int ScalePicNum) {
	loadPic(Chunk, PicNum);
}


void SPD_LoadMap(int ChunkNum, int MapNum) {
	loadedmap = MapNum;

// load the planes in
// If a plane's pointer still exists it will be overwritten (levels are
// allways reloaded, never cached)

	int size = mapheaderseg[MapNum].width * mapheaderseg[MapNum].height * 2;

	loadMapTexts(ChunkNum,MapNum);
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

uint8_t *SPD_ParseObj(const char *Filename, char ObjName[256], long *ObjSize) {
	FILE *f = fopen(Filename, "rb");
	assert(f != NULL);
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);
	uint8_t contents[size];
	assert(fread(contents, 1, size, f) == size);
	fclose(f);

//	printf("Read file %s of size %i\n", Filename, size);
	uint8_t *p = contents;
	int nameSet = 0;
	uint8_t *obj = NULL;
	long objSize = 0;
	while (1) {
		uint8_t type = SPD_ReadU8(&p);
		uint16_t length = SPD_ReadU16(&p);
		uint8_t *nextp = p+length;
		if (type == 0x90) { // name
			SPD_ReadU8or16(&p);
			if (SPD_ReadU8or16(&p) == 0) {
				SPD_ReadU16(&p);
			}
			uint8_t len = SPD_ReadU8(&p);
			memcpy(ObjName, p, len);
			ObjName[len] = 0;
			assert(!nameSet);
			nameSet = 1;
		} else if (type == 0xA0) { // contents
			length -= 4;
			if (SPD_ReadU8or16(&p)>=0x80) {
				length--;
			}
			SPD_ReadU16(&p);
			obj = realloc(obj, objSize+length);
			assert(obj != NULL);
			memcpy(obj+objSize, p, length);
			objSize += length;
		} else if (0) {
			printf("Record of type 0x%02x and length %i\n", (int)type, length);
			SPD_DumpHexChar(p, length);
		}
		p = nextp;
		if (p-contents >= size) {
			assert(p-contents == size);
			break;
		}
	}
	*ObjSize = objSize;
/*	printf("uint8_t %s[%i] = {\n", ObjName, objSize);
	SPD_DumpHexChar(obj, objSize);
	printf("};\n");*/
	return obj;
}

void SPD_DumpDict(const char *Name, uint8_t *Buf, long Size) {
	assert(Size == 1024);
	printf("huffnode %s[256] = {\n", Name);
	int i;
	uint8_t *p = Buf;
	for (i = 0; i < 256; i++) {
		printf("{%3i, %3i}, ", (int)SPD_ReadU16(&p), (int)SPD_ReadU16(&p));
		if (!(3&(i+1))) {
			printf("\n");
		}
	}
	printf("};\n");	
}

void SPD_DumpLongArray(const char *Prefix, uint8_t *Buf, long Size, int LongSize) {
	assert(!(Size%LongSize));
	assert(LongSize == 3 || LongSize == 4);
	int num=Size/LongSize;
	int32_t pos[num], size[num];
	
	int i, j, last=-1;
	uint8_t *p = Buf;
	for (i = 0; i < num; i++) {
		int32_t v=0;
		if (LongSize == 3) {
			v = SPD_ReadU8(&p)+0x100*(SPD_ReadU8(&p)+0x100*SPD_ReadU8(&p));
			if (v==0xFFFFFF) {
				v = -1;
			}
		} else {
			v = SPD_ReadU8(&p)+0x100*(SPD_ReadU8(&p)+0x100*(SPD_ReadU8(&p)+0x100*SPD_ReadU8(&p)));
			if (v==0xFFFFFFFF) {
				v = -1;
			}
		}
		pos[i] = v;
		size[i] = 0;
		if (v >= 0) {
			if (last >= 0) {
				size[last] = pos[i]-pos[last];
//				assert(size[last] >= 0);
			}
			last = i;
		}
	}
	printf("int32_t %sChunksPos[%i] = {\n", Prefix, num);
	for (i = 0; i < num; i++) {
		printf("%10i, ", pos[i]);
		if (!(3&(i+1))) {
			printf("\n");
		}
	}
	printf("};\n");
	printf("int32_t %sChunksSize[%i] = {\n", Prefix, num);
	for (i = 0; i < num; i++) {
		printf("%10i, ", size[i]);
		if (!(3&(i+1))) {
			printf("\n");
		}
	}
	printf("};\n");	
}

void SPD_DumpMapfiletype(const char *Prefix, uint8_t *Buf, long Size) {
	assert(Size >= 402);
	uint8_t *p = Buf;

	printf("uint16_t %smaphead_RLEWtag = %i;\n", Prefix, SPD_ReadU16(&p));

	printf("int32_t %smaphead_headeroffsets[100] = {\n", Prefix);
	int i;
	for (i = 0; i < 100; i++) {
		printf("%10i, ", SPD_ReadS32(&p));
		if (!(3&(i+1))) {
			printf("\n");
		}
	}
	printf("};\n");
}



