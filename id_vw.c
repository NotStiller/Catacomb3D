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

// ID_VW.C

#include "id_heads.h"


unsigned	fontnumber;		// 0 based font number for drawing

void
VWL_MeasureString (char *string, word *width, word *height, fontstruct *font)
{
	byte *f = (byte*)font;

	*height = font->height;
	for (*width = 0;*string;string++)
		*width += font->width[*(byte*)string];

}

void	VW_MeasurePropString (char *string, word *width, word *height)
{
	VWL_MeasureString(string,width,height,(fontstruct *)grsegs[STARTFONT+fontnumber]);
}

void VWB_DrawPropString(char *string)
{
	int x,y;
	x = px;
	y = py;
	VW_DrawPropString (string);
}


//===========================================================================
