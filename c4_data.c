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

#include "sp_data_int.h"
#include "c4_def.h"

const char *GamespecificExtension = "ABS";
void *grsegs[NUMCHUNKS];

pictabletype	*pictable;
pictabletype	*picmtable;
spritetabletype *spritetable;

const int StartFont = STARTFONT;
const int StartTile8 = STARTTILE8;
const int StartTile8M = STARTTILE8M;


// EAST / WEST WALLS
//
int	walllight1[NUMFLOORS] = {0,
	W_WARP1EWPIC,
	W_NEMPICEWPIC,W_PENTAEWPIC,W_ALTER_LFPIC,W_ALTER_RTPIC,
	W_SUB1EWPIC,W_SUB2EWPIC,W_SUB3EWPIC,

	W_TORCH1PIC,W_TORCH2PIC,
	W_LSUB_STONEPIC,
	W_BLOODY_LSUB_STONEPIC,
	W_BREATH_LWALL1PIC,W_BREATH_LWALL2PIC,

	EXPWALL1PIC,EXPWALL2PIC,EXPWALL3PIC,

	W_WOOD_DOORWAYPIC,W_WOOD_DOORWAY_GLOWPIC,

	W_WATER1EWPIC,W_DRAIN1EWPIC,
	W_WATER2EWPIC,W_DRAIN2EWPIC,

	W_WOODEN_DOORPIC,W_WOOD_DOOREWPIC,W_METAL_DOORPIC,W_GLOW_DOORPIC,

	W_FINALEXITPIC,

	W_WATER_EXP1PIC,W_WATER_EXP2PIC,W_WATER_EXP3PIC,

	W_PRE_CHEATSPIC,W_CHEAT_WARPPIC,W_CHEAT_FREEZEPIC,W_SURFACE_PLAQPIC,

	W_WATER_GATEEW1PIC,
	WALL8LPIC,WALL9LPIC,WALL10DPIC,WALL11LPIC,WALL12LPIC,WALL13LPIC,WALL14LPIC,WALL15LPIC,
	WALL16LPIC,WALL17LPIC,W_WINDOWEWPIC,WALL19LPIC,WALL20LPIC,WALL21LPIC,
	WALL22LPIC,WALL23LPIC,WALL24LPIC,WALL25LPIC,WALL26LPIC,WALL27LPIC,
	WALL28LPIC,WALL29LPIC,WALL30LPIC,WALL31LPIC,
	W_BREATH_LWALL4PIC,W_BREATH_LWALL3PIC,
	MAGE_STATUEPIC,ZOMBIE_STATUEPIC,EYE_STATUEPIC,NEM_STATUEPIC,
	SKELETON_STATUEPIC,SPOOK_STATUEPIC,ORCH_STATUEPIC,
	};

// NORTH / SOUTH WALLS
//
int	walldark1[NUMFLOORS] = {0,
	W_WARP1NSPIC,
	W_NEMPICEWPIC,W_PENTANSPIC,1,1,
	W_SUB1NSPIC,W_SUB2NSPIC,W_SUB3NSPIC,

	W_TORCH1PIC,W_TORCH2PIC,
	W_DSUB_STONEPIC,
	W_BLOODY_DSUB_STONEPIC,
	W_BREATH_DWALL1PIC,W_BREATH_DWALL2PIC,

	EXPWALL1PIC,EXPWALL2PIC,EXPWALL3PIC,

	W_WOOD_DOORWAYPIC,W_WOOD_DOORWAY_GLOWPIC,

	W_WATER1NSPIC,W_DRAIN1NSPIC,
	W_WATER2NSPIC,W_DRAIN2NSPIC,
	W_WOODEN_DOORPIC,W_WOOD_DOORNSPIC,W_METAL_DOORPIC,W_GLOW_DOORPIC,

	W_FINALEXITPIC,

	W_WATER_EXP1PIC,W_WATER_EXP2PIC,W_WATER_EXP3PIC,

	W_CHEAT_GODPIC,W_CHEAT_ITEMSPIC,W_POST_CHEATPIC,W_SURFACE_PLAQPIC,

	W_WATER_GATENS1PIC,
	WALL8DPIC,WALL9DPIC,WALL10LPIC,WALL11DPIC,WALL12DPIC,WALL13DPIC,WALL14DPIC,WALL15DPIC,
	WALL16DPIC,WALL17DPIC,W_WINDOWNSPIC,WALL19DPIC,WALL20DPIC,WALL21DPIC,
	WALL22DPIC,WALL23DPIC,WALL24DPIC,WALL25DPIC,WALL26DPIC,WALL27DPIC,
	WALL28DPIC,WALL29DPIC,WALL30DPIC,WALL31DPIC,
	W_BREATH_DWALL4PIC,W_BREATH_DWALL3PIC,
	MAGE_STATUEPIC,ZOMBIE_STATUEPIC,EYE_STATUEPIC,NEM_STATUEPIC,
	SKELETON_STATUEPIC,SPOOK_STATUEPIC,ORCH_STATUEPIC,
	};

// EAST / WEST WALLS
//
int	walllight2[NUMFLOORS] = {0,
	W_WARP2EWPIC,
	W_NEMPICEWPIC,W_PENTAEWPIC,W_ALTER_LFPIC,W_ALTER_RTPIC,
	W_SUB1EWPIC,W_SUB2EWPIC,W_SUB3EWPIC,

	W_TORCH2PIC,W_TORCH1PIC,
	W_LSUB_STONEPIC,
	W_BLOODY_LSUB_STONEPIC,
	W_BREATH_LWALL2PIC,W_BREATH_LWALL1PIC,

	EXPWALL2PIC,EXPWALL1PIC,EXPWALL3PIC,

	W_WOOD_DOORWAYPIC,W_WOOD_DOORWAY_GLOWPIC,

	W_WATER2EWPIC,W_DRAIN2EWPIC,
	W_WATER1EWPIC,W_DRAIN1EWPIC,

	W_WOODEN_DOORPIC,W_WOOD_DOOREWPIC,W_METAL_DOORPIC,W_GLOW_DOORPIC,

	W_FINALEXITPIC,

	W_WATER_EXP2PIC,W_WATER_EXP1PIC,W_WATER_EXP1PIC,

	W_PRE_CHEATSPIC,W_CHEAT_WARPPIC,W_CHEAT_FREEZEPIC,W_SURFACE_PLAQPIC,

	W_WATER_GATEEW2PIC,
	WALL8LPIC,WALL9LPIC,WALL10DPIC,WALL11LPIC,WALL12LPIC,WALL13LPIC,WALL14LPIC,WALL15LPIC,
	WALL16LPIC,WALL17LPIC,W_WINDOWEWPIC,WALL19LPIC,WALL20LPIC,WALL21LPIC,
	WALL22LPIC,WALL23LPIC,WALL24LPIC,WALL25LPIC,WALL26LPIC,WALL27LPIC,
	WALL28LPIC,WALL29LPIC,WALL30LPIC,WALL31LPIC,
	W_BREATH_LWALL3PIC,W_BREATH_LWALL4PIC,
	MAGE_STATUEPIC,ZOMBIE_STATUEPIC,EYE_STATUEPIC,NEM_STATUEPIC,
	SKELETON_STATUEPIC,SPOOK_STATUEPIC,ORCH_STATUEPIC,
	};

// NORTH / SOUTH WALLS
//
int	walldark2[NUMFLOORS] = {0,
	W_WARP2NSPIC,
	W_NEMPICEWPIC,W_PENTANSPIC,1,1,
	W_SUB1NSPIC,W_SUB2NSPIC,W_SUB3NSPIC,

	W_TORCH2PIC,W_TORCH1PIC,
	W_DSUB_STONEPIC,
	W_BLOODY_DSUB_STONEPIC,
	W_BREATH_DWALL2PIC,W_BREATH_DWALL1PIC,

	EXPWALL2PIC,EXPWALL1PIC,EXPWALL3PIC,

	W_WOOD_DOORWAYPIC,W_WOOD_DOORWAY_GLOWPIC,

	W_WATER2NSPIC,W_DRAIN2NSPIC,
	W_WATER1NSPIC,W_DRAIN1NSPIC,

	W_WOODEN_DOORPIC,W_WOOD_DOORNSPIC,W_METAL_DOORPIC,W_GLOW_DOORPIC,

	W_FINALEXITPIC,

	W_WATER_EXP2PIC,W_WATER_EXP1PIC,W_WATER_EXP1PIC,

	W_CHEAT_GODPIC,W_CHEAT_ITEMSPIC,W_POST_CHEATPIC,W_SURFACE_PLAQPIC,

	W_WATER_GATENS2PIC,
	WALL8DPIC,WALL9DPIC,WALL10LPIC,WALL11DPIC,WALL12DPIC,WALL13DPIC,WALL14DPIC,WALL15DPIC,
	WALL16DPIC,WALL17DPIC,W_WINDOWNSPIC,WALL19DPIC,WALL20DPIC,WALL21DPIC,
	WALL22DPIC,WALL23DPIC,WALL24DPIC,WALL25DPIC,WALL26DPIC,WALL27DPIC,
	WALL28DPIC,WALL29DPIC,WALL30DPIC,WALL31DPIC,
	W_BREATH_DWALL3PIC,W_BREATH_DWALL4PIC,
	MAGE_STATUEPIC,ZOMBIE_STATUEPIC,EYE_STATUEPIC,NEM_STATUEPIC,
	SKELETON_STATUEPIC,SPOOK_STATUEPIC,ORCH_STATUEPIC,
	};


#define RLETAG 0xABCD
#include "c4_data.inc"

void SPD_SetupGameData() {
	C4AudioFile.Data = SPD_ReadFile("AUDIO.ABS", &C4AudioFile.Size);
	SPD_LoadALSamples(C4AudioChunks, STARTADLIBSOUNDS, NUMSOUNDS);
	SPD_LoadALMusic(C4AudioChunks, STARTMUSIC, LASTMUSIC);
	SPA_InitSamples(NUMSOUNDS, LASTMUSIC);

	C4GraphFile.Data = SPD_ReadFile("EGAGRAPH.ABS", &C4GraphFile.Size);
	memset(grsegs, 0, sizeof(void*)*NUMCHUNKS);

	pictable = SPD_RegisterPics(C4GraphChunks, STRUCTPIC, STARTPICS, NUMPICS, 0);
	picmtable = SPD_RegisterPics(C4GraphChunks, STRUCTPICM, STARTPICM, NUMPICM, 1);
	spritetable = SPD_RegisterSprites(C4GraphChunks, STRUCTSPRITE, STARTSPRITES, NUMSPRITES);

	SPD_RegisterTiles(C4GraphChunks,  8, 0, STARTTILE8,  NUMTILE8);
	SPD_RegisterTiles(C4GraphChunks,  8, 1, STARTTILE8M, NUMTILE8M);
	SPD_RegisterTiles(C4GraphChunks, 16, 0, STARTTILE16,  NUMTILE16);
	SPD_RegisterTiles(C4GraphChunks, 16, 1, STARTTILE16M, NUMTILE16M);
	SPD_RegisterTiles(C4GraphChunks, 32, 0, STARTTILE32,  NUMTILE32);
	SPD_RegisterTiles(C4GraphChunks, 32, 1, STARTTILE32M, NUMTILE32M);

	SPD_LoadFont(C4GraphChunks+STARTFONT);

	C4MapHeadFile.Data = SPD_ReadFile("GAMEMAPS.ABS", &C4MapHeadFile.Size);
	{
extern maptype mapheaderseg[30];
		memset(&mapheaderseg, 0, sizeof(mapheaderseg));
		int i;
		for (i = TOWNE_CEMETARY_MAP; i < 20/*LASTMAP*/; i++) {
			loadMapHeader(C4MapHeadFile.Data,C4MapHeadOffsets[i], i, C4MapHeadFile.RLEWTag);
			loadMapTexts(C4GraphChunks+LEVEL1TEXT+i-TOWNE_CEMETARY_MAP,i);
		}
	}
}

void SPD_CombinedLoader(int Chunk) {
	if (grsegs[Chunk] != NULL) {
		return;
	}
	ChunkDesc *grchunk = &C4GraphChunks[Chunk];
	if (grchunk->Pos < 0) {
		return;
	}
	assert(grchunk->Loader != NULL);
	(*grchunk->Loader)(grchunk);
}
/*

static void loadMapHeaders(uint8_t *Data) {
extern maptype mapheaderseg[30];
	memset(&mapheaderseg, 0, sizeof(mapheaderseg));
	int i;
	for (i = TOWNE_CEMETARY_MAP; i < LASTMAP; i++) {
		loadMapHeader(Data,C4_maphead_headeroffsets[i], i, C4_maphead_RLEWtag);
	}	
}

void SPD_SetupGameData() {
	grhuffman = C4_grhuffman;
	GrChunksPos = C4_GrChunksPos;
	GrChunksSize = C4_GrChunksSize;
	AudioChunksPos = C4_AudioChunksPos;
	AudioChunksSize = C4_AudioChunksSize;
	mapData = SPD_ReadFile("GAMEMAPS.ABS", &mapDataSize);
	grData = SPD_ReadFile("EGAGRAPH.ABS", &grDataSize);

	grsegs = malloc(sizeof(void*)*NUMCHUNKS);
	memset(grsegs, 0, sizeof(void*)*NUMCHUNKS);

	loadMapHeaders(mapData);

	uint8_t *auData=NULL;
	long auDataSize=0;
	auData = SPD_ReadFile("AUDIO.ABS", &auDataSize);
	SPD_LoadALSamples(auData, C4_audiohuffman, AudioChunksPos, STARTADLIBSOUNDS, NUMSOUNDS);
	SPD_LoadALMusic(auData, C4_audiohuffman, AudioChunksPos, STARTMUSIC, LASTMUSIC);
	SPA_InitSamples(NUMSOUNDS, LASTMUSIC);
	free(auData);

	pictable = loadPicTable(grData+GrChunksPos[STRUCTPIC], GrChunksSize[STRUCTPIC], NUMPICS);
	picmtable = loadPicTable(grData+GrChunksPos[STRUCTPICM], GrChunksSize[STRUCTPICM], NUMPICM);
	spritetable = loadSpriteTable(grData+GrChunksPos[STRUCTSPRITE], GrChunksSize[STRUCTSPRITE], NUMSPRITES);
}

void SPD_CombinedLoader(int Chunk) {
	if (Chunk >= STARTFONT && Chunk < STARTFONT+NUMFONT) {
		loadFont(Chunk);
	} else if (Chunk >= STARTPICS && Chunk < STARTPICS+NUMPICS) {
		loadPic(Chunk, Chunk-STARTPICS);
	} else if (Chunk >= STARTPICM && Chunk < STARTPICM+NUMPICM) {
		loadMaskedPic(Chunk, Chunk-STARTPICM);
	} else if (Chunk >= STARTSPRITES && Chunk < STARTSPRITES+NUMSPRITES) {
		loadSprite(Chunk, Chunk-STARTSPRITES);
	} else if (Chunk == STARTTILE8) {
		loadTile(STARTTILE8, NUMTILE8, 1, 8, false);
	} else if (Chunk == STARTTILE8M) {
		loadTile(STARTTILE8M, NUMTILE8M, 1, 8, true);
	} else if (Chunk >= STARTTILE16 && Chunk < STARTTILE16+NUMTILE16) {
		loadTile(Chunk, 1, 2, 16, false);
	} else if (Chunk >= STARTTILE16M && Chunk < STARTTILE16M+NUMTILE16M) {
		loadTile(Chunk, 1, 2, 16, true);
	} else if (Chunk >= STARTTILE32 && Chunk < STARTTILE32+NUMTILE32) {
		loadTile(Chunk, 1, 4, 32, false);
		assert(false);
	} else if (Chunk >= STARTTILE32M && Chunk < STARTTILE32M+NUMTILE32M) {
		loadTile(Chunk, 1, 4, 32, true);
		assert(false);
	} else if (Chunk >= LEVEL1TEXT && Chunk <= LEVEL20TEXT) {
		printf("MAP TEXTS\n");
		loadMapTexts(Chunk,Chunk-LEVEL1TEXT);
	} else {
		assert(false);
	}
}
*/

