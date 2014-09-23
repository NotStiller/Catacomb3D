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

// ID_CA.C

/*
=============================================================================

Id Software Caching Manager
---------------------------

Must be started BEFORE the memory manager, because it needs to get the headers
loaded into the data segment

=============================================================================
*/

#include "id_heads.h"
#include "id_strs.h"

/*
=============================================================================

						 GLOBAL VARIABLES

=============================================================================
*/
void				*grsegs[NUMCHUNKS];

/*
=============================================================================

						 LOCAL VARIABLES

=============================================================================
*/



/*
=============================================================================

					   LOW LEVEL ROUTINES

=============================================================================
*/


/*
==========================
=
= CA_FarRead
=
= Read from a file to a far pointer
=
==========================
*/

boolean CA_FarRead (FILE *handle, byte *dest, long length)
{
	unsigned long read = fread(dest, 1, length, handle);
	if (read != length)
		Quit("CA_FarRead failed!");
	return true;
}

/*
==========================
=
= CA_FarWrite
=
= Write from a file to a far pointer
=
==========================
*/

boolean CA_FarWrite (FILE *handle, byte *source, long length)
{
	if (fwrite(source, 1, length, handle) != length)
		Quit("CA_FarWrite failed!");
	return true;
}


/*
============================================================================

		COMPRESSION routines, see JHUFF.C for more

============================================================================
*/

/*
======================
=
= CAL_HuffExpand
=
= Length is the length of the EXPANDED data
=
======================
*/

void CAL_HuffExpand (byte *source, byte *dest,long length,huffnode *hufftable)
{
  huffnode *headptr;
  headptr = hufftable+254;	// head node is allways node 254

	huffnode *curhuffnode = headptr;
	int i, j;
	int written = 0;
	i = 0;
	while (written < length) {
		byte srcbyte = source[i++];
		for (j = 0; j < 8; j++) {
			unsigned short b = curhuffnode->bit0;
			if (srcbyte&1) {
				b = curhuffnode->bit1;
			}
			srcbyte = srcbyte>>1;
			if (b < 256) {
				dest[written++] = b;
				curhuffnode = headptr;
				if (written == length) {
					break;
				}
			} else {
			assert(b-256 >= 0);
				curhuffnode = &hufftable[b-256];
			}
		}
	}
}


/*
======================
=
= CAL_CarmackExpand
=
= Length is the length of the EXPANDED data
=
======================
*/

#define NEARTAG	0xa7
#define FARTAG	0xa8

void CAL_CarmackExpand (uint16_t *source, uint16_t *dest, long length)
{
	uint16_t	ch,chhigh,count,offset;
	uint16_t	*copyptr, *inptr, *outptr;

	length/=2;

	inptr = source;
	outptr = dest;

	while (length)
	{
		ch = *inptr++;
		chhigh = ch>>8;
		if (chhigh == NEARTAG)
		{
			count = ch&0xff;
			if (!count)
			{				// have to insert a word containing the tag byte
//				ch |= *((unsigned char *)inptr)++;
				ch |= *(uint8_t *)inptr;
				inptr = (uint16_t*) ((uint8_t*)inptr+1);
				*outptr++ = ch;
				length--;
			}
			else
			{
				offset = *(uint8_t*)inptr;
				inptr = (uint16_t*)((uint8_t*)inptr+1);
				copyptr = outptr - offset;
				length -= count;
				while (count--)
					*outptr++ = *copyptr++;
			}
		}
		else if (chhigh == FARTAG)
		{
			count = ch&0xff;
			if (!count)
			{				// have to insert a word containing the tag byte
//				ch |= *((unsigned char *)inptr)++;
				ch |= *(uint8_t*)inptr;
				inptr = (uint16_t*)((uint8_t*)inptr+1);
				*outptr++ = ch;
				length --;
			}
			else
			{
				offset = *inptr++;
				copyptr = dest + offset;
				length -= count;
				while (count--)
					*outptr++ = *copyptr++;
			}
		}
		else
		{
			*outptr++ = ch;
			length --;
		}
	}
}



/*
======================
=
= CA_RLEWcompress
=
======================
*/

long CA_RLEWCompress (uint16_t *source, long length, uint16_t *dest,
  uint16_t rlewtag)
{
  long complength;
  uint16_t value,count,i;
  uint16_t *start,*end;

  start = dest;

  end = source + (length+1)/2;

//
// compress it
//
  do
  {
	count = 1;
	value = *source++;
	while (*source == value && source<end)
	{
	  count++;
	  source++;
	}
	if (count>3 || value == rlewtag)
	{
    //
    // send a tag / count / value string
    //
      *dest++ = rlewtag;
      *dest++ = count;
      *dest++ = value;
    }
    else
    {
    //
    // send word without compressing
    //
      for (i=1;i<=count;i++)
	*dest++ = value;
	}

  } while (source<end);

  complength = 2*(dest-start);
  return complength;
}


/*
======================
=
= CA_RLEWexpand
= length is EXPANDED length
=
======================
*/

void CA_RLEWexpand (uint16_t *source, uint16_t *dest,long length,
  uint16_t rlewtag)
{
  uint16_t *end = dest + (length)/2;
  uint16_t value, count, i;
  do
  {
	value = *source++;
	if (value != rlewtag)
	//
	// uncompressed
	//
	  *dest++=value;
	else
	{
	//
	// compressed string
	//
	  count = *source++;
	  value = *source++;
	  for (i=1;i<=count;i++){
		*dest++ = value;
      }
	}
  } while (dest<end);

}

//===========================================================================


void CA_CacheGrChunk (int chunk)
{
	SPD_LoadGrChunk(chunk);
}



