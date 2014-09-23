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

#include "srcport.h"
#include <SDL/SDL.h>
#include "c3_def.h"

// including opl.h is necessary for AdLibWrite and AdLibGetSample
// opl.h in turn needs dosbox.h and a definition of bool, as this is compiled as a c source
#define INCLUDED_FROM_SOURCEPORT
#include "dosbox.h" 

// Change this to change sound quality.
static const int sampleRate = 44100;	// the audio output sample rate

// stuff used by the backend for sound prerendering and playing
int prerenderedSoundsNum=0;
signed short **prerenderedSounds=NULL;
int *prerenderedSoundsLength=NULL;
int prerenderedMusicNum=0;
signed short **prerenderedMusic=NULL;
int *prerenderedMusicLength=NULL;

signed short *curSnd = NULL;
int curSndLength, curSndHead=0, musicOn=0;
signed short *curMusic = NULL;
int curMusicLength, curMusicHead=0;

// this function does the sampling
void SPA_AudioCallback(void *userdata, uint8_t *stream, int len) {
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

void SPA_Init() {
	AdLibInit(sampleRate);
	SDL_AudioSpec desired;
	desired.freq = sampleRate;
	desired.format = AUDIO_S16;
	desired.channels = 1;
	desired.samples = 1024;
	desired.callback = SPA_AudioCallback;
	desired.userdata = NULL;
	assert(SDL_OpenAudio(&desired, NULL) == 0);

}

void SP_MusicOff(void) {
	SDL_LockAudio();
	musicOn = 0;
	SDL_UnlockAudio();
}

void SP_MusicOn(void) {
	SDL_LockAudio();
	musicOn = 1;
	SDL_UnlockAudio();
}

int SP_PlaySound(int SoundName) {
// priority is not loaded, it's anyway always 0.
	if (SoundName < 0 || SoundName >= prerenderedSoundsNum) {
		return false;
	}
	SDL_LockAudio();
	curSnd = prerenderedSounds[SoundName];
	curSndLength = prerenderedSoundsLength[SoundName];
	assert(curSnd != NULL);
	curSndHead = 0;
	SDL_UnlockAudio();
	return true;
}

void SP_StartMusic(int MusicName) {
	if (MusicName < 0 || MusicName >= prerenderedMusicNum) {
		return;
	}
	SDL_LockAudio();
	curMusic = prerenderedMusic[MusicName];
	curMusicLength = prerenderedMusicLength[MusicName];
	assert(curMusic != NULL);
	curMusicHead = 0;
	SDL_UnlockAudio();
}


void SPA_InitSamples(int NumSamples, int NumMusic) {
	assert(prerenderedSounds == NULL);
	prerenderedSoundsNum = NumSamples;
	prerenderedSounds = malloc(sizeof(signed short*)*NumSamples);
	prerenderedSoundsLength = malloc(sizeof(int)*NumSamples);

	prerenderedMusicNum = NumMusic;
	prerenderedMusic = malloc(sizeof(signed short*)*NumMusic);
	prerenderedMusicLength = malloc(sizeof(int)*NumMusic);
}

void SPA_RenderMusic(int MusicName, uint8_t *Data) {
#ifdef NOMUSIC
	prerenderedMusicNum = 0;
	return;
#endif
	MusicGroup *Music = (MusicGroup*)Data;
	int	i;

	AdLibWrite(alEffects,0);
	for (i = 0;i < sqMaxTracks;i++)
		AdLibWrite(alFreqH + i + 1,0);

	
	word *musicAlCmds = Music->values;
	int numAlCmds = Music->length/4;

	long long musicLength=0;
	for (i = 0; i < numAlCmds; i++) {
		int len = musicAlCmds[2*i+1];
		musicLength += len;
	}

	const int alCmdRate = 8*70;
	int numSamples = (long long)musicLength*sampleRate/alCmdRate;
	signed short *samples = (signed short*)malloc(numSamples*2);

	long long j=0;
	long offset=0;
	for (i = 0; i < numAlCmds; i++) {
		word w = musicAlCmds[2*i+0];
		word len = musicAlCmds[2*i+1];
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


void SPA_RenderSample(int SampleName, uint8_t *Data) {

	AdLibSound *sound = (AdLibSound*)Data;
	Instrument	*inst = (Instrument*)((char*)sound+6);
	assert(inst->mSus || inst->cSus);

	AdLibWrite(alFreqH + 0,0);

	byte		c=3,m=0;
	AdLibWrite(m + alChar,inst->mChar);
	AdLibWrite(m + alScale,inst->mScale);
	AdLibWrite(m + alAttack,inst->mAttack);
	AdLibWrite(m + alSus,inst->mSus);
	AdLibWrite(m + alWave,inst->mWave);
	AdLibWrite(c + alChar,inst->cChar);
	AdLibWrite(c + alScale,inst->cScale);
	AdLibWrite(c + alAttack,inst->cAttack);
	AdLibWrite(c + alSus,inst->cSus);
	AdLibWrite(c + alWave,inst->cWave);

	byte *alCmds = (byte*)sound+23;
	int numCmds = sound->common.length;
	int alBlock = ((*((byte*)sound+22) & 7) << 2) | 0x20;

	const int alCmdRate = 2*70;
	int numSamples = (numCmds+1)*sampleRate/alCmdRate;
	signed short *samples = (signed short*)malloc(numSamples*2);
	int i;
	for (i = 0; i < numCmds; i++) {
		byte s = *alCmds++;
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


