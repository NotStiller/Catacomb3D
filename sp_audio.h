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

#ifndef SP_AUDIO_H
#define SP_AUDIO_H

typedef enum {SND_OFF=0, SND_ADLIB=1} SoundSource;

typedef struct {
	uint32_t Length, 
			Priority;
	uint8_t mChar, cChar,
			mScale, cScale,
			mAttack, cAttack,
			mSus, cSus,
			mWave, cWave,
			nConn,
			Unused[5],
			Block;
	uint8_t *Data;
} SoundSample;

typedef struct {
	uint16_t Count;
	uint16_t *Data;
} MusicSample;

// used by game code
int SPA_IsAnySoundPlaying(void);
int SPA_GetSoundSource(void);
int SPA_GetMusicSource(void);
void SPA_MusicOff(void);
void SPA_MusicOn(void);
int SPA_PlaySound(int SoundName);
void SPA_SetSoundSource(SoundSource Source);
void SPA_SetMusicSource(SoundSource Source);
void SPA_StartMusic(int Musicname);
void SPA_WaitUntilSoundIsDone(void);

// used internally
void SPA_Init(void);
void SPA_InitSamples(int NumSamples, int NumMusic);
void SPA_RenderMusic(int SampleName, MusicSample *Sample);
void SPA_RenderSample(int SampleName, SoundSample *Sample);

#endif

