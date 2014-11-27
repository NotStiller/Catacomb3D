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

#include "c3_def.h"
#include "sp_data_int.h"

const char *GamespecificExtension = "C3D";


void *grsegs[NUMCHUNKS];

pictabletype	*pictable;
pictabletype	*picmtable;
spritetabletype *spritetable;

const int StartFont = STARTFONT;
const int StartTile8 = STARTTILE8;
const int StartTile8M = STARTTILE8M;


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

#define RLETAG 0xABCD
#include "c3_data.inc"

void SPD_SetupGameData() {
	C3AudioFile.Data = SPD_ReadFile("AUDIO.C3D", &C3AudioFile.Size);
	SPD_LoadALSamples(C3AudioChunks, STARTADLIBSOUNDS, NUMSOUNDS);
	SPD_LoadALMusic(C3AudioChunks, STARTMUSIC, LASTMUSIC);
	SPA_InitSamples(NUMSOUNDS, LASTMUSIC);

	C3GraphFile.Data = SPD_ReadFile("EGAGRAPH.C3D", &C3GraphFile.Size);
	memset(grsegs, 0, sizeof(void*)*NUMCHUNKS);

	pictable = SPD_RegisterPics(C3GraphChunks, STRUCTPIC, STARTPICS, NUMPICS, 0);
	picmtable = SPD_RegisterPics(C3GraphChunks, STRUCTPICM, STARTPICM, NUMPICM, 1);
	spritetable = SPD_RegisterSprites(C3GraphChunks, STRUCTSPRITE, STARTSPRITES, NUMSPRITES);

	SPD_RegisterTiles(C3GraphChunks,  8, 0, STARTTILE8,  NUMTILE8);
	SPD_RegisterTiles(C3GraphChunks,  8, 1, STARTTILE8M, NUMTILE8M);
	SPD_RegisterTiles(C3GraphChunks, 16, 0, STARTTILE16,  NUMTILE16);
	SPD_RegisterTiles(C3GraphChunks, 16, 1, STARTTILE16M, NUMTILE16M);
	SPD_RegisterTiles(C3GraphChunks, 32, 0, STARTTILE32,  NUMTILE32);
	SPD_RegisterTiles(C3GraphChunks, 32, 1, STARTTILE32M, NUMTILE32M);

	SPD_LoadFont(C3GraphChunks+STARTFONT);
	SPD_LoadFont(C3GraphChunks+STARTFONT+1);

	C3MapHeadFile.Data = SPD_ReadFile("GAMEMAPS.C3D", &C3MapHeadFile.Size);
	{
extern maptype mapheaderseg[30];
		memset(&mapheaderseg, 0, sizeof(mapheaderseg));
		int i;
		for (i = APPROACH_MAP; i <= NEMESISSLAIR_MAP; i++) {
			loadMapHeader(C3MapHeadFile.Data,C3MapHeadOffsets[i], i, C3MapHeadFile.RLEWTag);
			loadMapTexts(C3GraphChunks+LEVEL1TEXT+i-APPROACH_MAP,i);
		}
	}
}

void SPD_CombinedLoader(int Chunk) {
	if (grsegs[Chunk] != NULL) {
		return;
	}
	ChunkDesc *grchunk = &C3GraphChunks[Chunk];
	if (grchunk->Pos < 0) {
		return;
	}
	assert(grchunk->Loader != NULL);
	(*grchunk->Loader)(grchunk);
}


