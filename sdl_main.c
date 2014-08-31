#include <stdlib.h>
#include <SDL/SDL.h>
#include "c3_def.h"
#include <pthread.h>
#include "id_sd.h"

void ungrab() {
	SDL_WM_GrabInput(SDL_GRAB_OFF);
	SDL_ShowCursor(SDL_ENABLE);
}

// including opl.h is necessary for AdLibWrite and AdLibGetSample
// opl.h in turn needs dosbox.h and a definition of bool, as this is compiled as a c source
#define EYESOPEN
#include "dosbox.h" 

#define U8(BUFFER) ((unsigned int)*(unsigned char*)(BUFFER))
#define U16(BUFFER) (U8(BUFFER)+256*U8(BUFFER+1))


// Some constants, should be okay to change them !
static const int sampleRate = 44100;	// the audio output sample rate
int screenScaleUp=2; 					// 2 means 640x400 window etc.
#define GRABKEY SDLK_m					// the button that is used to grab the mouse

// Some constants, don't change them !
static const int gameTimerRate=70;		// I think 70 is correct, but maybe it should read 35 ?
static const int screenWidth = 320;		// don't change these 2, otherwise the game might crash !
static const int screenHeight = 200;

// some state for input handling
static int mouseDX=0, mouseDY=0;
static unsigned int mouseButtons=0;
int BE_StrafeOn=0;

// stuff used by the backend
pthread_mutex_t SDLMutex;
static SDL_Surface* screen = NULL;
static unsigned char *screenBuffer = NULL;
static int pleaseExit=false;

// stuff used by the backend for sound prerendering and playing
#define MAXRENDEREDSOUNDS 128
static signed short *renderedSounds[MAXRENDEREDSOUNDS];
static int renderedSoundsLength[MAXRENDEREDSOUNDS];
static void *renderedSoundsOriginal[MAXRENDEREDSOUNDS];
signed short *curSnd = NULL;
int curSndLength, curSndHead=0, musicOn=0;
signed short *curMusic = NULL;
int curMusicLength, curMusicHead=0;


// some prototypes that are needed by main
void* GameThread(void *Arg);
static void audioCallback(void *userdata, Uint8 *stream, int len);
int loadObjHack(const char *Filename, void **DataPtr);
static void setPalette();
int translateKey(int Sym);

int main(int argc, char **argv) {
	SDL_Init(SDL_INIT_EVERYTHING);
	screen = SDL_SetVideoMode(screenScaleUp*screenWidth, screenScaleUp*screenHeight, 8, SDL_SWSURFACE);
	assert(screen != NULL);

	SDL_EnableUNICODE(1);

	AdLibInit(sampleRate);

	int i;
	for (i = 0; i < MAXRENDEREDSOUNDS; i++) {
		renderedSounds[i] = 0;
		renderedSoundsOriginal[i] = 0;
		renderedSoundsLength[i] = 0;
	}

	SDL_AudioSpec desired;
	desired.freq = sampleRate;
	desired.format = AUDIO_S16;
	desired.channels = 1;
	desired.samples = 1024;
	desired.callback = audioCallback;
	desired.userdata = NULL;
	assert(SDL_OpenAudio(&desired, NULL) == 0);


// clear screen
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = screenScaleUp*screenWidth;
	rect.h = screenScaleUp*screenHeight;
	pthread_mutex_lock(&SDLMutex);
	assert(SDL_FillRect(screen, &rect, 0) == 0);
	pthread_mutex_unlock(&SDLMutex);

// allocate and clear buffer
	screenBuffer = malloc(320*200);
	memset(screenBuffer, 0, 320*200);

	setPalette();

	pthread_t thread;
	assert(!pthread_mutex_init(&SDLMutex, NULL));
	assert(!pthread_create(&thread, NULL, &GameThread, NULL));

	boolean grab=false;
	boolean done=false;
	while (!done) {
		SDL_Event event;
		pthread_mutex_lock(&SDLMutex);
		if (pleaseExit) {
			done = true;
		}
		while(SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				done = true;
			} else if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
//					done = true;
					break;
				case GRABKEY:
					grab = !grab;
					if (grab) {
						if (SDL_WM_GrabInput(SDL_GRAB_ON) != SDL_GRAB_ON) {
							grab = false;
						} else {
							SDL_ShowCursor(SDL_DISABLE);
							BE_StrafeOn = 1;
						}
					} else {
						if (SDL_WM_GrabInput(SDL_GRAB_OFF) != SDL_GRAB_OFF) {
							grab = true;
						} else {
							SDL_ShowCursor(SDL_ENABLE);
							BE_StrafeOn = 0;
						}
					}
					break;
				}
			}
			
			if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
				boolean down = event.type == SDL_KEYDOWN;
				int idKey = translateKey(event.key.keysym.sym);
				if (idKey != 0) {
					Keyboard[idKey] = down;
					if (down) {
						LastScan = idKey;
					}
				}
				char c=0;
				if ((event.key.keysym.unicode&0xFF80) == 0) {
					c = event.key.keysym.unicode&0x7F;
				}
				if (down && c != 0) {
					LastASCII = c;
				}
			} else if (event.type == SDL_MOUSEMOTION) {
				mouseDX += event.motion.xrel;
				mouseDY += event.motion.yrel;
			} else if (event.type == SDL_MOUSEBUTTONDOWN) {
				if (event.button.button == SDL_BUTTON_LEFT) {
					mouseButtons |= 1;
				} else if (event.button.button == SDL_BUTTON_RIGHT) {
					mouseButtons |= 2;
				}
			} else if (event.type == SDL_MOUSEBUTTONUP) {
				if (event.button.button == SDL_BUTTON_LEFT) {
					mouseButtons &= ~1;
				} else if (event.button.button == SDL_BUTTON_RIGHT) {
					mouseButtons &= ~2;
				}
			}
		}
		pthread_mutex_unlock(&SDLMutex);
	}

	SDL_Quit();
	return 0;
}


void* GameThread(void *Arg) {
	InitGame ();
	CheckMemory ();
	LoadLatchMem ();

// prototype for the function defined below
	Uint32 gameTimer(Uint32 Interval, void *Param);

	SDL_AddTimer(1000/gameTimerRate, gameTimer, NULL);
	SDL_PauseAudio(0);
	DemoLoop();
}


Uint32 gameTimer(Uint32 Interval, void *Param) {
	SDL_t0Service();
	return 1000/gameTimerRate;
}

// this function does the sampling
static void audioCallback(void *userdata, Uint8 *stream, int len) {
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


static signed short *renderMusic(MusicGroup *Music, int *NumSamples) {
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

	const int alCmdRate = 8*gameTimerRate;
	*NumSamples = (long long)musicLength*sampleRate/alCmdRate;
	signed short *samples = (signed short*)malloc(*NumSamples*2);

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

	return samples;
}


static signed short *renderSound(AdLibSound *sound, int *NumSamples) {
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

	const int alCmdRate = 2*gameTimerRate;
	*NumSamples = (numCmds+1)*sampleRate/alCmdRate;
	signed short *samples = (signed short*)malloc(*NumSamples*2);
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

	return samples;
}



int translateKey(int Sym) {
	switch (Sym) {
	default: return 0; break;
	case SDLK_RETURN: return sc_Return; break;
	case SDLK_ESCAPE: return sc_Escape; break;
	case SDLK_SPACE: return sc_Space; break;
	case SDLK_BACKSPACE: return sc_BackSpace; break;
	case SDLK_TAB: return sc_Tab; break;
	case SDLK_LALT: return sc_Alt; break;
	case SDLK_RALT: return sc_Alt; break;
	case SDLK_LCTRL: return sc_Control; break;
	case SDLK_RCTRL: return sc_Control; break;
	case SDLK_LSHIFT: return sc_LShift; break;
	case SDLK_RSHIFT: return sc_RShift; break;
	case SDLK_CAPSLOCK: return sc_CapsLock; break;
	
	case SDLK_UP: return sc_UpArrow; break;
	case SDLK_DOWN: return sc_DownArrow; break;
	case SDLK_LEFT: return sc_LeftArrow; break;
	case SDLK_RIGHT: return sc_RightArrow; break;

	case SDLK_PAGEUP: return sc_PgUp; break;
	case SDLK_PAGEDOWN: return sc_PgDn; break;
	case SDLK_INSERT: return sc_Insert; break;
	case SDLK_DELETE: return sc_Delete; break;
	case SDLK_HOME: return sc_Home; break;
	case SDLK_END: return sc_End; break;
	case SDLK_F1: return sc_F1; break;
	case SDLK_F2: return sc_F2; break;
	case SDLK_F3: return sc_F3; break;
	case SDLK_F4: return sc_F4; break;
	case SDLK_F5: return sc_F5; break;
	case SDLK_F6: return sc_F6; break;
	case SDLK_F7: return sc_F7; break;
	case SDLK_F8: return sc_F8; break;
	case SDLK_F9: return sc_F9; break;
	case SDLK_F10: return sc_F10; break;
	case SDLK_F11: return sc_F11; break;
	case SDLK_F12: return sc_F12; break;
	case SDLK_a: return sc_A; break;
	case SDLK_b: return sc_B; break;
	case SDLK_c: return sc_C; break;
	case SDLK_d: return sc_D; break;
	case SDLK_e: return sc_E; break;
	case SDLK_f: return sc_F; break;
	case SDLK_g: return sc_G; break;
	case SDLK_h: return sc_H; break;
	case SDLK_i: return sc_I; break;
	case SDLK_j: return sc_J; break;
	case SDLK_k: return sc_K; break;
	case SDLK_l: return sc_L; break;
	case SDLK_m: return sc_M; break;
	case SDLK_n: return sc_N; break;
	case SDLK_o: return sc_O; break;
	case SDLK_p: return sc_P; break;
	case SDLK_q: return sc_Q; break;
	case SDLK_r: return sc_R; break;
	case SDLK_s: return sc_S; break;
	case SDLK_t: return sc_T; break;
	case SDLK_u: return sc_U; break;
	case SDLK_v: return sc_V; break;
	case SDLK_w: return sc_W; break;
	case SDLK_x: return sc_X; break;
	case SDLK_y: return sc_Y; break;
	case SDLK_z: return sc_Z; break;
	}
}


/* Source for the following is
http://commons.wikimedia.org/w/index.php?title=File:EGA_Table.PNG&oldid=39767054
*/
static unsigned long EGAPalette[64] = {
	0x000000, //  0
	0x0000AA, //  1
	0x00AA00, //  2
	0x00AAAA, //  3
	0xAA0000, //  4
	0xAA00AA, //  5
	0xAAAA00, //  6
	0xAAAAAA, //  7
	
	0x000055, //  8
	0x0000FF, //  9
	0x00AA55, // 10
	0x00AAFF, // 11
	0xAA0055, // 12
	0xAA00FF, // 13
	0xAAAA55, // 14
	0xAAAAFF, // 15

	0x005500, // 16
	0x0055AA, // 17
	0x00FF00, // 18
	0x00FFAA, // 19
	0xAA5500, // 20
	0xAA55AA, // 21
	0xAAFF00, // 22
	0xAAFFAA, // 23

	0x005555, // 24
	0x0055FF, // 25
	0x00FF55, // 26
	0x00FFFF, // 27
	0xAA5555, // 28
	0xAA55FF, // 29
	0xAAFF55, // 30
	0xAAFFFF, // 31

	0x550000, // 32
	0x5500AA, // 33
	0x55AA00, // 34
	0x55AAAA, // 35
	0xFF0000, // 36
	0xFF00AA, // 37
	0xFFAA00, // 38
	0xFFAAAA, // 39

	0x550055, // 40
	0x5500FF, // 41
	0x55AA55, // 42
	0x55AAFF, // 43
	0xFF0055, // 44
	0xFF00FF, // 45
	0xFFAA55, // 46
	0xFFAAFF, // 47

	0x555500, // 48
	0x5555AA, // 49
	0x55FF00, // 50
	0x55FFAA, // 51
	0xFF5500, // 52
	0xFF55AA, // 53
	0xFFFF00, // 54
	0xFFFFAA, // 55

	0x555555, // 56
	0x5555FF, // 57
	0x55FF55, // 58
	0x55FFFF, // 59
	0xFF5555, // 60
	0xFF55FF, // 61
	0xFFFF55, // 62
	0xFFFFFF, // 63
};


static int EGADefaultColors[] = {0, 1, 2, 3, 4, 5, 20, 7, 56, 57, 58, 59, 60, 61, 62, 63};

static void setPalette() {
	SDL_Color cols[256];
	int i;
	for (i = 0; i < 16; i++) {
		unsigned long hash = EGAPalette[EGADefaultColors[i]];
		cols[i].r = (hash>>16)&0xFF;
		cols[i].g = (hash>>8)&0xFF;
		cols[i].b = hash&0xFF;
	}
	SDL_SetPalette(screen, SDL_LOGPAL|SDL_PHYSPAL, cols, 0, 16);
}





static void flipPixel(unsigned x, unsigned y) { // this is a mean hack to make FizzleFade work, but whatever...
	SDL_Rect rect;
	rect.y = screenScaleUp*y;
	rect.x = screenScaleUp*x;
	rect.w = screenScaleUp;
	rect.h = screenScaleUp;
	pthread_mutex_lock(&SDLMutex);
	assert(SDL_FillRect(screen, &rect, screenBuffer[x+y*screenWidth]) == 0);
	pthread_mutex_unlock(&SDLMutex);
}

void plotXOR(unsigned x, unsigned y, unsigned color) {
	if (x < 0 || y < 0 || x >= screenWidth || y >= screenHeight) {
		return;
	}
	screenBuffer[y*screenWidth+x] ^= color;
}

static void putPixel(unsigned x, unsigned y, unsigned color) {
	screenBuffer[y*screenWidth+x] = color;
}


int drawPropChar(int X, int Y, int FontNumber, int Char) {
	fontstruct *font = (fontstruct*)grsegs[STARTFONT+FontNumber];
	int width = font->width[Char];
	byte *chardata = (byte*)font + font->location[Char];
	byte *c = chardata;
	int x, y, i;
	for (y = 0; y < font->height; y++) {
		int i = 8;
		for (x = 0; x < width;) {
			unsigned short b = *c++;
			for (i = 0; i < 8; i++) {
				if (b&0x80) {
					plotXOR(X+x, Y+y, fontcolor);
				}
				x++;
				b <<= 1;
			}
		}
	}
	return width;
}

// MouseButtons and MouseDelta were originally macros in id_heads.h or so.

void MouseButtons(unsigned int *B) {
	pthread_mutex_lock(&SDLMutex);
	if (B != NULL) {
		*B = mouseButtons;
	}
	pthread_mutex_unlock(&SDLMutex);
}

void MouseDelta(int *X, int *Y) {
	pthread_mutex_lock(&SDLMutex);
	if (X != NULL) {
		*X = mouseDX;
	}
	if (Y != NULL) {
		*Y = mouseDY;
	}
	mouseDX = 0;
	mouseDY = 0;
	pthread_mutex_unlock(&SDLMutex);
}


void BE_Exit() {
	pthread_mutex_lock(&SDLMutex);
	pleaseExit = true;
	pthread_mutex_unlock(&SDLMutex);
	pthread_exit(NULL);
}

void BE_FlipBuffer (void) {
	pthread_mutex_lock(&SDLMutex);
	SDL_LockSurface(screen);
	assert(screen->h == screenScaleUp*screenHeight);
	assert(screen->w == screenScaleUp*screenWidth);
	assert(screen->format->BytesPerPixel == 1);
	int y;
	for (y=0; y < screen->h; y++) {
		if (screenScaleUp == 1) {
			memcpy(screen->pixels+y*screen->pitch, screenBuffer+screenWidth*y, screenWidth);
		} else {
			int i, x, Y=y/screenScaleUp;
			unsigned char *dest = screen->pixels+y*screen->pitch;
			unsigned char *src = &screenBuffer[Y*screenWidth];
			for (x=0; x < screenWidth; x++) {
				for (i=0; i < screenScaleUp; i++) {
					*dest++ = *src;
				}
				src++;
			}
		}
	}	
	SDL_UnlockSurface(screen);
	assert(SDL_Flip(screen) == 0);
	pthread_mutex_unlock(&SDLMutex);
}

long BE_GetTics(void) {
	static long start=0;
	long t = SDL_GetTicks();
	if (!start) {
		start = t;
	}
	return 50*(t-start)/1000;
}

void BE_MusicOff(void) {
	SDL_LockAudio();
	musicOn = 0;
	SDL_UnlockAudio();
}

void BE_MusicOn(void) {
	SDL_LockAudio();
	musicOn = 1;
	SDL_UnlockAudio();
}

void BE_PlaySound(void *Sound) {
	SDL_LockAudio();
	int i;
	for (i = 0; i < MAXRENDEREDSOUNDS; i++) {
		if (Sound == renderedSoundsOriginal[i]) {
			curSnd = renderedSounds[i];
			curSndLength = renderedSoundsLength[i];
			break;
		} else if (!renderedSoundsOriginal[i]) {
			curSnd = renderSound((AdLibSound*)Sound, &curSndLength);
			renderedSoundsOriginal[i] = Sound;
			renderedSounds[i] = curSnd;
			renderedSoundsLength[i] = curSndLength;
			break;
		}
	}
	assert(curSnd != NULL);
	curSndHead = 0;
	SDL_UnlockAudio();
}

void BE_StartMusic(void *Music) {
	SDL_LockAudio();
	int i;
	for (i = 0; i < MAXRENDEREDSOUNDS; i++) {
		if (Music == renderedSoundsOriginal[i]) {
			curMusic = renderedSounds[i];
			curMusicLength = renderedSoundsLength[i];
			break;
		} else if (!renderedSoundsOriginal[i]) {
			curMusic = renderMusic((MusicGroup*)Music, &curMusicLength);
			renderedSoundsOriginal[i] = Music;
			renderedSounds[i] = curMusic;
			renderedSoundsLength[i] = curMusicLength;
			break;
		}
	}
	assert(curMusic != NULL);
	curMusicHead = 0;
	SDL_UnlockAudio();
}



// id_vw*

void VW_DrawPropString(char *String) {
	while (*String) {
		int w = drawPropChar(px, py, fontnumber, *(unsigned char*)String++);
		px += w;
	}
}


void VW_Hlin(unsigned xl, unsigned xh, unsigned y, unsigned color) {
	if (xh < 0 || xl >= screenWidth || xl > xh || y < 0 || y >= screenHeight) {
		return;
	}
	if (xl < 0) {
		xl = 0;
	}
	if (xh >= screenWidth) {
		xh = screenWidth-1;
	}
	memset(&screenBuffer[y*screenWidth+xl], color, xh+1-xl);
}


void VW_Bar (unsigned x, unsigned y, unsigned width, unsigned height,
	unsigned color)
{
	if (x < 0) {
		width += x;
		x = 0;
	} else if (x >= screenWidth) {
		width += screenWidth-1-x;
		x = screenWidth-1;
	}
	if (y < 0) {
		height += y;
		y = 0;
	} else if (y >= screenHeight) {
		height += screenHeight-1-y;
		y = screenHeight-1;
	}
	if (x+width > screenWidth) {
		width = screenWidth-x;
	}
	if (y+height > screenHeight) {
		height = screenHeight-y;
	}
	assert(x >= 0);
	assert(y >= 0);
	assert(width >= 0);
	assert(height >= 0);
	int i;
	for (i = 0; i < height; i++) {
		memset(&screenBuffer[(y+i)*screenWidth+x], color, width);
	}
}


void	VW_ClearVideo (int color)
{
#if 0
	SDL_Rect rect;
	rect.x = 0;
	rect.y = 0;
	rect.w = screenWidth;
	rect.h = screenHeight;
	pthread_mutex_lock(&SDLMutex);
	assert(SDL_FillRect(screen, &rect, color) == 0);
	pthread_mutex_unlock(&SDLMutex);
#else
	memset(screenBuffer, color, screenWidth*screenHeight);
#endif
}


void FizzleFade (unsigned width, unsigned height, boolean abortable)
{
	unsigned        x,y,p,frame;
	long            rndval;

	rndval = 1;
	y = 0;
	SDL_Rect rect;

	rect.w = 1;
	rect.h = 1;
	TimeCount=frame=0;
	do      // while (1)
	{
		if (abortable)
		{
			IN_ReadControl(0,&c);
			if (c.button0 || c.button1 || Keyboard[sc_Space] || Keyboard[sc_Enter])
			{
				BE_FlipBuffer();
				return;
			}
		}


//#define PIXPERFRAME     1600
#define PIXPERFRAME     1600
		for (p=0;p<PIXPERFRAME;p++)
		{
			y = (rndval&0xFF)-1;
			x = (rndval>>8)&0x1FF;
			if (rndval&1) {
				rndval = 
				rndval = (rndval^0x24000)/2;
			} else {
				rndval = rndval/2;
			}

			assert(x >= 0);
			assert(y >= 0);
			if (x>=width || y>=height)
				continue;

			
			flipPixel(x, y);

			if (rndval == 1)                // entire sequence has been completed
			{
				SDL_Flip(screen);
				return;
			};
		}
		SDL_Flip(screen);
		frame+=1;
		while (TimeCount<frame) {        // don't go too fast
		}
	} while (1);

}



void VW_ColorBorder (int color)
{
	bordercolor = color;
	setPalette();
}


void VW_SetDefaultColors(void)
{
	colors[3][16] = bordercolor;
	setPalette();
	screenfaded = false;
}


void VW_DrawTile8(unsigned X, unsigned Y, unsigned Tile) {
	byte *t = (byte*)grsegs[STARTTILE8]+32*Tile;
	int x,y;
	X*=8;
	for (y = 0; y < 8; y++) {
		unsigned int p0 = (unsigned int)t[0];
		unsigned int p1 = (unsigned int)t[8]<<1;
		unsigned int p2 = (unsigned int)t[16]<<2;
		unsigned int p3 = (unsigned int)t[24]<<3;
		t++;
		for (x = 0; x < 8; x++) {
			unsigned int c = (p3&8)+(p2&4)+(p1&2)+(p0&1);
			putPixel(X+8-x,Y+y, c);
			p0>>=1;
			p1>>=1;
			p2>>=1;
			p3>>=1;
		}
	}
}


void VW_MaskBlock(byte *Source, unsigned X, unsigned Y, unsigned Width, unsigned Height, unsigned PlaneSize)
{
	int x,y,i;
	byte *planes = Source;
	int bits = 0;
	unsigned int p0, p1, p2, p3, m;
	for (y = 0; y < Height; y++) {
		for (x = 0; x < Width; x++) {
			m = (unsigned int)planes[0];
			p0 = (unsigned int)planes[1*PlaneSize];
			p1 = (unsigned int)planes[2*PlaneSize]<<1;
			p2 = (unsigned int)planes[3*PlaneSize]<<2;
			p3 = (unsigned int)planes[4*PlaneSize]<<3;
			planes++;
			for (i = 0; i < 8; i++) {
				if (!(m&1)) {
					unsigned int c = (p3&8)+(p2&4)+(p1&2)+(p0&1);
					putPixel(X+8*x+8-i, Y+y, c);
				}
				m >>= 1;
				p0 >>= 1;
				p1 >>= 1;
				p2 >>= 1;
				p3 >>= 1;
			}
		}
	}
}


void VW_MemToScreen(byte *Source,unsigned X, unsigned Y,unsigned Width,unsigned Height) {
	int x,y,i;
	byte *planes = Source;
	int PlaneSize=Width*Height;
	unsigned int p0, p1, p2, p3;
	for (y = 0; y < Height; y++) {
		for (x = 0; x < Width; x++) {
			p0 = (unsigned int)planes[0*PlaneSize];
			p1 = ((unsigned int)planes[1*PlaneSize])<<1;
			p2 = ((unsigned int)planes[2*PlaneSize])<<2;
			p3 = ((unsigned int)planes[3*PlaneSize])<<3;
			planes++;
			for (i = 0; i < 8; i++) {
				unsigned int c = (p3&8)+(p2&4)+(p1&2)+(p0&1);
				putPixel(X+8*x+8-i, Y+y, c);
				p0 >>= 1;
				p1 >>= 1;
				p2 >>= 1;
				p3 >>= 1;
			}
		}
	}
}


void VW_MemToScreen2(byte *Source,unsigned X, unsigned Y,unsigned Width,unsigned Height, unsigned PlaneSize) {
	if (X+Width <= 0 || Y+Height <= 0 || X >= 320 || Y >= 200) {
		return;
	}

	int x,y,i;
	byte *planes = Source;
	unsigned int p0, p1, p2, p3;

	for (y = 0; y < Height; y++) {
		for (x = 0; x < Width; x++) {
			p0 = (unsigned int)planes[0*PlaneSize];
			p1 = ((unsigned int)planes[1*PlaneSize])<<1;
			p2 = ((unsigned int)planes[2*PlaneSize])<<2;
			p3 = ((unsigned int)planes[3*PlaneSize])<<3;
			planes++;
			for (i = 0; i < 8; i++) {
				unsigned int c = (p3&8)+(p2&4)+(p1&2)+(p0&1);
				putPixel(X+8*x+8-i, Y+y, c);
				p0 >>= 1;
				p1 >>= 1;
				p2 >>= 1;
				p3 >>= 1;
			}
		}
	}
}




// c3_asm.asm

unsigned short wallheight[VIEWWIDTH];
unsigned short wallwidth[VIEWWIDTH];
memptr wallseg[VIEWWIDTH];
int wallofs[VIEWWIDTH];

//long wallscalecall[65*6];


void	ScaleWalls (void) {
	int j, k, x, w;
	w = 1;
	for (x = 0; x < VIEWWIDTH; x += w) {
		w = wallwidth[x];
		if (w <= 0) {
			w = 2;
			continue;
		}
		int h = wallheight[x];
		assert(h >= 0);
		assert(h < MAXSCALE+1);
		t_compscale *scale = scaledirectory[h];
		byte *wallsrc = wallseg[x]+wallofs[x];

		int srcy;
		for (srcy = 0; srcy < 64; srcy++) {
			int col = *wallsrc++;
			int sy = scale->starty[srcy];
			int ey = scale->endy[srcy];
			if (sy < 0) {
				sy = 0;
			}
			if (sy < ey) {
				VW_Bar(x, sy, w, ey-sy, col);
			}
		}
	}
}


// id_us_a.asm

static unsigned rndindex=0;

static unsigned rndtable[] = {0,   8, 109, 220, 222, 241, 149, 107,  75, 248, 254, 140,  16,  66,
	74,  21, 211,  47,  80, 242, 154,  27, 205, 128, 161,  89,  77,  36,
	95, 110,  85,  48, 212, 140, 211, 249,  22,  79, 200,  50,  28, 188,
	52, 140, 202, 120,  68, 145,  62,  70, 184, 190,  91, 197, 152, 224,
	149, 104,  25, 178, 252, 182, 202, 182, 141, 197,   4,  81, 181, 242,
	145,  42,  39, 227, 156, 198, 225, 193, 219,  93, 122, 175, 249,   0,
	175, 143,  70, 239,  46, 246, 163,  53, 163, 109, 168, 135,   2, 235,
	25,  92,  20, 145, 138,  77,  69, 166,  78, 176, 173, 212, 166, 113,
	94, 161,  41,  50, 239,  49, 111, 164,  70,  60,   2,  37, 171,  75,
	136, 156,  11,  56,  42, 146, 138, 229,  73, 146,  77,  61,  98, 196,
	135, 106,  63, 197, 195,  86,  96, 203, 113, 101, 170, 247, 181, 113,
	80, 250, 108,   7, 255, 237, 129, 226,  79, 107, 112, 166, 103, 241,
	24, 223, 239, 120, 198,  58,  60,  82, 128,   3, 184,  66, 143, 224,
	145, 224,  81, 206, 163,  45,  63,  90, 168, 114,  59,  33, 159,  95,
	28, 139, 123,  98, 125, 196,  15,  70, 194, 253,  54,  14, 109, 226,
	71,  17, 161,  93, 186,  87, 244, 138,  20,  52, 123, 251,  26,  36,
	17,  46,  52, 231, 232,  76,  31, 221,  84,  37, 216, 165, 212, 106,
	197, 242,  98,  43,  39, 175, 254, 145, 190,  84, 118, 222, 187, 136,
	120, 163, 236, 249 };


unsigned LastRnd;

void US_InitRndT (boolean randomize) {
	if (randomize) {
		rndindex = time(NULL);
	} else {
		rndindex = 0;
	}
}

int US_RndT(void) {
	return rndtable[(rndindex++)&0xFF];
}


// id_vw_a.asm

void	VW_WaitVBL (int number) {
	long tics = BE_GetTics()+number;
	VW_UpdateScreen();
	while (BE_GetTics() < tics) {
	}
}


// id_vw_ae.asm

extern	unsigned	bufferwidth,bufferheight,screenspot;	// used by font drawing stuff
int			px,py;
byte		pdrawmode,fontcolor;

void VW_Plot(unsigned x, unsigned y, unsigned color) {
	if (x < 0 || y < 0 || x >= screenWidth || y >= screenHeight) {
		return;
	}
	putPixel(x, y, color);
}


void VW_Vlin(unsigned yl, unsigned yh, unsigned x, unsigned color) {
	for (; yl <= yh; yl++) {
		VW_Plot(x, yl, color);
	}
}



