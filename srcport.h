#ifndef COMPAT_H
#define COMPAT_H

// defines to make this work
#define CASTAT(type, where) (*(type*)&(where))
#define NYI(FuncName) printf("NYI: %s in %s:%i\n", (#FuncName), __FILE__, __LINE__);
//#define MK_FP(Segment, Offset) (void*)((unsigned long)MEMORY+(unsigned long)(Segment)*16+(unsigned long)(Offset))
//#define FP_SEG(Ptr)  ((((unsigned long)(Ptr)-(unsigned long)MEMORY)>>16)<<12)
//#define FP_OFF(Ptr)  (((unsigned long)(Ptr)-(unsigned long)MEMORY)&0xFFFFul)
#define inportb(a) (0)
#define outportb(a,b) (0)
#define movedata(source,dest,len) (memmove((dest), (source), (len)))
#define interrupt
#define far
#define _seg
#define huge 
#define ASM(a)
#define O_BINARY 0
#define O_TEXT 0
#define S_IREAD S_IRUSR
#define S_IWRITE S_IWUSR
#define SPLITSCREENOFFSET 144
#include <signal.h>
#include <stdint.h>

void BE_FlipBuffer();

void BE_StartMusic(void *Music);
void BE_MusicOff();
void BE_MusicOn();
void BE_PlaySound(void *Sound);
void BE_Exit();

extern int BE_StrafeOn;

extern int _argc;
extern char **_argv;

#include <assert.h>
 
#endif

