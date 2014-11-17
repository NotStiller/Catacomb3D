#ifndef C4_WIN_H
#define C4_WIN_H

#include "id_heads.h"


void Win_Create(BufferSetup *Buffer, int Width,int Height);
void Win_Clear(BufferSetup *Buffer, int X, int Y, int Width,int Height, int BGColor, int FontColor);
void Win_CPrint(char *Text);
void Win_CPrintLine(char *Text);
void Win_Print(char *Text);
void Win_PrintCentered(char *Text);


#endif

