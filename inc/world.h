#ifndef WORLD_H
#define WORLD_H

#include "graphics.h"

#include <stdbool.h>
#include <cglm/struct.h>

typedef struct {
    long int x;
    long int y;
    long int z;
} vec3l;

typedef struct {
    int x;
    int y;
    int z;
} vec3i;

typedef struct {
    bool success;
    vec3l coords;
    int normal_x;
    int normal_y;
    int normal_z;
} pick_info;

typedef enum {
    BLOCK_AIR,
    BLOCK_GRASS,
    BLOCK_DIRT,
    // BLOCK_UNKNOWN, todo use this, and look up block opacity
    NUM_BLOCKS,
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
    unsigned int vbo;
    int num_triangles;
    block blocks[CHUNK_RADIX][CHUNK_RADIX][CHUNK_RADIX];
} chunk;

/*
for the first iteration of this guy, lets just have a fixed size array and not worry
about writing to disk or anything

still what is final usage going to look like?
hint heres my position, velocity and facing, better load and mesh these (and consequently others can be unloaded)
pls draw all the chunks
pls update all the chunks

so it needs an allocator for chunks
could use malloc atm, maybe arena allocator later
*/
#define MAX_CHUNKS_S 10
#define MAX_CHUNKS_SS MAX_CHUNKS_S*MAX_CHUNKS_S
#define MAX_CHUNKS_SSS MAX_CHUNKS_S*MAX_CHUNKS_SS
typedef struct {
    chunk *chunk_pointers[MAX_CHUNKS_SSS];
} chunk_manager;


void generate_chunk(chunk *c, int x, int y, int z);
void mesh_chunk(chunk *c);
void draw_chunk(chunk *ch, context *c);


// basically gets it to update and load chunks
void chunk_manager_position_hint(chunk_manager *cm, vec3s pos);
block get_block(chunk_manager *cm, vec3l pos);
void set_block(chunk_manager *cm, vec3l pos, block b);

void draw_chunks(chunk_manager *cm, context *c);
void init_world_noise();
pick_info pick_block(chunk_manager *world, vec3s pos, vec3s facing, float max_distance);

void world_to_block_and_chunk(vec3i *chunk, vec3i *block, vec3l block_global);
void test_wtbc();

#endif