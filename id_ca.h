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

// ID_CA.H

#ifndef __TYPES__
#include "id_types.h"
#endif

#ifndef __ID_GLOB__
#include "id_glob.h"
#endif

#define __ID_CA__

//===========================================================================

#define NUMMAPS		30
#define MAPPLANES	3

//===========================================================================

extern	void				*grsegs[NUMCHUNKS];

extern	char		*titleptr[8];

//===========================================================================

boolean CA_FarRead (FILE *handle, byte *dest, long length);
boolean CA_FarWrite (FILE *handle, byte *source, long length);

long CA_RLEWCompress (uint16_t *source, long length, uint16_t *dest,
  uint16_t rlewtag);

void CA_RLEWexpand (uint16_t *source, uint16_t *dest,long length,
  uint16_t rlewtag);

void CA_Startup (void);
void CA_Shutdown (void);

void CA_CacheAudioChunk (int chunk);
void CA_LoadAllSounds (void);

void CA_CacheGrChunk (int chunk);
void CA_CacheMap (int mapnum);

