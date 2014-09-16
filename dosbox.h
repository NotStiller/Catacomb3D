/******************************************************************
   This is not the real dosbox.h from DOSBox, but simply a stub,
   so that I don't have to change opl.h and opl.cpp.
*******************************************************************/


/*
	define Bits, Bitu, Bit32s, Bit32u, Bit16s, Bit16u, Bit8s, Bit8u here
*/

#include <string.h>
#include <stdint.h>
typedef uintptr_t	Bitu;
typedef intptr_t	Bits;
typedef uint32_t	Bit32u;
typedef int32_t		Bit32s;
typedef uint16_t	Bit16u;
typedef int16_t		Bit16s;
typedef uint8_t		Bit8u;
typedef int8_t		Bit8s;


#define INLINE inline

#ifdef INCLUDED_FROM_SOURCEPORT
typedef int bool;

void AdLibInit(Bit32u samplerate);
void AdLibWrite(Bitu idx, Bit8u val);
void AdLibGetSample(Bit16s* sndptr, Bits numsamples);

#else
void adlib_init(Bit32u samplerate);
void adlib_write(Bitu idx, Bit8u val);
void adlib_getsample(Bit16s* sndptr, Bits numsamples);

extern "C" void AdLibInit(Bit32u samplerate) { adlib_init(samplerate); }
extern "C" void AdLibWrite(Bitu idx, Bit8u val) { adlib_write(idx, val); } 
extern "C" void AdLibGetSample(Bit16s* sndptr, Bits numsamples) { adlib_getsample(sndptr, numsamples); }
#endif

