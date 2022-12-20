#ifndef BSPFILE_H
#define BSPFILE_H

#include <stdint.h>

enum LumpType
{
	LUMP_ENTITIES = 0,
	LUMP_MATERIALS,
	LUMP_PLANES,
	LUMP_NODES,
	LUMP_LEAVES,
	LUMP_LEAFFACES,
	LUMP_LEAFBRUSHES,
	LUMP_MODELS,
	LUMP_BRUSHES,
	LUMP_BRUSHSIDES,
	LUMP_VERTS,
	LUMP_INDICES,
	LUMP_FOGS,
	LUMP_FACES,
	LUMP_LIGHTMAPS,
	LUMP_LIGHTGRIDS,
	LUMP_VISDATA,
	NUM_LUMPS
};

enum FaceType
{
	FACE_POLYGON = 1,
	FACE_PATCH,
	FACE_MESH,
	FACE_BILLBOARD
};

typedef struct
{
	uint32_t offset; // from beginning of file
	uint32_t length; // in bytes
} BSPLump;

typedef struct
{
	uint8_t magic[4];
	uint32_t version;
	BSPLump lumps[NUM_LUMPS];
} BSPHeader;

typedef struct
{
	uint8_t name[64];
	uint32_t flags;
	uint32_t contents;
} BSPMaterial;

typedef struct
{
	float position[3];
	float uv[2][2];
	float normal[3];
	uint8_t color[4];
} BSPVert;

typedef struct
{
	uint32_t vert;
} BSPIndex;

typedef struct
{
	uint32_t texture;
	uint32_t fog;
	uint32_t type;
	uint32_t firstVert;
	uint32_t numVerts;
	uint32_t firstIndex;
	uint32_t numIndices;
	uint32_t lightmapIndex;
	uint32_t lightmapStart[2];
	uint32_t lightmapSize[2];
	float lightmapOrigin[3];
	float lightmapUV[2][3];
	float normal[3];
	uint32_t patchSize[2];
} BSPFace;

typedef struct
{
	uint8_t *raw;
	BSPHeader *header;
	char *entities;
	BSPMaterial *materials;
	void *planes;
	void *nodes;
	void *leaves;
	void *leaffaces;
	void *leafbrushes;
	void *models;
	void *brushes;
	void *brushsides;
	BSPVert *verts;
	BSPIndex *indices;
	void *fogs;
	BSPFace *faces;
	void *lightmaps;
	void *lightgrids;
	void *visdata;
	unsigned int *realIndices;
	unsigned int realIndexCount;
} BSPLevel;

#endif
