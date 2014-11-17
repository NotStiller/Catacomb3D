/* Catacomb Abyss Source Code
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

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "c4_def.h"
#include "gelib.h"

#define MAX_GAMELIST_NAMES 20
#define FNAME_LEN				9

////////////////////////////////////////////////////////////////////////////
//
// Global variables
//
boolean InLoadSaveGame = false;
//AudioDeviceType ge_DigiMode;
boolean ConserveMemory = false;
char GameListNames[MAX_GAMELIST_NAMES+1][FNAME_LEN],current_disk=1;
short NumGames=0;
short PPT_LeftEdge=0,PPT_RightEdge=320;
boolean LeaveDriveOn=false,ge_textmode=true;
char Filename[FILENAME_LEN+1], ID[sizeof(GAMENAME)], VER[sizeof(SAVEVER_DATA)];


//--------------------------------------------------------------------------
// PrintPropText()
//--------------------------------------------------------------------------

// THE FOLLOWING MUST BE INITIALIZED BEFORE CALLING THIS ROUTINE:
//

// WindowX, WindowW, PrintY - These are ID global variables and are
// automatically initialized when using their window routines.
//
// WindowX is the left edge of the window.
// WindowW is the width of the window.
// PrintY is the top edge of the window area.
//
// All values are represented in unshifted pixels.

// PPT_LeftEdge, PPT_RightEdge - These are globals used by PrintPropText().
// They define the left and right edge of the text area in pixels.

void PrintPropText(char *text)
{
	#define RETURN_CHAR '\n'

	char pb[200];

	fontstruct *font = (fontstruct *)grsegs[STARTFONT];
	char savech;
	short	length,maxend,maxx,loop,curx;
	boolean centerit,lastcharcr;

	while (*text)
	{
		if (*text == '^')
			centerit=true,text++;
		else
			centerit=false;

		/* Search forward for the last possible character in the line. This
		** character is:  1) RETURN (end of line)  2) ZERO (end of buffer)
		** and  3) the character at "WP->vWidth" bytes from the start of the line.
		*/
		curx=PPT_LeftEdge;
		length=0;
		while ((curx+font->width[text[length+1]] < PPT_RightEdge) &&
				(text[length]) &&
				(text[length] != RETURN_CHAR))
						curx+=font->width[text[length++]];

		/* Search backward from the point we just found for a SPACE (for word
		** wrapping).
		*/
		if ((text[length]) && (text[length] != RETURN_CHAR))
		{
			maxx = curx;
			maxend = length;
			while ((length) && (text[length] != ' '))
				curx-=font->width[text[length--]];

			/* Were there any SPACES on this line? If not, take the MAX!
			*/
			if (!length)
				length=maxend,curx=maxx;
		}

		/* If we can, lets keep the SPACE or RETURN that follows a line at
		** the end of that line.
		*/
		if (((text[length] == ' ') || (text[length] == RETURN_CHAR)) && (length < PPT_RightEdge))
			length++;

		// All of this is kludged to work with ID _Print routines...
		//
		savech=text[length];
		text[length]=0;
		if (text[length-1] == RETURN_CHAR)
		{
			lastcharcr=true;
			text[length-1]=0;
		}
		else
			lastcharcr=false;
		memcpy(pb,text,length+1);
		if (centerit)
		{
			US_CPrintLine(pb);
		}
		else
		{
			PrintX = PPT_LeftEdge;
			US_Print(pb);
			US_Print("\n");
		}
		if (lastcharcr)
			text[length-1]=RETURN_CHAR;
		text[length]=savech;
		//
		// end of ID _Print kludge...

		text += length;
	}
}

////////////////////////////////////////////////////////////////////////////
//
// DisplayText()
//
void DisplayText(textinfo *textinfo)
{
printf("implement DisplayText\n");
#if 0
	#define PAGE_WIDTH 	78

	int loop, PageNum, LastNum,num;
	boolean InHelp = true,faded_in = false;
	unsigned holddisp,holdpan,holdbuffer,holdaddress;

// Can you believe it takes all this just to change to 640 mode!!???!
//
	VW_ScreenToScreen(0,FREESTART-STATUSLEN,40,80);
	VW_SetLineWidth(80);
	VW_Bar (0,0,640,200,0);
	VW_SetScreenMode(EGA640GR);
	VW_SetLineWidth(80);
	BlackPalette();

// Now, let's display some text...
//
	PPT_RightEdge=PAGE_WIDTH*8;
	PPT_LeftEdge=16;
	PrintY= 30;
	WindowX=WindowY=0;

	LastNum = -1;
	PageNum = 1;
	while (InHelp)
	{
		// Display new page of text.
		//
		if (PageNum != LastNum)
		{
			US_DrawWindow(1,1,PAGE_WIDTH,23);
			PrintPropText(textinfo->pages[PageNum-1]);
			LastNum = PageNum;
		}

		SPG_FlipBuffer();
		if (!faded_in)
		{
			VW_FadeIn();
			faded_in = true;
		}

		// Scroll through text / exit.
		//
		IN_ReadControl(0,&control);
		if (control.button1 || SP_Keyboard(1))
			InHelp=false;
		else
		{
#warning are there other control schemes than keyboard ?
/*			if (ControlTypeUsed != ctrl_Keyboard)
				control.dir = dir_None;*/

			if (((control.dir == dir_North) || (control.dir == dir_West)) && (PageNum > 1))
			{
				PageNum--;
				while ((control.dir == dir_North) || (control.dir == dir_West))
					IN_ReadControl(0,&control);
			}
			else
				if (((control.dir == dir_South) || (control.dir == dir_East)) && (PageNum < textinfo->totalpages))
				{
					PageNum++;
					while ((control.dir == dir_South) || (control.dir == dir_East))
						IN_ReadControl(0,&control);
				}
		}
	}

	// Wait for 'exit key' to be released.
	//
	while (control.button1 || SP_Keyboard(1))
		IN_ReadControl(0,&control);

// Can you believe it takes all this just to change to 320 mode!!???!
//
	VW_FadeOut();
	VW_SetLineWidth(40);
	VW_Bar (0,0,320,200,0);
	VW_SetScreenMode(EGA320GR);
	BlackPalette();
	VW_ScreenToScreen(FREESTART-STATUSLEN,0,40,80);
#endif
}

//--------------------------------------------------------------------------
// BlackPalette()
//--------------------------------------------------------------------------
void BlackPalette()
{
	printf("Change palette to black!\n");
	screenfaded = true;
}

//--------------------------------------------------------------------------
// ColoredPalette()
//--------------------------------------------------------------------------
void ColoredPalette()
{
	printf("Change palette to normal!\n");
	screenfaded = false;
}

////////////////////////////////////////////////////////////////////////////
//
// Verify()
//
long Verify(char *filename)
{
	long size;
	FILE *f;
	f = fopen(filename, "rb");
	if (f == NULL) {
		return 0;
	}
	fseek(f, 0, SEEK_END);
	fclose(f);
	size = ftell(f);

	return(size);
}

///////////////////////////////////////////////////////////////////////////
//
//	GE_SaveGame
//
//	Handles user i/o for saving a game
//
///////////////////////////////////////////////////////////////////////////

void GE_SaveGame()
{
printf("implement GE_SaveGame!\n");
return;
#if 0
	boolean GettingFilename=true;
	char drive;
//	char Filename[FILENAME_LEN+1],drive; //, ID[sizeof(GAMENAME)], VER[sizeof(SAVEVER_DATA)];
	FILE *f;
	struct dfree dfree;
	long davail;

	VW_FixRefreshBuffer();
	ReadGameList();
	while (GettingFilename)
	{
		DisplayGameList(2,7,3,10);
		US_DrawWindow(5,1,30,3);
		memset(Filename,0,sizeof(Filename));
		US_CPrint("Enter name to SAVE this game:");
		SPG_FlipBuffer();
		if (screenfaded)
			VW_FadeIn();
		if (!US_LineInput((linewidth<<2)-32,20,Filename,"",true,8,0))
			goto EXIT_FUNC;
		if (!strlen(Filename))
			goto EXIT_FUNC;

		drive = getdisk();
		getdfree(drive+1,&dfree);
		davail = (long)dfree.df_avail*(long)dfree.df_bsec*(long)dfree.df_sclus;

		if (davail < 10000l)
		{
			char status[40] = "\nDrive:     Free: ";

			US_CenterWindow(30,6);
			US_Print("\n");
			US_CPrintLine("Disk Full: Can't save game.");
			US_CPrintLine("Try inserting another disk.");
			status[8] = drive+'A';
			itoa(davail,&status[18],10);
			US_CPrint(status);
			SPG_FlipBuffer();

			IN_Ack();
		}
		else
		{
			strcat(Filename,".SAV");
			GettingFilename = false;
			if (Verify(Filename))								// FILE EXISTS
			{
				US_CenterWindow(22,4);
				US_CPrintLine("That file already exists...");
				US_CPrintLine("Overwrite it ????");
				US_CPrintLine("(Y)es or (N)o?");
				SPG_FlipBuffer();

				while((!SP_Keyboard(21)) && (!SP_Keyboard(49)) && !SP_Keyboard(27));

				if (SP_Keyboard(27))
					goto EXIT_FUNC;
				if (SP_Keyboard(49))
				{
					GettingFilename = true;
					SPG_FlipBuffer();
				}
			}
		}
	}

	f = fopen(Filename, "wb");
	if (f == NULL)
		goto EXIT_FUNC;

	if ((!CA_FarWrite(f,(void *)GAMENAME,sizeof(GAMENAME))) || (!CA_FarWrite(f,(void *)SAVEVER_DATA,sizeof(SAVEVER_DATA))))
	{
		if (!screenfaded)
			VW_FadeOut();

		return;
	}

	if (!USL_SaveGame(f))
		Quit("Save game error");



EXIT_FUNC:;

	if (f != NULL) {
		fclose(f);
	}

	if (f==NULL)
	{
		remove(Filename);
		US_CenterWindow(22,6);
		US_CPrintLine("DISK ERROR");
		US_CPrintLine("Check: Write protect...");
		US_CPrintLine("File name...");
		US_CPrintLine("Bytes free on disk...");
		US_CPrintLine("Press SPACE to continue.");
		SPG_FlipBuffer();
		while (!SP_Keyboard(57));
		while (SP_Keyboard(57));
	}

	while (SP_Keyboard(1));

	if (!screenfaded)
		VW_FadeOut();
#endif
}


///////////////////////////////////////////////////////////////////////////
//
//	GE_LoadGame
//
//	Handles user i/o for loading a game
//
///////////////////////////////////////////////////////////////////////////

boolean GE_LoadGame()
{
printf("implement GE_LoadGame!\n");
return false;
#if 0
	boolean GettingFilename=true,rt_code=false;
	int handle;

	IN_ClearKeysDown();
	memset(ID,0,sizeof(ID));
	memset(VER,0,sizeof(VER));
	VW_FixRefreshBuffer();
	ReadGameList();
	while (GettingFilename)
	{
		DisplayGameList(2,7,3,10);
		US_DrawWindow(5,1,30,3);
		memset(Filename,0,sizeof(Filename));
		US_CPrint("Enter name of game to RESTORE:");
		SPG_FlipBuffer();
		if (screenfaded)
			VW_FadeIn();
		if (!US_LineInput((linewidth<<2)-32,20,Filename,"",true,8,0))
			goto EXIT_FUNC;
		strcat(Filename,".SAV");
		GettingFilename = false;

		if (!Verify(Filename))								// FILE DOESN'T EXIST
		{
			US_CenterWindow(22,3);
			US_CPrintLine(" That file doesn't exist....");
			US_CPrintLine("Press SPACE to try again.");
			SPG_FlipBuffer();

			while (!SP_Keyboard(57));
			while (SP_Keyboard(57));
			GettingFilename = true;
		}
	}

	handle = open(Filename,O_RDWR|O_BINARY);
	if (handle==-1)
		goto EXIT_FUNC;

	if ((!CA_FarRead(handle,(void *)&ID,sizeof(ID))) || (!CA_FarRead(handle,(void *)&VER,sizeof(VER))))
		return(false);

	if ((strcmp(ID,GAMENAME)) || (strcmp(VER,SAVEVER_DATA)))
	{
		US_CenterWindow(32,4);
		US_CPrintLine("That isn't a "GAMENAME);
		US_CPrintLine(".SAV file.");
		US_CPrintLine("Press SPACE to continue.");
		SPG_FlipBuffer();
		while (!SP_Keyboard(57));
		while (SP_Keyboard(57));

		if (!screenfaded)
			VW_FadeOut();

		return(false);
	}

	if (!USL_LoadGame(handle))
		Quit("Load game error.");

	rt_code = true;


EXIT_FUNC:;
	if (handle==-1)
	{
		US_CenterWindow(22,3);
		US_CPrintLine("DISK ERROR ** LOAD **");
		US_CPrintLine("Press SPACE to continue.");
		while (!SP_Keyboard(57));
		while (SP_Keyboard(57));
	}
	else
		close(handle);

	if (!screenfaded)
		VW_FadeOut();

	return(rt_code);
#endif
}


//==========================================================================
// JAMPAK routines
//==========================================================================
#define N		4096
#define F		18

// THRESHOLD : encode string into position and length if match_length is
// greater than this

#define THRESHOLD				2

// index for root of binary search trees
//

#define NIL       			N

unsigned long 	textsize = 0,  		// text size counter
					codesize = 0,			// code size counter
					printcount = 0;     	// counter for reporting progress every 1K bytes

unsigned char text_buf[N + F - 1];

memptr segptr;

////////////////////////////////////////////////////////////////////////////
//
// GetKeyChoice()
//
char GetKeyChoice(char *choices,boolean clear)
{
	extern void DoEvents(void);

	boolean waiting;
	char *s,*ss;

	IN_ClearKeysDown();

	waiting = true;
	while (waiting)
	{
		s = choices;
		while (*s)
		{
			if (SP_Keyboard(*s++))
			{
				waiting=false;
				break;
			}
		}
	}

	IN_ClearKeysDown();

	return(*(--s));
}

///////////////////////////////////////////////////////////////////////////
//
// DisplayGameList()
//
void DisplayGameList(short winx, short winy, short list_width, short list_height)
{
	#define SPACES 2

	short width,col,row,orgcol,games_printed=0,h;

	// Possibly shrink window.
	//
	h = (NumGames / list_width) + ((NumGames % list_width) > 0);
	if (h < list_height)
		list_height = h;

	// Open window and print header...
	//
	US_DrawWindow(winx,winy,list_width*(8+SPACES*2),list_height+3);
	US_CPrintLine("LIST OF SAVED GAMES");
	US_Print("\n");

	col = orgcol = PrintX;
	row = PrintY;

	// Display as many 'save game' files as can fit in the window.
	//
	width = list_width;
	while ((games_printed<NumGames) && (list_height))
	{
		// Print filename and padding spaces.
		//
		US_Printxy(col+(SPACES*8),row,GameListNames[games_printed]);
		col += 8*((SPACES*2)+8);

		// Check for end-of-line or end-of-window.
		//
		width--;
		if (!width)
		{
			col = orgcol;
			row += 8;
			width = list_width;
			list_height--;
			US_Print("\n");
		}

		games_printed++;
	}
}

////////////////////////////////////////////////////////////////////////////
//
// ReadGameList()
//
void ReadGameList()
{
printf("implement ReadGameList!\n");
return;
#if 0
	struct ffblk ffblk;
	short done,len;

	NumGames = -1;
	done = findfirst("*.sav",&ffblk,0);

	while (!done)
	{
		if (NumGames == MAX_GAMELIST_NAMES)
			memcpy(GameListNames,GameListNames[1],MAX_GAMELIST_NAMES*sizeof(GameListNames[0]));
		else
			NumGames++;

		fnsplit(ffblk.ff_name,NULL,NULL,GameListNames[NumGames],NULL);

		done=findnext(&ffblk);
	}

	NumGames++;
#endif
}

////////////////////////////////////////////////////////////////////////////
//
// LoadTextFile()
//
long LoadTextFile(char *filename,textinfo *textinfo)
{
	long size;
	FILE *f;
	f = fopen(filename, "rb");
	assert(f != NULL);
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	assert(size > 0);
	fseek(f, 0, SEEK_SET);
	MM_GetPtr (&textinfo->textptr,size);
	assert(fread(textinfo->textptr, 1, size, f) == size);
	fclose(f);
	InitTextFile(textinfo);

	return(size);
}

//-------------------------------------------------------------------------
// FreeTextFile()
//-------------------------------------------------------------------------
void FreeTextFile(textinfo *textinfo)
{
	if (textinfo->textptr)
		MM_FreePtr(&textinfo->textptr);
}

////////////////////////////////////////////////////////////////////////////
//
// InitTextFile()
//
void InitTextFile(textinfo *textinfo)
{
	#define END_PAGE  '@'

	char *text = *(char**)textinfo->textptr;

	textinfo->totalpages = 0;
	while (*text != END_PAGE)
	{
		if (textinfo->totalpages < MAX_TEXT_PAGES)
			textinfo->pages[textinfo->totalpages++] = text;
		else {
			char string[100];
			sprintf(string, "GE ERROR: Too many text pages. --> %d",textinfo->totalpages);
			Quit(string);
		}

		while (*text != END_PAGE)
		{
			if ((*text == '\r') && (*(text+1) == '\n'))
			{
				*text = 32;
				*(text+1) = '\n';
				text+=2;
			}
			else
				text++;
		}
		*text = 0;
		text += 3;
	}
	*text = 0;
}



#if 0
/*
===================
=
= FizzleFade
=
===================
*/

#define PIXPERFRAME     10000	//1600

void FizzleFade (unsigned source, unsigned dest,
	unsigned width,unsigned height, boolean abortable)
{
	unsigned        drawofs,pagedelta;
	unsigned        char maskb[8] = {1,2,4,8,16,32,64,128};
	unsigned        x,y,p,frame;
	long            rndval;
//	ScanCode			 lastLastScan=LastScan=0;
	ScanCode			 lastLastScan=0;

	width--;
	height--;

	pagedelta = dest-source;
//	VW_SetScreen (dest,0);
	rndval = 1;
	y = 0;

asm     mov     es,[screenseg]
asm     mov     dx,SC_INDEX
asm     mov     al,SC_MAPMASK
asm     out     dx,al

	TimeCount=frame=0;
	do      // while (1)
	{
		if ((abortable) || (Flags & FL_QUICK))
		{
			IN_ReadControl(0,&control);
			if (control.button0 || control.button1 || (lastLastScan != SP_LastScan())
			|| SP_Keyboard(sc_Escape) || (Flags & FL_QUICK))
			{
				VW_ScreenToScreen (source,dest,(width+1)/8,height+1);
				goto exitfunc;
			}
		}

		for (p=0;p<PIXPERFRAME;p++)
		{
			//
			// seperate random value into x/y pair
			//
			asm     mov     ax,[WORD PTR rndval]
			asm     mov     dx,[WORD PTR rndval+2]
			asm     mov     bx,ax
			asm     dec     bl
			asm     mov     [BYTE PTR y],bl                 // low 8 bits - 1 = y xoordinate
			asm     mov     bx,ax
			asm     mov     cx,dx
			asm     shr     cx,1
			asm     rcr     bx,1
			asm     shr     bx,1
			asm     shr     bx,1
			asm     shr     bx,1
			asm     shr     bx,1
			asm     shr     bx,1
			asm     shr     bx,1
			asm     shr     bx,1
			asm     mov     [x],bx                                  // next 9 bits = x xoordinate
			//
			// advance to next random element
			//
			asm     shr     dx,1
			asm     rcr     ax,1
			asm     jnc     noxor
			asm     xor     dx,0x0001
			asm     xor     ax,0x2000
noxor:
			asm     mov     [WORD PTR rndval],ax
			asm     mov     [WORD PTR rndval+2],dx

			if (x>width || y>height)
				continue;
			drawofs = source+ylookup[y];

			asm     mov     cx,[x]
			asm     mov     si,cx
			asm     and     si,7
			asm     mov dx,GC_INDEX
			asm     mov al,GC_BITMASK
			asm     mov     ah,BYTE PTR [maskb+si]
			asm     out dx,ax

			asm     mov     si,[drawofs]
			asm     shr     cx,1
			asm     shr     cx,1
			asm     shr     cx,1
			asm     add     si,cx
			asm     mov     di,si
			asm     add     di,[pagedelta]

			asm     mov     dx,GC_INDEX
			asm     mov     al,GC_READMAP                   // leave GC_INDEX set to READMAP
			asm     out     dx,al

			asm     mov     dx,SC_INDEX+1
			asm     mov     al,1
			asm     out     dx,al
			asm     mov     dx,GC_INDEX+1
			asm     mov     al,0
			asm     out     dx,al

			asm     mov     bl,[es:si]
			asm     xchg [es:di],bl

			asm     mov     dx,SC_INDEX+1
			asm     mov     al,2
			asm     out     dx,al
			asm     mov     dx,GC_INDEX+1
			asm     mov     al,1
			asm     out     dx,al

			asm     mov     bl,[es:si]
			asm     xchg [es:di],bl

			asm     mov     dx,SC_INDEX+1
			asm     mov     al,4
			asm     out     dx,al
			asm     mov     dx,GC_INDEX+1
			asm     mov     al,2
			asm     out     dx,al

			asm     mov     bl,[es:si]
			asm     xchg [es:di],bl

			asm     mov     dx,SC_INDEX+1
			asm     mov     al,8
			asm     out     dx,al
			asm     mov     dx,GC_INDEX+1
			asm     mov     al,3
			asm     out     dx,al

			asm     mov     bl,[es:si]
			asm     xchg [es:di],bl

			if (rndval == 1)                // entire sequence has been completed
				goto exitfunc;
		}
		frame++;
//		while (TimeCount<frame)         // don't go too fast
//		;
	} while (1);

exitfunc:;
	EGABITMASK(255);
	EGAMAPMASK(15);
	return;
}
#endif

//--------------------------------------------------------------------------
// FindFile()
//--------------------------------------------------------------------------
boolean FindFile(char *filename,char *disktext,char disknum)
{
	if (Verify(filename))
		return true;
	return false;
}


