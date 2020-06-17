#ifndef WORLD_H
#define WORLD_H

#include <stdbool.h>
#include "graphics.h"

typedef enum {
    BLOCK_AIR,
    BLOCK_DIRT,
    BLOCK_GRASS,
    NUM_BLOCKS
} block_tag;
// there are ways to pack this
// could also use a bunch of #defines

typedef struct {
    block_tag tag;
} block;

#define CHUNK_RADIX 16

/*
The plan for chunks
-------------------

Eventually they are going to be all async and crazy

for now lets just actually define a chunk, generate a shit mesh for it

flags will be  like
    mesh ready
    empty
    visible
    paging status etc.

*/

typedef struct {
    int x;
    int y;
    int z;
    unsigned int vao;
    int num_triangles;
    block blocks[CHUNK_RADIX][CHUNK_RADIX][CHUNK_RADIX];
} chunk;

void generate_chunk(chunk *c, int x, int y, int z);
void mesh_chunk(chunk *c);
void draw_chunk(chunk *ch, context *c);

#endif