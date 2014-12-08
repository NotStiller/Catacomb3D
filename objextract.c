#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Just a quick hack to extract the data from all the .OBJ files.

typedef struct {
	uint16_t bit0,bit1;	// 0-255 is a character, >255 is a pointer to a node
} huffnode;


uint8_t SPD_ReadU8(uint8_t **Buffer) {
	return *(*Buffer)++;
}

int8_t SPD_ReadS8(uint8_t **Buffer) {
	uint8_t v = *(*Buffer)++;
	return *(int8_t*)&v;
}

uint16_t SPD_ReadU16(uint8_t **Buffer) {
	uint16_t v = SPD_ReadU8(Buffer)+0x100*SPD_ReadU8(Buffer);
	return v;
}

int16_t SPD_ReadS16(uint8_t **Buffer) {
	uint16_t v = SPD_ReadU16(Buffer);
	return *(int16_t*)&v;
}

uint16_t SPD_ReadU8or16(uint8_t **Buffer) { // these are not needed for the Catacomb files, but for .OBJ files
	uint16_t v = SPD_ReadU8(Buffer);
	if (v&0x80) {
		int8_t b=SPD_ReadU8(Buffer);
		v = (v&0x7F)<<8;
		v += b;
	}
	return v;
}

uint32_t SPD_ReadU32(uint8_t **Buffer) {
	return SPD_ReadU16(Buffer)+0x10000*SPD_ReadU16(Buffer);
/*	uint16_t lower = SPD_ReadU16(Buffer), upper = SPD_ReadU16(Buffer);
	int16_t slower = *(int16_t*)&lower, supper = *(int16_t*)&upper;
	uint32_t v1 = lower+0x10000*upper;
	uint32_t v2 = slower+0x10000*supper;
	assert(v1 == v2);
	return v1;*/
}

int32_t SPD_ReadS32(uint8_t **Buffer) {
	uint32_t v = SPD_ReadU32(Buffer);
	return *(int32_t*)&v;
}

uint8_t *SPD_ParseObj(const char *Filename, char ObjName[256], long *ObjSize) {
	FILE *f = fopen(Filename, "rb");
	assert(f != NULL);
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);
	uint8_t contents[size];
	assert(fread(contents, 1, size, f) == size);
	fclose(f);

	uint8_t *p = contents;
	int nameSet = 0;
	uint8_t *obj = NULL;
	long objSize = 0;
	while (1) {
		uint8_t type = SPD_ReadU8(&p);
		uint16_t length = SPD_ReadU16(&p);
		uint8_t *nextp = p+length;
		if (type == 0x90) { // name
			SPD_ReadU8or16(&p);
			if (SPD_ReadU8or16(&p) == 0) {
				SPD_ReadU16(&p);
			}
			uint8_t len = SPD_ReadU8(&p);
			memcpy(ObjName, p, len);
			ObjName[len] = 0;
			assert(!nameSet);
			nameSet = 1;
		} else if (type == 0xA0) { // contents
			length -= 4;
			if (SPD_ReadU8or16(&p)>=0x80) {
				length--;
			}
			SPD_ReadU16(&p);
			obj = realloc(obj, objSize+length);
			assert(obj != NULL);
			memcpy(obj+objSize, p, length);
			objSize += length;
		} else if (0) {
			printf("Record of type 0x%02x and length %i\n", (int)type, length);
		}
		p = nextp;
		if (p-contents >= size) {
			assert(p-contents == size);
			break;
		}
	}
	*ObjSize = objSize;
	return obj;
}


void SPD_DumpHexChar(const uint8_t *Buffer, long Size) {
	long i;
	for (i = 0; i < Size; i += 8) {
		int j;
		for (j = 0; j < 8 && i+j < Size; j++) {
			printf("%02x, ", Buffer[i+j]);			
		}
		for (; j < 8; j++) {
			printf("    ");
		}
		printf("  //  ");
		for (j = 0; j < 8 && i+j < Size; j++) {
			uint8_t c = Buffer[i+j];
			printf("%c", isprint(c)?*(char*)&c:'.');
		}
		printf("\n");
	}
}

void SPD_DumpU8Hex(const uint8_t *Buffer, long Size) {
	long i;
	for (i = 0; i < Size; i++) {
		printf("0x%02x, ", Buffer[i]);
		if (!((i+1)&7)) {
			printf("\n");
		}
	}
}

void SPD_DumpDict(const char *Name, uint8_t *Buf, long Size) {
	assert(Size == 1024);
	printf("huffnode %s[256] = {\n", Name);
	int i;
	uint8_t *p = Buf;
	for (i = 0; i < 256; i++) {
		int bit0 = (int)SPD_ReadU16(&p);
		int bit1 = (int)SPD_ReadU16(&p);
		printf("{%3i, %3i}, ", bit0, bit1);
		if (!(3&(i+1))) {
			printf("\n");
		}
	}
	printf("};\n");	
}

void SPD_DumpLongArray(const char *Prefix, uint8_t *Buf, long Size, int LongSize) {
	assert(!(Size%LongSize));
	assert(LongSize == 3 || LongSize == 4);
	int num=Size/LongSize;
	int32_t pos[num], size[num];
	
	int i, j, last=-1;
	uint8_t *p = Buf;
	for (i = 0; i < num; i++) {
		int32_t v=0;
		if (LongSize == 3) {
			v = SPD_ReadU8(&p)+0x100*(SPD_ReadU8(&p)+0x100*SPD_ReadU8(&p));
			if (v==0xFFFFFF) {
				v = -1;
			}
		} else {
			v = SPD_ReadU8(&p)+0x100*(SPD_ReadU8(&p)+0x100*(SPD_ReadU8(&p)+0x100*SPD_ReadU8(&p)));
			if (v==0xFFFFFFFF) {
				v = -1;
			}
		}
		pos[i] = v;
		size[i] = 0;
		if (v >= 0) {
			if (last >= 0) {
				size[last] = pos[i]-pos[last];
//				assert(size[last] >= 0);
			}
			last = i;
		}
	}
	printf("int32_t %sChunksPos[%i] = {\n", Prefix, num);
	for (i = 0; i < num; i++) {
		printf("%10i, ", pos[i]);
		if (!(3&(i+1))) {
			printf("\n");
		}
	}
	printf("};\n");
	printf("int32_t %sChunksSize[%i] = {\n", Prefix, num);
	for (i = 0; i < num; i++) {
		printf("%10i, ", size[i]);
		if (!(3&(i+1))) {
			printf("\n");
		}
	}
	printf("};\n");	
}


void SPD_DumpChunks(const char *Prefix, uint8_t *Buf, long Size, int LongSize) {
	assert(!(Size%LongSize));
	assert(LongSize == 3 || LongSize == 4);
	int num=Size/LongSize;
	int32_t pos[num], size[num];
	
	int i, j, last=-1;
	uint8_t *p = Buf;
	for (i = 0; i < num; i++) {
		int32_t v=0;
		if (LongSize == 3) {
			v = SPD_ReadU8(&p)+0x100*(SPD_ReadU8(&p)+0x100*SPD_ReadU8(&p));
			if (v==0xFFFFFF) {
				v = -1;
			}
		} else {
			v = SPD_ReadU8(&p)+0x100*(SPD_ReadU8(&p)+0x100*(SPD_ReadU8(&p)+0x100*SPD_ReadU8(&p)));
			if (v==0xFFFFFFFF) {
				v = -1;
			}
		}
		pos[i] = v;
		size[i] = 0;
		if (v >= 0) {
			if (last >= 0) {
				size[last] = pos[i]-pos[last];
//				assert(size[last] >= 0);
			}
			last = i;
		}
	}
	printf("DataFile %sFile = {%sHuffman, RLETAG, 0, NULL};\n", Prefix, Prefix);
	printf("ChunkDesc %sChunks[%i] = {\n", Prefix, num);
	for (i = 0; i < num; i++) {
		printf("\t{%i, %i, %i, &%sFile, NULL},\n", i, pos[i], size[i], Prefix);
	}
	printf("};\n");
}


void SPD_DumpMapfiletype(const char *Prefix, uint8_t *Buf, long Size) {
	assert(Size >= 402);
	uint8_t *p = Buf;

	uint16_t rletag = SPD_ReadU16(&p);
	printf("DataFile %sFile = {NULL, 0x%4x, 0, NULL};\n", Prefix, rletag);
	printf("int32_t %sOffsets[100] = {\n", Prefix);
	int i;
	for (i = 0; i < 100; i++) {
		printf("%10i, ", SPD_ReadS32(&p));
		if (!(3&(i+1))) {
			printf("\n");
		}
	}
	printf("};\n");
}



void SPD_ExtractC3Data() {
	char name[256];
	long size;
	uint8_t *buf;

	buf = SPD_ParseObj("C3DADICT.OBJ", name, &size);
	SPD_DumpDict("C3AudioHuffman", buf, size);
	free(buf);
	buf = SPD_ParseObj("C3DEDICT.OBJ", name, &size);
	SPD_DumpDict("C3GraphHuffman", buf, size);
	free(buf);
	buf = SPD_ParseObj("C3DAHEAD.OBJ", name, &size);
	SPD_DumpChunks("C3Audio", buf, size, 4);
	free(buf);
	buf = SPD_ParseObj("C3DEHEAD.OBJ", name, &size);
	SPD_DumpChunks("C3Graph", buf, size, 3);
	free(buf);
	buf = SPD_ParseObj("C3DMHEAD.OBJ", name, &size);
	SPD_DumpMapfiletype("C3MapHead", buf, size);
	free(buf);
}


int StringEndsOn(const char *Str, const char *End) {
	int offs = strlen(Str) - strlen(End);
	if (offs < 0) {
		return 0;
	}

	int i;
	for (i = 0; i < strlen(End); i++) {
		if (tolower(Str[offs+i]) != tolower(End[i])) {
			return 0;
		}
	}
	return 1;
}

int main(int argc, char **argv) {
	char name[256];
	long size;
	uint8_t *buf;

	if (argc < 2) {
		printf("usage: objextract *.OBJ > output\n");
	}
	int i;
	for (i = 1; i < argc; i++) {
		char *arg = argv[i];

		if (StringEndsOn(arg, "C3DADICT.OBJ")) {
			buf = SPD_ParseObj(arg, name, &size);
			SPD_DumpDict("C3AudioHuffman", buf, size);
			free(buf);
		} else if (StringEndsOn(arg, "C3DEDICT.OBJ")) {
			buf = SPD_ParseObj(arg, name, &size);
			SPD_DumpDict("C3GraphHuffman", buf, size);
			free(buf);
		} else if (StringEndsOn(arg, "C3DAHEAD.OBJ")) {
			buf = SPD_ParseObj(arg, name, &size);
			SPD_DumpChunks("C3Audio", buf, size, 4);
			free(buf);
		} else if (StringEndsOn(arg, "C3DEHEAD.OBJ")) {
			buf = SPD_ParseObj(arg, name, &size);
			SPD_DumpChunks("C3Graph", buf, size, 3);
			free(buf);
		} else if (StringEndsOn(arg, "C3DMHEAD.OBJ")) {
			buf = SPD_ParseObj(arg, name, &size);
			SPD_DumpMapfiletype("C3MapHead", buf, size);
			free(buf);
		} else if (StringEndsOn(arg, "ABSADICT.OBJ")) {
			buf = SPD_ParseObj(arg, name, &size);
			SPD_DumpDict("C4AudioHuffman", buf, size);
			free(buf);
		} else if (StringEndsOn(arg, "ABSEDICT.OBJ")) {
			buf = SPD_ParseObj(arg, name, &size);
			SPD_DumpDict("C4GraphHuffman", buf, size);
			free(buf);
		} else if (StringEndsOn(arg, "ABSAHEAD.OBJ")) {
			buf = SPD_ParseObj(arg, name, &size);
			SPD_DumpChunks("C4Audio", buf, size, 4);
			free(buf);
		} else if (StringEndsOn(arg, "ABSEHEAD.OBJ")) {
			buf = SPD_ParseObj(arg, name, &size);
			SPD_DumpChunks("C4Graph", buf, size, 3);
			free(buf);
		} else if (StringEndsOn(arg, "ABSMHEAD.OBJ")) {
			buf = SPD_ParseObj(arg, name, &size);
			SPD_DumpMapfiletype("C4MapHead", buf, size);
			assert(size > 402+216);
			printf("uint8_t C4_tile_flags[%i] = {\n", size-402+216);
			SPD_DumpU8Hex(buf+402+216, size-402-216);
			printf("};\n");	
			free(buf);
		} 
	}
}

