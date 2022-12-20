#include <stdio.h>
#include <stdlib.h>

#include "bsp.h"

BSPLevel *loadMap(Stack *s, const char *const filename)
{
	BSPLevel *level = stalloc(s, sizeof(BSPLevel));
	if (!level)
		return NULL;

	FILE *filePtr;

	if (!(filePtr = fopen(filename, "rb")))
		return NULL;

	fseek(filePtr, 0, SEEK_END);
	size_t fileSize = ftell(filePtr);
	rewind(filePtr);

	level->raw = stalloc(s, fileSize);

	if (!level->raw)
	{
		fclose(filePtr);
		return NULL;
	}

	fread(level->raw, sizeof(uint8_t), fileSize, filePtr);
	fclose(filePtr);

	level->header = (BSPHeader *)(level->raw);

	BSPLump *lumps = level->header->lumps;

	level->entities = (char *)(level->raw + lumps[LUMP_ENTITIES].offset);
	level->materials = (BSPMaterial *)(level->raw + lumps[LUMP_MATERIALS].offset);
	level->verts = (BSPVert *)(level->raw + lumps[LUMP_VERTS].offset);
	level->indices = (BSPIndex *)(level->raw + lumps[LUMP_INDICES].offset);
	level->faces = (BSPFace *)(level->raw + lumps[LUMP_FACES].offset);

	printf("num verts: %lu\n", lumps[LUMP_VERTS].length / sizeof(BSPVert));
	printf("num indices: %lu\n", lumps[LUMP_INDICES].length / sizeof(BSPIndex));

	convertMapVerts(level);
	getRealMapIndices(s, level);

	printf("num tex: %lu\n", lumps[LUMP_MATERIALS].length / sizeof(BSPMaterial));

	for (int i = 0; i < lumps[LUMP_MATERIALS].length / sizeof(BSPMaterial); i++)
	{
		level->materials[i].name[63] = '\0';
	}

	return level;
}

void convertMapVerts(BSPLevel *level)
{
	unsigned int vertCount = level->header->lumps[LUMP_VERTS].length / sizeof(BSPVert);
	for (unsigned int v = 0; v < vertCount; v++)
	{
		float x, y, z;
		x = level->verts[v].position[0];
		y = level->verts[v].position[2];
		z = -1.0f * level->verts[v].position[1];
		level->verts[v].position[0] = x;
		level->verts[v].position[1] = y;
		level->verts[v].position[2] = z;

		x = level->verts[v].normal[0];
		y = level->verts[v].normal[2];
		z = -1.0f * level->verts[v].normal[1];
		level->verts[v].normal[0] = x;
		level->verts[v].normal[1] = y;
		level->verts[v].normal[2] = z;
	}
}

void getRealMapIndices(Stack *s, BSPLevel *level)
{
	unsigned int faceCount = level->header->lumps[LUMP_FACES].length / sizeof(BSPFace);
	unsigned int indexCount = 0;
	for (unsigned int f = 0; f < faceCount; f++)
	{
		if (level->faces[f].type != FACE_POLYGON && level->faces[f].type != FACE_MESH)
			continue;
		indexCount += level->faces[f].numIndices;
	}

	level->realIndexCount = indexCount;
	level->realIndices = stalloc(s, sizeof(unsigned int) * indexCount);
	if (!level->realIndices)
	{
		return destroyMap(level);
	}

	unsigned int currentIndex = 0;
	for (unsigned int f = 0; f < faceCount; f++)
	{
		if (level->faces[f].type != FACE_POLYGON && level->faces[f].type != FACE_MESH)
			continue;

		unsigned int firstFaceVert = level->faces[f].firstVert;
		unsigned int firstFaceIndex = level->faces[f].firstIndex;
		unsigned int faceIndexCount = level->faces[f].numIndices;
		for (unsigned int i = 0; i < faceIndexCount; i++)
		{
			level->realIndices[currentIndex] =
				level->indices[firstFaceIndex + i].vert + firstFaceVert;
			currentIndex += 1;
		}
	}
}

BSPLevel *destroyMap(BSPLevel *level)
{
	free(level->raw);
	free(level->realIndices);
	free(level);
	return NULL;
}
