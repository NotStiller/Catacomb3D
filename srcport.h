/* Catacomb 3-D SDL Port
 * Copyright (C) 2014 twitter.com/NotStiller
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

#ifndef COMPAT_H
#define COMPAT_H

#include <stdint.h>
#include <assert.h>

// the following defs are mostly accessed by the original sources
#define CASTAT(type, where) (*(type*)&(where))

typedef void* memptr;

void SP_Exit();
void SP_GameEnter();
void SP_GameLeave();
int  SP_StrafeOn();
void SP_PollEvents();
int SP_LastScan();
char SP_LastASCII();
int SP_Keyboard(int Key);
void SP_SetTimeCount(long Ticks);
long SP_TimeCount();

void SPA_Init();
void SPA_AudioCallback(void *userdata, uint8_t *stream, int len);
void SPA_InitSamples(int NumSamples, int NumMusic);
void SPA_RenderMusic(int SampleName, uint8_t *Data);
void SPA_RenderSample(int SampleName, uint8_t *Data);
void SPA_MusicOff();
void SPA_MusicOn();
void SP_StartMusic(int Musicname);
int SP_PlaySound(int SoundName);

void SPG_Init();
void SPG_SetPalette();

void SPD_SetupScaleWall(int Chunk);
void SPD_SetupScalePic(int Chunk);
void SPD_LoadGrChunk(int Chunk);
void SPD_SetupCatacomb3DData();
void SPG_DrawMaskedPic(uint8_t *Source, int ScrX, int ScrY);
void SPG_DrawMaskedPicSkip(uint8_t *Source, int ScrX, int ScrY, int LowerV, int UpperV);
void SPG_DrawPic(uint8_t *Source, int ScrX, int ScrY);
void SPG_DrawPicSkip(uint8_t *Source, int ScrX, int ScrY, int LowerV, int UpperV);
void SPG_FlipBuffer();

typedef struct
{
  uint16_t bit0,bit1;	// 0-255 is a character, > is a pointer to a node
} huffnode;

typedef	struct
{
	int32_t planestart[3];
	uint16_t planelength[3];
	uint16_t width,height;
	char name[16];

	char *texts[27];
	uint16_t *rawplanes[3];
	long rawplaneslength[3];
	uint16_t *mapsegs[3];
} maptype;

extern int loadedmap;
extern maptype mapheaderseg[30];



 
#endif

