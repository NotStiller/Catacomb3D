/* Catacomb 3-D SDL Port
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

#ifndef SP_DATA_H
#define SP_DATA_H

void SPD_SetupScaleWall(int Chunk, int PicNum, int ScaleWallNum);
void SPD_SetupScalePic(int Chunk, int PicNum, int ScalePicNum);
void SPD_LoadGrChunk(int Chunk);
void SPD_ReadFromFile(FILE *Handle, uint8_t *Dest, long Length);
void SPD_WriteToFile(FILE *Handle, uint8_t *Dest, long Length);
uint8_t *SPD_ReadFile(const char *Name, long *Size);
uint8_t *SPD_ReadFile2(FILE *File, long *Size);


uint8_t SPD_ReadU8(uint8_t **Buffer);
int8_t SPD_ReadS8(uint8_t **Buffer);
uint16_t SPD_ReadU16(uint8_t **Buffer);
int16_t SPD_ReadS16(uint8_t **Buffer);
uint16_t SPD_ReadU8or16(uint8_t **Buffer);
uint32_t SPD_ReadU32(uint8_t **Buffer);
int32_t SPD_ReadS32(uint8_t **Buffer);

void SPD_WriteU8(uint8_t **Buffer, uint8_t Value);
void SPD_WriteS8(uint8_t **Buffer, int8_t Value);
void SPD_WriteU16(uint8_t **Buffer, uint16_t Value);
void SPD_WriteS16(uint8_t **Buffer, int16_t Value);
void SPD_WriteU32(uint8_t **Buffer, uint32_t Value);
void SPD_WriteS32(uint8_t **Buffer, int32_t Value);


void SPD_SetupCatacomb3DData();
void SPD_SetupCatacombAbyssData();

#endif

