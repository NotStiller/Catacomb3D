#define EYESOPEN
#include "dosbox.h"


void AdLibInit(Bit32u samplerate) {
}

void AdLibWrite(Bitu idx, Bit8u val) {
}

void AdLibGetSample(Bit16s* sndptr, Bits numsamples) {
	int i;
	for (i = 0; i < numsamples; i++) {
		sndptr[i] = 0;
	}
}

