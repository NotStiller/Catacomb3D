/* Catacomb 3-D Source Code
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


//
//	ID Engine
//	ID_SD.c - Sound Manager
//	v1.1d1
//	By Jason Blochowiak
//

//
//	This module handles dealing with generating sound on the appropriate
//		hardware
//
//	Depends on: User Mgr (for parm checking)
//
//	Globals:
//		For User Mgr:
//			SoundMode - What device is used for sound effects
//				(Use SM_SetSoundMode() to set)
//			MusicMode - What device is used for music
//				(Use SM_SetMusicMode() to set)
//		For Cache Mgr:
//			NeedsDigitized - load digitized sounds?
//			NeedsMusic - load music?
//

#pragma hdrstop		// Wierdo thing with MUSE

//#include <dos.h>

#ifdef	_MUSE_      // Will be defined in ID_Types.h
#include "id_sd.h"
#else
#include "id_heads.h"
#endif
#pragma	hdrstop
#pragma	warn	-pia

#define	SDL_SoundFinished()	{SoundNumber = SoundPriority = 0;}

// Macros for AdLib stuff
#define	selreg(n)	outportb(0x388,n)
#define	writereg(n)	outportb(0x389,n)
#define	readstat()	inportb(0x388)

//	Global variables
	boolean		NeedsDigitized,NeedsMusic;
	SDMode		SoundMode;
	SMMode		MusicMode;
	boolean		ssIsTandy;
	word		ssPort = 2;

//	Internal variables
static	boolean			SD_Started;
static	char			*ParmStrings[] =
						{
							"noal",
							nil
						};
static	word			SoundNumber,SoundPriority;
//static	word			t0CountTable[] = {8,8,8,8,40,40};

//	AdLib variables
static	boolean			alNoCheck;
static	byte		 *alSound;
static	word			alBlock;
static	longword		alLengthLeft;
static	longword		alTimeCount;
static	Instrument		alZeroInst;

// This table maps channel numbers to carrier and modulator op cells
static	byte			carriers[9] =  { 3, 4, 5,11,12,13,19,20,21},
						modifiers[9] = { 0, 1, 2, 8, 9,10,16,17,18},
// This table maps percussive voice numbers to op cells
						pcarriers[5] = {19,0xff,0xff,0xff,0xff},
						pmodifiers[5] = {16,17,18,20,21};

//	Sequencer variables
static	boolean			sqActive;
static	word			alFXReg;
static	ActiveTrack		*tracks[sqMaxTracks],
						mytracks[sqMaxTracks];
static	word			sqMode,sqFadeStep;
static	word			*sqHack,*sqHackPtr,sqHackLen,sqHackSeqLen;
static	long			sqHackTime;

//	Internal routines


///////////////////////////////////////////////////////////////////////////
//
//	SDL_ALStopSound() - Turns off any sound effects playing through the
//		AdLib card
//
///////////////////////////////////////////////////////////////////////////
static void SDL_ALStopSound(void)
{
	alSound = 0;
}

////////////////////////////////////////////////////////////////////////////
//
//	SDL_ShutDevice() - turns off whatever device was being used for sound fx
//
////////////////////////////////////////////////////////////////////////////
static void
SDL_ShutDevice(void)
{
	switch (SoundMode)
	{
	case sdm_AdLib:
//		SDL_ShutAL();
		break;
	}
	SoundMode = sdm_Off;
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_CleanDevice() - totally shuts down all sound devices
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_CleanDevice(void)
{
}

///////////////////////////////////////////////////////////////////////////
//
//	SDL_StartDevice() - turns on whatever device is to be used for sound fx
//
///////////////////////////////////////////////////////////////////////////
static void
SDL_StartDevice(void)
{
	switch (SoundMode)
	{
	case sdm_AdLib:
		break;
	}
	SoundNumber = SoundPriority = 0;
}


//	Public routines

///////////////////////////////////////////////////////////////////////////
//
//	SD_SetSoundMode() - Sets which sound hardware to use for sound effects
//
///////////////////////////////////////////////////////////////////////////
boolean
SD_SetSoundMode(SDMode mode)
{
	boolean	result;

	SD_StopSound();

	switch (mode)
	{
	case sdm_Off:
		NeedsDigitized = false;
		result = true;
		break;
	case sdm_AdLib:
		NeedsDigitized = false;
		result = true;
		break;
	default:
		result = false;
		break;
	}

	if (result && (mode != SoundMode))
	{
		SDL_ShutDevice();
		SoundMode = mode;
		SDL_StartDevice();
	}

	return(result);
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_SetMusicMode() - sets the device to use for background music
//
///////////////////////////////////////////////////////////////////////////
boolean
SD_SetMusicMode(SMMode mode)
{
	boolean	result;

	SD_FadeOutMusic();
	while (SD_MusicPlaying())
		;

	switch (mode)
	{
	case smm_Off:
		NeedsMusic = false;
		result = true;
		break;
	case smm_AdLib:
		NeedsMusic = true;
		result = true;
		break;
	default:
		result = false;
		break;
	}

	if (result)
		MusicMode = mode;

	return(result);
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_Startup() - starts up the Sound Mgr
//		Detects all additional sound hardware and installs my ISR
//
///////////////////////////////////////////////////////////////////////////
void
SD_Startup(void)
{
	int	i;

	if (SD_Started)
		return;

	ssIsTandy = false;
	alNoCheck = false;

	SP_SetTimeCount(0);
	alTimeCount = 0;

	SD_SetSoundMode(sdm_Off);
	SD_SetMusicMode(smm_Off);

	SD_Started = true;
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_Default() - Sets up the default behaviour for the Sound Mgr whether
//		the config file was present or not.
//
///////////////////////////////////////////////////////////////////////////
void
SD_Default(boolean gotit,SDMode sd,SMMode sm)
{
	boolean	gotsd,gotsm;

	gotsd = gotsm = gotit;

	if (gotsd)	// Make sure requested sound hardware is available
	{
		switch (sd)
		{
		case sdm_AdLib:
			gotsd = true;
			break;
		}
	}
	if (!gotsd)
	{
		sd = sdm_AdLib;
	}
	if (sd != SoundMode)
		SD_SetSoundMode(sd);


	if (gotsm)	// Make sure requested music hardware is available
	{
		switch (sm)
		{
		case sdm_AdLib:
			gotsm = true;
			break;
		}
	}
	if (!gotsm)
	{
		sm = smm_AdLib;
	}
	if (sm != MusicMode)
		SD_SetMusicMode(sm);
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_Shutdown() - shuts down the Sound Mgr
//		Removes sound ISR and turns off whatever sound hardware was active
//
///////////////////////////////////////////////////////////////////////////
void
SD_Shutdown(void)
{
	if (!SD_Started)
		return;

	SD_MusicOff();
	SDL_ShutDevice();
	SDL_CleanDevice();
	SD_Started = false;
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_PlaySound() - plays the specified sound on the appropriate hardware
//
///////////////////////////////////////////////////////////////////////////
void
SD_PlaySound(soundnames sound)
{
	if ((SoundMode != sdm_AdLib) || (sound == -1))
		return;

	if (SP_PlaySound(sound)) {
		SoundNumber = sound;
	}
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_SoundPlaying() - returns the sound number that's playing, or 0 if
//		no sound is playing
//
///////////////////////////////////////////////////////////////////////////
word
SD_SoundPlaying(void)
{
	boolean	result = false;

	switch (SoundMode)
	{
	case sdm_AdLib:
		result = alSound? true : false;
		break;
	}

	if (result)
		return(SoundNumber);
	else
		return(false);
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_StopSound() - if a sound is playing, stops it
//
///////////////////////////////////////////////////////////////////////////
void
SD_StopSound(void)
{
	switch (SoundMode)
	{
	case sdm_AdLib:
		SDL_ALStopSound();
		break;
	}

	SDL_SoundFinished();
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_WaitSoundDone() - waits until the current sound is done playing
//
///////////////////////////////////////////////////////////////////////////
void
SD_WaitSoundDone(void)
{
printf("Waiting for sound not done yet !\n");
/*	while (SD_SoundPlaying());*/
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_MusicOn() - turns on the sequencer
//
///////////////////////////////////////////////////////////////////////////
void
SD_MusicOn(void)
{
	sqActive = true;
	SP_MusicOn();
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_MusicOff() - turns off the sequencer and any playing notes
//
///////////////////////////////////////////////////////////////////////////
void
SD_MusicOff(void)
{
	SP_MusicOff();
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_StartMusic() - starts playing the music pointed to
//
///////////////////////////////////////////////////////////////////////////
void
SD_StartMusic(int MusicName)
{
	if (MusicMode == smm_AdLib)
	{
		SP_StartMusic(MusicName);
		SD_MusicOn();
	}
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_FadeOutMusic() - starts fading out the music. Call SD_MusicPlaying()
//		to see if the fadeout is complete
//
///////////////////////////////////////////////////////////////////////////
void
SD_FadeOutMusic(void)
{
	switch (MusicMode)
	{
	case smm_AdLib:
		// DEBUG - quick hack to turn the music off
		SD_MusicOff();
		break;
	}
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_MusicPlaying() - returns true if music is currently playing, false if
//		not
//
///////////////////////////////////////////////////////////////////////////
boolean
SD_MusicPlaying(void)
{
	boolean	result;

	switch (MusicMode)
	{
	case smm_AdLib:
		result = false;
		// DEBUG - not written
		break;
	default:
		result = false;
	}

	return(result);
}
