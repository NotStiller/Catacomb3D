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
//      ID Engine
//      ID_US_1.c - User Manager - General routines
//      v1.1d1
//      By Jason Blochowiak
//      Hacked up for Catacomb 3D
//

//
//      This module handles dealing with user input & feedback
//
//      Depends on: Input Mgr, View Mgr, some variables from the Sound, Caching,
//              and Refresh Mgrs, Memory Mgr for background save/restore
//
//      Globals:
//              ingame - Flag set by game indicating if a game is in progress
//      abortgame - Flag set if the current game should be aborted (if a load
//                      game fails)
//              loadedgame - Flag set if a game was loaded
//              abortprogram - Normally nil, this points to a terminal error message
//                      if the program needs to abort
//              restartgame - Normally set to gd_Continue, this is set to one of the
//                      difficulty levels if a new game should be started
//              PrintX, PrintY - Where the User Mgr will print (global coords)
//              WindowX,WindowY,WindowW,WindowH - The dimensions of the current
//                      window
//

#include "id_heads.h"

//      Special imports
		ScanCode        firescan;

//      Global variables
		char            *abortprogram;
		boolean         NoWait;
		word            PrintX,PrintY;
		word            WindowX,WindowY,WindowW,WindowH;

//      Internal variables
#define ConfigVersion   1

static  char            *ParmStrings[] = {"NOWAIT"};
static  boolean         US_Started;

		boolean         Button0,Button1,
					CursorBad;

		void            (*USL_MeasureString)(char *,word *,word *) = VW_MeasurePropString,
					(*USL_DrawString)(char *) = VW_DrawPropString;

		boolean         (*USL_SaveGame)(FILE*),(*USL_LoadGame)(FILE*, gametype*);
		void            (*USL_ResetGame)(void);
		SaveGame        Games[MaxSaveGames];
		HighScore       Scores[MaxScores] =
					{
						{"Sir Lancelot",500,3},
						{"",0},
						{"",0},
						{"",0},
						{"",0},
						{"",0},
						{"",0},
					};

//      Internal routines

//      Public routines

///////////////////////////////////////////////////////////////////////////
//
//      USL_GiveSaveName() - Returns a pointer to a static buffer that contains
//              the filename to use for the specified save game
//
///////////////////////////////////////////////////////////////////////////
char *
USL_GiveSaveName(word game)
{
	static char name[1000];
	snprintf(name, 1000, "SAVEGAM%1i.%s", game, GamespecificExtension);
	return(name);
}

///////////////////////////////////////////////////////////////////////////
//
//      US_SetSaveHook() - Sets the routines that the User Mgr calls after
//              writing the save game headers
//
///////////////////////////////////////////////////////////////////////////
void
US_SetSaveHook(boolean (*save)(FILE*))
{
	USL_SaveGame = save;
}

///////////////////////////////////////////////////////////////////////////
//
//      USL_ReadConfig() - Reads the configuration file, if present, and sets
//              things up accordingly. If it's not present, uses defaults. This file
//              includes the high scores.
//
///////////////////////////////////////////////////////////////////////////
static void
USL_ReadConfig(void)
{
	boolean         gotit;
	char            sig[strlen(GamespecificExtension)+1];
	word            version;
	FILE			*file;
	SoundSource     sd;
	SoundSource     sm;

	char name[1000];
	snprintf(name, 1000, "CONFIG.%s", GamespecificExtension);

	if (file = fopen(name,"rb"))
	{
		fread(sig,1,strlen(GamespecificExtension)+1,file);
		fread(&version,1,sizeof(version),file);
		if (strcmp(sig,GamespecificExtension) || (version != ConfigVersion))
		{
			fclose(file);
			printf("Failed to read config !\n");
		}
		fread(Scores,1,sizeof(HighScore) * MaxScores,file);
		fread(&sd,1,sizeof(SoundSource),file);
		fread(&sm,1,sizeof(SoundSource),file);
		fread(&KbdDefs,1,sizeof(KbdDefs),file);
		fclose(file);

		SPA_SetSoundSource(sd);
		SPA_SetMusicSource(sm);
	}
}

///////////////////////////////////////////////////////////////////////////
//
//      USL_WriteConfig() - Writes out the current configuration, including the
//              high scores.
//
///////////////////////////////////////////////////////////////////////////
static void
USL_WriteConfig(void)
{
	word    version;
	FILE *file;
	SoundSource SoundMode, MusicMode;


	SoundMode = SPA_GetSoundSource();
	MusicMode = SPA_GetMusicSource();

	version = ConfigVersion;
	char name[1000];
	snprintf(name, 1000, "CONFIG.%s", GamespecificExtension);
	file = fopen(name,"wb");
	if (file != NULL)
	{
		fwrite(GamespecificExtension,1,strlen(GamespecificExtension)+1,file);
		fwrite(&version,1,sizeof(version),file);
		fwrite(Scores,1,sizeof(HighScore) * MaxScores,file);
		fwrite(&SoundMode,1,sizeof(SoundSource),file);
		fwrite(&MusicMode,1,sizeof(SoundSource),file);
		fwrite(&KbdDefs,1,sizeof(KbdDefs),file);
		fclose(file);
	}
}

///////////////////////////////////////////////////////////////////////////
//
//      USL_CheckSavedGames() - Checks to see which saved games are present
//              & valid
//
///////////////////////////////////////////////////////////////////////////
static void
USL_CheckSavedGames(void)
{
	boolean         ok;
	char            *filename;
	word            i;
	FILE			*file;
	SaveGame        *game;

	USL_SaveGame = 0;
	USL_LoadGame = 0;

	for (i = 0,game = Games;i < MaxSaveGames;i++,game++)
	{
		filename = USL_GiveSaveName(i);
		ok = false;
		if (file = fopen(filename,"rb"))
		{
			if
			(
				(fread(game,1,sizeof(*game),file) == sizeof(*game))
			&&      (!strcmp(game->signature,GamespecificExtension))
//			&&      (game->oldtest == &PrintX)
			)
				ok = true;

			fclose(file);
		}

		if (ok)
			game->present = true;
		else
		{
			strcpy(game->signature,GamespecificExtension);
			game->present = false;
			strcpy(game->name,"Empty");
		}
	}
}

///////////////////////////////////////////////////////////////////////////
//
//      US_Startup() - Starts the User Mgr
//
///////////////////////////////////////////////////////////////////////////
void
US_Startup(void)
{
	int     i;

	if (US_Started)
		return;

	US_InitRndT(true);              // Initialize the random number generator

	USL_ReadConfig();               // Read config file

	US_Started = true;
}

///////////////////////////////////////////////////////////////////////////
//
//      US_Setup() - Does the disk access part of the User Mgr's startup
//
///////////////////////////////////////////////////////////////////////////
void
US_Setup(void)
{
	USL_CheckSavedGames();  // Check which saved games are present
}

///////////////////////////////////////////////////////////////////////////
//
//      US_Shutdown() - Shuts down the User Mgr
//
///////////////////////////////////////////////////////////////////////////
void
US_Shutdown(void)
{
	if (!US_Started)
		return;

	if (!abortprogram)
		USL_WriteConfig();

	US_Started = false;
}

///////////////////////////////////////////////////////////////////////////
//
//      US_CheckParm() - checks to see if a string matches one of a set of
//              strings. The check is case insensitive. The routine returns the
//              index of the string that matched, or -1 if no matches were found
//
///////////////////////////////////////////////////////////////////////////
int
US_CheckParm(char *parm,char **strings)
{
	char    cp,cs,
			*p,*s;
	int             i;

	while (!isalpha(*parm)) // Skip non-alphas
		parm++;

	for (i = 0;*strings && **strings;i++)
	{
		for (s = *strings++,p = parm,cs = cp = 0;cs == cp;)
		{
			cs = *s++;
			if (!cs)
				return(i);
			cp = *p++;

			if (isupper(cs))
				cs = tolower(cs);
			if (isupper(cp))
				cp = tolower(cp);
		}
	}
	return(-1);
}

//      Window/Printing routines

///////////////////////////////////////////////////////////////////////////
//
//      US_SetPrintRoutines() - Sets the routines used to measure and print
//              from within the User Mgr. Primarily provided to allow switching
//              between masked and non-masked fonts
//
///////////////////////////////////////////////////////////////////////////
void
US_SetPrintRoutines(void (*measure)(char*,word *,word *),void (*print)(char*))
{
	USL_MeasureString = measure;
	USL_DrawString = print;
}

///////////////////////////////////////////////////////////////////////////
//
//      US_Print() - Prints a string in the current window. Newlines are
//              supported.
//
///////////////////////////////////////////////////////////////////////////
void
US_Print(char *s)
{
	char    c,*se;
	word    w,h;

	while (*s)
	{
		se = s;
		while ((c = *se) && (c != '\n'))
			se++;
		*se = '\0';

		USL_MeasureString(s,&w,&h);
		px = PrintX;
		py = PrintY;
		USL_DrawString(s);

		s = se;
		if (c)
		{
			*se = c;
			s++;

			PrintX = WindowX;
			PrintY += h;
		}
		else
			PrintX += w;
	}
}

///////////////////////////////////////////////////////////////////////////
//
//      US_PrintUnsigned() - Prints an unsigned long
//
///////////////////////////////////////////////////////////////////////////
void
US_PrintUnsigned(longword n)
{
	char    buffer[32];

	sprintf(buffer, "%u", n);
	US_Print(buffer);
}

///////////////////////////////////////////////////////////////////////////
//
//      US_PrintSigned() - Prints a signed long
//
///////////////////////////////////////////////////////////////////////////
void
US_PrintSigned(long n)
{
	char    buffer[32];

	sprintf(buffer, "%i", n);
	US_Print(buffer);
}

///////////////////////////////////////////////////////////////////////////
//
//      USL_PrintInCenter() - Prints a string in the center of the given rect
//
///////////////////////////////////////////////////////////////////////////
void
USL_PrintInCenter(char *s,Rect r)
{
	word    w,h,
			rw,rh;

	USL_MeasureString(s,&w,&h);
	rw = r.lr.x - r.ul.x;
	rh = r.lr.y - r.ul.y;

	px = r.ul.x + ((rw - w) / 2);
	py = r.ul.y + ((rh - h) / 2);
	USL_DrawString(s);
}

///////////////////////////////////////////////////////////////////////////
//
//      US_PrintCentered() - Prints a string centered in the current window.
//
///////////////////////////////////////////////////////////////////////////
void
US_PrintCentered(char *s)
{
	Rect    r;

	r.ul.x = WindowX;
	r.ul.y = WindowY;
	r.lr.x = r.ul.x + WindowW;
	r.lr.y = r.ul.y + WindowH;

	USL_PrintInCenter(s,r);
}

///////////////////////////////////////////////////////////////////////////
//
//      US_CPrintLine() - Prints a string centered on the current line and
//              advances to the next line. Newlines are not supported.
//
///////////////////////////////////////////////////////////////////////////
void
US_CPrintLine(char *s)
{
	word    w,h;

	USL_MeasureString(s,&w,&h);

	if (w > WindowW)
		Quit("US_CPrintLine() - String exceeds width");
	px = WindowX + ((WindowW - w) / 2);
	py = PrintY;
	USL_DrawString(s);
	PrintY += h;
}

///////////////////////////////////////////////////////////////////////////
//
//      US_CPrint() - Prints a string in the current window. Newlines are
//              supported.
//
///////////////////////////////////////////////////////////////////////////
void
US_CPrint(char *S)
{
	char str[strlen(S)+1];
	strcpy(str, S);
	char *s = str;
	char    c,*se;

	while (*s)
	{
		se = s;
		while ((c = *se) && (c != '\n'))
			se++;
		*se = '\0';

		US_CPrintLine(s);

		s = se;
		if (c)
		{
			*se = c;
			s++;
		}
	}
}


///////////////////////////////////////////////////////////////////////////
//
// US_Printxy()
//
///////////////////////////////////////////////////////////////////////////

void US_Printxy(word x, word y, char *text)
{
	word orgx, orgy;

	orgx = PrintX;
	orgy = PrintY;

//	PrintX = WindowX+x;
//	PrintY = WindowY+y;
	PrintX = x;
	PrintY = y;
	US_Print(text);

	PrintX = orgx;
	PrintY = orgy;
}


///////////////////////////////////////////////////////////////////////////
//
//      US_ClearWindow() - Clears the current window to white and homes the
//              cursor
//
///////////////////////////////////////////////////////////////////////////
void
US_ClearWindow(void)
{
	VW_Bar(WindowX,WindowY,WindowW,WindowH,WHITE);
	PrintX = WindowX;
	PrintY = WindowY;
}

///////////////////////////////////////////////////////////////////////////
//
//      US_DrawWindow() - Draws a frame and sets the current window parms
//
///////////////////////////////////////////////////////////////////////////
void
US_DrawWindow(word x,word y,word w,word h)
{
	word    i,
			sx,sy,sw,sh;

	WindowX = x * 8;
	WindowY = y * 8;
	WindowW = w * 8;
	WindowH = h * 8;

	PrintX = WindowX;
	PrintY = WindowY;

	sx = (x - 1) * 8;
	sy = (y - 1) * 8;
	sw = (w + 1) * 8;
	sh = (h + 1) * 8;

	US_ClearWindow();

	VW_DrawTile8M(sx/8,sy,0);
	VW_DrawTile8M(sx/8,sy + sh,6);
	for (i = sx + 8;i <= sx + sw - 8;i += 8) {
		VW_DrawTile8M(i/8,sy,1);
		VW_DrawTile8M(i/8,sy + sh,7);
	}
	VW_DrawTile8M(i/8,sy,2);
	VW_DrawTile8M(i/8,sy + sh,8);

	for (i = sy + 8;i <= sy + sh - 8;i += 8) {
		VW_DrawTile8M(sx/8,i,3);
		VW_DrawTile8M((sx + sw)/8,i,5);
	}
}

///////////////////////////////////////////////////////////////////////////
//
//      US_CenterWindow() - Generates a window of a given width & height in the
//              middle of the screen
//
///////////////////////////////////////////////////////////////////////////
void
US_CenterWindow(word w,word h)
{
	US_DrawWindow(((MaxX / 8) - w) / 2,((MaxY / 8) - h) / 2,w,h);
}


///////////////////////////////////////////////////////////////////////////
//
//      US_SaveWindow() - Saves the current window parms into a record for
//              later restoration
//
///////////////////////////////////////////////////////////////////////////
void
US_SaveWindow(WindowRec *win)
{
	win->x = WindowX;
	win->y = WindowY;
	win->w = WindowW;
	win->h = WindowH;

	win->px = PrintX;
	win->py = PrintY;
}

///////////////////////////////////////////////////////////////////////////
//
//      US_RestoreWindow() - Sets the current window parms to those held in the
//              record
//
///////////////////////////////////////////////////////////////////////////
void
US_RestoreWindow(WindowRec *win)
{
	WindowX = win->x;
	WindowY = win->y;
	WindowW = win->w;
	WindowH = win->h;

	PrintX = win->px;
	PrintY = win->py;
}

//      Input routines

///////////////////////////////////////////////////////////////////////////
//
//      USL_XORICursor() - XORs the I-bar text cursor. Used by US_LineInput()
//
///////////////////////////////////////////////////////////////////////////
static void
USL_XORICursor(int x,int y,char *s,word cursor, int color)
{
	char    buf[MaxString];
	word    w,h;

	strcpy(buf,s);
	buf[cursor] = '\0';
	USL_MeasureString(buf,&w,&h);

	px = x + w - 1;
	py = y;
	int temp = fontcolor;
	fontcolor = color;
	USL_DrawString("\x80");
	fontcolor = temp;
}

///////////////////////////////////////////////////////////////////////////
//
//      US_LineInput() - Gets a line of user input at (x,y), the string defaults
//              to whatever is pointed at by def. Input is restricted to maxchars
//              chars or maxwidth pixels wide. If the user hits escape (and escok is
//              true), nothing is copied into buf, and false is returned. If the
//              user hits return, the current string is copied into buf, and true is
//              returned
//
///////////////////////////////////////////////////////////////////////////
boolean
US_LineInput(int x,int y,char *buf,char *def,boolean escok,
				int maxchars,int maxwidth,int bgcolor)
{
	boolean         redraw,
				cursorvis,cursormoved,
				done,result;
	ScanCode        sc;
	char            c,
				s[MaxString],olds[MaxString];
	word            i,
				cursor,
				w,h,
				len;
	longword        lasttime;

	if (def)
		strcpy(s,def);
	else
		*s = '\0';
	*olds = '\0';
	cursor = strlen(s);
	cursormoved = redraw = true;

	cursorvis = done = false;
	lasttime = SP_TimeCount();
	SPI_ClearKeysDown();

	while (!done)
	{
		if (cursorvis)
			USL_XORICursor(x,y,s,cursor,bgcolor);
//	asm     pushf
//	asm     cli

		sc = SPI_GetLastKey();
		c = SPI_GetLastASCII();
		SPI_ClearKeysDown();

//	asm     popf

		switch (sc)
		{
		case sc_LeftArrow:
			if (cursor)
				cursor--;
			c = key_None;
			cursormoved = true;
			break;
		case sc_RightArrow:
			if (s[cursor])
				cursor++;
			c = key_None;
			cursormoved = true;
			break;
		case sc_Home:
			cursor = 0;
			c = key_None;
			cursormoved = true;
			break;
		case sc_End:
			cursor = strlen(s);
			c = key_None;
			cursormoved = true;
			break;

		case sc_Return:
			strcpy(buf,s);
			done = true;
			result = true;
			c = key_None;
			break;
		case sc_Escape:
			if (escok)
			{
				done = true;
				result = false;
			}
			c = key_None;
			break;

		case sc_BackSpace:
			if (cursor)
			{
				strcpy(s + cursor - 1,s + cursor);
				cursor--;
				redraw = true;
			}
			c = key_None;
			cursormoved = true;
			break;
		case sc_Delete:
			if (s[cursor])
			{
				strcpy(s + cursor,s + cursor + 1);
				redraw = true;
			}
			c = key_None;
			cursormoved = true;
			break;

		case 0x4c:      // Keypad 5
		case sc_UpArrow:
		case sc_DownArrow:
		case sc_PgUp:
		case sc_PgDn:
		case sc_Insert:
			c = key_None;
			break;
		}

		if (c)
		{
			len = strlen(s);
			USL_MeasureString(s,&w,&h);

			if
			(
				isprint(c)
			&&      (len < MaxString - 1)
			&&      ((!maxchars) || (len < maxchars))
			&&      ((!maxwidth) || (w < maxwidth))
			)
			{
				for (i = len + 1;i > cursor;i--)
					s[i] = s[i - 1];
				s[cursor++] = c;
				redraw = true;
			}
		}

		if (redraw)
		{
			px = x;
			py = y;
			int temp=fontcolor;
			fontcolor = bgcolor;
			USL_DrawString(olds);
			fontcolor = temp;
			strcpy(olds,s);

			px = x;
			py = y;
			USL_DrawString(s);

			redraw = false;
		}

		if (cursormoved)
		{
			cursorvis = false;
			lasttime = SP_TimeCount() - TickBase;

			cursormoved = false;
		}
		if (SP_TimeCount() - lasttime > TickBase / 2)
		{
			lasttime = SP_TimeCount();

			cursorvis ^= true;
		}
		if (cursorvis)
			USL_XORICursor(x,y,s,cursor,fontcolor);

		SPG_FlipBuffer();
	}

	if (cursorvis)
		USL_XORICursor(x,y,s,cursor,bgcolor);
	if (!result)
	{
		px = x;
		py = y;
		USL_DrawString(olds);
	}
	SPG_FlipBuffer();

	SPI_ClearKeysDown();
	return(result);
}
