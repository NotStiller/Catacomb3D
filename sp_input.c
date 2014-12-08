
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

#include "id_heads.h"
#include "sp_input.h"

// some states for input handling
static int mouseDX=0, mouseDY=0;
int	keyboard[128];
char lastASCII;
int lastScan;

// 	Global variables
		KeyboardDef	KbdDefs = {0x1d,0x38,0x48,0x4b,0x4d,0x50};

//	Internal variables
static	boolean		IN_Started;
static	boolean		CapsLock;
static	byte        *ScanNames[] =		// Scan code names with single chars
					{
	"?","?","1","2","3","4","5","6","7","8","9","0","-","+","?","?",
	"Q","W","E","R","T","Y","U","I","O","P","[","]","|","?","A","S",
	"D","F","G","H","J","K","L",";","\"","?","?","?","Z","X","C","V",
	"B","N","M",",",".","/","?","?","?","?","?","?","?","?","?","?",
	"?","?","?","?","?","?","?","?","\xf","?","-","\x15","5","\x11","+","?",
	"\x13","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?",
	"?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?",
	"?","?","?","?","?","?","?","?","?","?","?","?","?","?","?","?"
					},	// DEBUG - consolidate these
					ExtScanCodes[] =	// Scan codes with >1 char names
					{
	1,0xe,0xf,0x1d,0x2a,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,
	0x3f,0x40,0x41,0x42,0x43,0x44,0x57,0x59,0x46,0x1c,0x36,
	0x37,0x38,0x47,0x49,0x4f,0x51,0x52,0x53,0x45,0x48,
	0x50,0x4b,0x4d,but_Mouse1,but_Mouse2,but_Mouse3,0x00
					},
					*ExtScanNames[] =	// Names corresponding to ExtScanCodes
					{
	"Esc","BkSp","Tab","Ctrl","LShft","Space","CapsLk","F1","F2","F3","F4",
	"F5","F6","F7","F8","F9","F10","F11","F12","ScrlLk","Enter","RShft",
	"PrtSc","Alt","Home","PgUp","End","PgDn","Ins","Del","NumLk","Up",
	"Down","Left","Right","LMB","RMB","MMB",""
					};



void SPI_ClearKeysDown(void) {
	int	i;

	lastScan = sc_None;
	lastASCII = key_None;
	for (i = 0;i < 128;i++)
		keyboard[i] = 0;
}

int SPI_GetKeyDown(int Key) {
	assert(Key >= 0 && Key < 128);
	SP_PollEvents();
	return keyboard[Key];
}

char SPI_GetLastASCII() {
	SP_PollEvents();
	return lastASCII;
}

int SPI_GetLastKey() {
	SP_PollEvents();
	return lastScan;
}

void SPI_GetMouseDelta(int *X, int *Y) {
	SP_PollEvents();
	if (X != NULL) {
		*X = mouseDX;
	}
	if (Y != NULL) {
		*Y = mouseDY;
	}
	mouseDX = 0;
	mouseDY = 0;
}

void SPI_GetPlayerControl(ControlInfo *info) {
	word		buttons;
	int			dx,dy;
	Motion		mx,my;
	KeyboardDef	*def;

	def = &KbdDefs;

	if (SPI_GetKeyDown(def->up))
		my = motion_Up;
	else if (SPI_GetKeyDown(def->down))
		my = motion_Down;
	else
		my = motion_None;
	
	if (SPI_GetKeyDown(def->left))
		mx = motion_Left;
	else if (SPI_GetKeyDown(def->right))
		mx = motion_Right;
	else
		mx = motion_None;

	info->fire = SPI_GetKeyDown(def->button0) || SPI_GetKeyDown(but_Mouse1);
	info->strafe = SPI_GetKeyDown(def->button1);
	info->run = SPI_GetKeyDown(sc_RShift)||SPI_GetKeyDown(sc_LShift);
	info->potion = SPI_GetKeyDown(sc_Space) || SPI_GetKeyDown(sc_C) || SPI_GetKeyDown(sc_H);
	info->bolt = SPI_GetKeyDown(sc_Z) || SPI_GetKeyDown(sc_Y) || SPI_GetKeyDown(sc_B);
	info->nuke = SPI_GetKeyDown(sc_Enter) || SPI_GetKeyDown(sc_X);

	SPI_GetMouseDelta(&dx,&dy);

	info->xaxis = mx;
	info->yaxis = my;
	info->x = dx;
	info->y = dy;
	if (mx < -1 || mx > 1 || my < -1 || my > 1) {
		printf("bad dir mx=%i my=%i\n", mx, my);
		mx = 0;
		my = 0;
	}
}

char *SPI_GetScanName(ScanCode scan) {
	byte		**p;
	ScanCode	*s;

	for (s = ExtScanCodes,p = ExtScanNames;*s;p++,s++)
		if (*s == scan)
			return(*p);

	return(ScanNames[scan]);
}

void SPI_InputASCII(char ASCIICode) {
	lastASCII = ASCIICode;
}

void SPI_InputKey(int Key, int Down) {
	keyboard[Key] = Down;
	if (Down) {
		lastScan = Key;
	}
}

void SPI_InputMouseMotion(int DX, int DY) {
	mouseDX += DX;
	mouseDY += DY;
}

void SPI_WaitForever(void)
{
	keyboard[lastScan] = 0;
	lastScan = sc_None;

	while (SPI_GetLastKey() == sc_None) { }

	keyboard[lastScan] = 0;
	lastScan = sc_None;
}

///////////////////////////////////////////////////////////////////////////
//
//	IN_UserInput() - Waits for the specified delay time (in ticks) or the
//		user pressing a key or a mouse button. 
//
///////////////////////////////////////////////////////////////////////////
boolean SPI_WaitFor(long Delay) {
	long lasttime;
	lasttime = SP_TimeCount();
	do {
		if (SPI_GetLastKey() != sc_None) {
			return true;
		}
	} while (SP_TimeCount() - lasttime < Delay);

	return false;
}



