#ifndef COMPAT_H
#define COMPAT_H

// defines to make this source port work
#define CASTAT(type, where) (*(type*)&(where))
#define SPLITSCREENOFFSET 144

typedef void* memptr;


#include <stdint.h>
#include <assert.h>
void SP_Exit();
void SP_FlipBuffer();
void SP_GameEnter();
void SP_GameLeave();
void SP_MusicOff();
void SP_MusicOn();
void SP_PlaySound(void *Sound);
void SP_StartMusic(void *Music);
int  SP_StrafeOn();
void SP_SetTimeCount(long Ticks);
long SP_TimeCount();

int LastScan();
char LastASCII();
int Keyboard(int Key);

uint8_t GetTilemap(int x, int y);

typedef struct
{
  uint16_t bit0,bit1;	// 0-255 is a character, > is a pointer to a node
} huffnode;


extern int32_t GrChunksNum, GrChunksPos[], GrChunksSize[];
extern int32_t AudioChunksNum, AudioChunksPos[], AudioChunksSize[];
extern huffnode audiohuffman[], grhuffman[];

 
#endif

