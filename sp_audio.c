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

#include <SDL/SDL.h>
#include "id_heads.h"

// including opl.h is necessary for AdLibWrite and AdLibGetSample
// opl.h in turn needs dosbox.h and a definition of bool, as this is compiled as a c source
#define INCLUDED_FROM_SOURCEPORT
#include "dosbox.h" 

#define	sqMaxTracks	10

// 	Registers for the AdLib card
// Operator stuff
#define	alChar		0x20
#define	alScale		0x40
#define	alAttack	0x60
#define	alSus		0x80
#define	alWave		0xe0
// Channel stuff
#define	alFreqL		0xa0
#define	alFreqH		0xb0
// Global stuff
#define	alEffects	0xbd


// Change this to change sound quality.
static const int sampleRate = 44100;	// the audio output sample rate

// stuff used by the backend for sound prerendering and playing
int prerenderedSoundsNum=0, prerenderedSoundsDone=0;
signed short **prerenderedSounds=NULL;
int *prerenderedSoundsLength=NULL;
int prerenderedMusicNum=0, prerenderedMusicDone=0;
signed short **prerenderedMusic=NULL;
int *prerenderedMusicLength=NULL;

signed short *curSnd = NULL;
int curSndLength, curSndHead=0, curSndPriority=0;
signed short *curMusic = NULL;
int curMusicLength, curMusicHead=0, musicOn=0;

SoundSource soundSource = SND_OFF, musicSource = SND_OFF;

// this function does the sampling
static void spa_AudioCallback(void *userdata, uint8_t *stream, int len) {
	len /= 2;
	int i;

	long samples[len];
	if (curMusic != NULL && musicOn) {
		for (i = 0; i < len ; i++) {
			samples[i] = curMusic[curMusicHead++];
			if (curMusicHead >= curMusicLength) {
				curMusicHead = 0;
			}
		}
	} else {
		memset(samples, 0, sizeof(long)*len);
	}
	if (curSnd != NULL) {
		for (i = 0; i < len && curSndHead < curSndLength; i++) {
			samples[i] += curSnd[curSndHead++];
		}
		if (curSndHead >= curSndLength) {
			curSnd = NULL;
		}
	}

	signed short *out = (signed short*)stream;
	for (i = 0; i < len; i++) {
		signed long sin = samples[i];
		short signed sout;
		if (sin < 32768) {
			if (sin > -32768) {
				sout = sin;
			} else {
				sout = -32768;
			}
		} else {
			sout = 32767;
		}
		out[i] = sout;
	}
}


void spa_RenderAMusic(int MusicName, MusicSample *Music) {
	int	i;

	AdLibWrite(alEffects,0);
	for (i = 0;i < sqMaxTracks;i++)
		AdLibWrite(alFreqH + i + 1,0);

	
	long long musicLength=0;
	for (i = 0; i < Music->Count; i++) {
		int len = Music->Data[2*i+1];
		musicLength += len;
	}

	const int alCmdRate = 8*70;
	int numSamples = (long long)musicLength*sampleRate/alCmdRate;
	signed short *samples = (signed short*)malloc(numSamples*2);

	long long j=0;
	long offset=0;
	for (i = 0; i < Music->Count; i++) {
		word w = Music->Data[2*i+0];
		word len = Music->Data[2*i+1];
		AdLibWrite(w&0xFF,(w>>8)&0xFF);
		if (len > 0) {
			long newOffset = (j+(long long)len)*sampleRate/alCmdRate;
			AdLibGetSample(&samples[offset], newOffset - offset);
			offset = newOffset;
			j += (long long)len;
		}
	}
	AdLibWrite(alEffects,0);
	for (i = 0;i < sqMaxTracks;i++)
		AdLibWrite(alFreqH + i + 1,0);
	
	prerenderedMusic[MusicName] = samples;
	prerenderedMusicLength[MusicName] = numSamples;
}

void spa_RenderMusic() {
	if (prerenderedMusicDone) {
		return;
	} else {
		prerenderedMusicDone = 1;
	}

	printf("Rendering music.\n");
	int i;
	for (i = 0; i < prerenderedMusicNum; i++) {
		spa_RenderAMusic(i, SPD_GetMusic(i));
	}
}

void spa_RenderASample(int SampleName, SoundSample *Sample) {

	assert(Sample->mSus || Sample->cSus);

	AdLibWrite(alFreqH + 0,0);

	byte		c=3,m=0;
	AdLibWrite(m + alChar,Sample->mChar);
	AdLibWrite(m + alScale,Sample->mScale);
	AdLibWrite(m + alAttack,Sample->mAttack);
	AdLibWrite(m + alSus,Sample->mSus);
	AdLibWrite(m + alWave,Sample->mWave);
	AdLibWrite(c + alChar,Sample->cChar);
	AdLibWrite(c + alScale,Sample->cScale);
	AdLibWrite(c + alAttack,Sample->cAttack);
	AdLibWrite(c + alSus,Sample->cSus);
	AdLibWrite(c + alWave,Sample->cWave);

	int numCmds = Sample->Length;
	int alBlock = ((Sample->Block & 7) << 2) | 0x20;

	prerenderedSounds[SampleName] = NULL;
	prerenderedSoundsLength[SampleName] = 0;

	if (numCmds > 10000 || numCmds < 0) {
		return;
	}	

	const int alCmdRate = 2*70;
	int numSamples = (numCmds+1)*sampleRate/alCmdRate;
	signed short *samples = (signed short*)malloc(numSamples*2);
	int i;
	for (i = 0; i < numCmds; i++) {
		byte s = Sample->Data[i];
		if (!s) {
			AdLibWrite(alFreqH + 0,0);
		} else {
			AdLibWrite(alFreqL + 0,s);
			AdLibWrite(alFreqH + 0,alBlock);
		}
		AdLibGetSample(&samples[i*sampleRate/alCmdRate], (i+1)*sampleRate/alCmdRate - i*sampleRate/alCmdRate);
	}
	AdLibGetSample(&samples[numCmds*sampleRate/alCmdRate], (numCmds+1)*sampleRate/alCmdRate - numCmds*sampleRate/alCmdRate);

	prerenderedSounds[SampleName] = samples;
	prerenderedSoundsLength[SampleName] = numSamples;
}

void spa_RenderSounds() {
	if (prerenderedSoundsDone) {
		return;
	} else {
		prerenderedSoundsDone = 1;
	}
	printf("Rendering sound.\n");
	int i;
	for (i = 0; i < prerenderedSoundsNum; i++) {
		spa_RenderASample(i, SPD_GetSound(i));
	}
}

int SPA_IsAnySoundPlaying() {
	int playing;
	SDL_LockAudio();
	playing = curSnd != NULL;
	curSndPriority = 0;
	SDL_UnlockAudio();
	return playing;
}

int SPA_GetSoundSource(void) {
	return soundSource;
}

int SPA_GetMusicSource(void) {
	return musicSource;
}

void SPA_MusicOff(void) {
printf("MUSIC OFF!\n");
	SDL_LockAudio();
	musicOn = 0;
	SDL_UnlockAudio();
}

void SPA_MusicOn(void) {
printf("MUSIC ON! music source %i\n", musicSource);
	SDL_LockAudio();
	musicOn = (musicSource != SND_OFF) && 1;
	SDL_UnlockAudio();
}

int SPA_PlaySound(int SoundName) {
// priority is not loaded, it's anyway always 0.
	if (soundSource == SND_OFF || SoundName < 0 || SoundName >= prerenderedSoundsNum) {
		return false;
	}
	int prio = SPD_GetSound(SoundName)->Priority;
	SDL_LockAudio();
	if (curSnd == NULL || prio >= curSndPriority) {
		curSnd = prerenderedSounds[SoundName];
		curSndLength = prerenderedSoundsLength[SoundName];
		curSndPriority = prio;
		curSndHead = 0;
	} else {
		printf("Sound rejected because of priority (%i < %i) !\n", prio, curSndPriority);
	}
	SDL_UnlockAudio();
	return true;
}


void SPA_SetMusicSource(SoundSource Source) {
	printf("music source %i wanted %i\n", musicSource, Source);
	if (Source == SND_OFF && musicSource != SND_OFF) {
		musicSource = SND_OFF;
		SPA_MusicOff();
	} else if (Source == SND_ADLIB && musicSource == SND_OFF) {
		musicSource = SND_ADLIB;
		spa_RenderMusic();
		SPA_MusicOn();
	}
	printf("music source %i wanted %i\n", musicSource, Source);
}

void SPA_SetSoundSource(SoundSource Source) {
	printf("sound source %i wanted %i\n", soundSource, Source);
	if (Source == SND_OFF && soundSource != SND_OFF) {
		soundSource = SND_OFF;
		SDL_LockAudio();
		curSnd = NULL;
		SDL_UnlockAudio();
	} else if (Source == SND_ADLIB && soundSource == SND_OFF) {
		soundSource = SND_ADLIB;
		spa_RenderSounds();
	}
	printf("sound source %i wanted %i\n", soundSource, Source);
}

void SPA_StartMusic(int MusicName) {
	if (musicSource == SND_OFF || MusicName < 0 || MusicName >= prerenderedMusicNum) {
		return;
	}
	SDL_LockAudio();
	curMusic = prerenderedMusic[MusicName];
	curMusicLength = prerenderedMusicLength[MusicName];
	assert(curMusic != NULL);
	curMusicHead = 0;
	musicOn = 1;
	SDL_UnlockAudio();
}

void SPA_WaitUntilSoundIsDone() {
	long end = SDL_GetTicks()+2000;
	while (SDL_GetTicks() < end) {
		if (!SPA_IsAnySoundPlaying()) {
			return;
		}
	}
}

void SPA_Init() {
	AdLibInit(sampleRate);
	SDL_AudioSpec desired;
	desired.freq = sampleRate;
	desired.format = AUDIO_S16;
	desired.channels = 1;
	desired.samples = 1024;
	desired.callback = spa_AudioCallback;
	desired.userdata = NULL;
	assert(SDL_OpenAudio(&desired, NULL) == 0);

}


void SPA_InitSamples(int NumSamples, int NumMusic) {
	assert(prerenderedSounds == NULL);
	prerenderedSoundsNum = NumSamples;
	prerenderedSounds = malloc(sizeof(signed short*)*NumSamples);
	prerenderedSoundsLength = malloc(sizeof(int)*NumSamples);
	memset(prerenderedSounds, 0, sizeof(signed short*)*NumSamples);
	memset(prerenderedSoundsLength, 0, sizeof(int)*NumSamples);

	prerenderedMusicNum = NumMusic;
	prerenderedMusic = malloc(sizeof(signed short*)*NumMusic);
	prerenderedMusicLength = malloc(sizeof(int)*NumMusic);
	memset(prerenderedMusic, 0, sizeof(signed short*)*NumMusic);
	memset(prerenderedMusicLength, 0, sizeof(int)*NumMusic);
}


