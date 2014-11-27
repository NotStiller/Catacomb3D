#include "c_win.h"

static int winX, winY, winW, winH;
static int winPrintX, winPrintY;
static int winFontColor = 0;
static BufferSetup *winBuffer = NULL;

void	Win_Create(BufferSetup *Buffer, int w,int h) {
printf("Win_Create(%p, %i,%i)\n", Buffer, w, h);
	winBuffer = Buffer;

	w *= 8;
	h *= 8;

	int x = (winBuffer->Width - w) / 2;
	int y = (winBuffer->Height - h) / 2;


	int    i,sx,sy,sw,sh;

	winX = x;
	winY = y;
	winW = w;
	winH = h;

	winPrintX = winX;
	winPrintY = winY;

	sx = x - 8;
	sy = y - 8;
	sw = w + 8;
	sh = h + 8;

	SPG_Bar(winBuffer, x, y, w, h, 7);

	SPG_DrawTile8(winBuffer, sx,sy,(byte*)grsegs[StartTile8M]+0*64);
	SPG_DrawTile8(winBuffer, sx,sy+sh,(byte*)grsegs[StartTile8M]+6*64);

	for (i = sx + 8;i <= sx + sw - 8;i += 8) {
		SPG_DrawTile8(winBuffer, i,sy,(byte*)grsegs[StartTile8M]+1*64);
		SPG_DrawTile8(winBuffer, i,sy+sh,(byte*)grsegs[StartTile8M]+7*64);
/*		VW_DrawTile8M(i/8,sy,1);
		VW_DrawTile8M(i/8,sy + sh,7);*/
	}
	SPG_DrawTile8(winBuffer, i,sy,(byte*)grsegs[StartTile8M]+2*64);
	SPG_DrawTile8(winBuffer, i,sy+sh,(byte*)grsegs[StartTile8M]+8*64);
/*	VW_DrawTile8M(i/8,sy,2);
	VW_DrawTile8M(i/8,sy + sh,8);*/

	for (i = sy + 8;i <= sy + sh - 8;i += 8) {
	SPG_DrawTile8(winBuffer, sx,i,(byte*)grsegs[StartTile8M]+3*64);
	SPG_DrawTile8(winBuffer, sx+sw,i,(byte*)grsegs[StartTile8M]+5*64);
/*		VW_DrawTile8M(sx/8,i,3);
		VW_DrawTile8M((sx + sw)/8,i,5);*/
	}

	winFontColor = fontcolor^7;
}

void Win_Clear(BufferSetup *Buffer, int X, int Y, int Width, int Height, int BGColor, int FontColor) {
	winBuffer = Buffer;

	winX = X;
	winY = Y;
	winW = Width;
	winH = Height;

	winPrintX = X;
	winPrintY = Y;

	SPG_Bar(winBuffer, winX, winY, winW, winH, BGColor);
	winFontColor = FontColor;
}


void Win_Print(char *s) {
	char    c,*se;
	int    w,h;

	while (*s) {
		se = s;
		while ((c = *se) && (c != '\n')) {
			se++;
		}
		*se = '\0';

		SPG_MeasureString(s,0,&w,&h);
		SPG_DrawString(winBuffer, winPrintX, winPrintY, s, 0, winFontColor);

		s = se;
		if (c) {
			*se = c;
			s++;

			winPrintX = WindowX;
			winPrintY += h;
		}
		else {
			winPrintX += w;
		}
	}
}

void Win_PrintCentered(char *s) {
	int w,h;

	SPG_MeasureString(s,0,&w,&h);
	SPG_DrawString(winBuffer, winX + ((winW - w) / 2), winY + ((winH - h) / 2), s, 0, winFontColor);

}

void Win_CPrintLine(char *s) {
	int w,h;

	SPG_MeasureString(s,0,&w,&h);
	assert(w <= winW);
	SPG_DrawString(winBuffer, winX + ((winW - w) / 2), winPrintY, s, 0, winFontColor);
	winPrintY += h;
}

void Win_CPrint(char *S) {
	char str[strlen(S)+1];
	strcpy(str, S);
	char *s = str;
	char    c,*se;

	while (*s)
	{
		se = s;
		while ((c = *se) && (c != '\n')) {
			se++;
		}
		*se = '\0';

		Win_CPrintLine(s);

		s = se;
		if (c) {
			*se = c;
			s++;
		}
	}
}


