#ifndef WORLD_H
#define WORLD_H

#include "graphics.h"
#include "util.h"
#include "chunk.h"
#include <stdbool.h>
#include <cglm/struct.h>

#include "simplex.h"


/*
    In this file we deal with chunks in aggregate
*/

typedef struct {
    bool success;
    vec3l coords;
    int normal_x;
    int normal_y;
    int normal_z;
} pick_info;


// generated information for 
typedef struct {
    unsigned int vao;
    unsigned int vbo;
    int num_triangles;
    chunk chunk;
    
    // opt
    bool all_one_block;
    block_tag block;
} chunk_slot;

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
#define MAX_CHUNKS_S 32
#define MAX_CHUNKS_SS (MAX_CHUNKS_S*MAX_CHUNKS_S)

#define MAX_CHUNKS_Y 8
#define MAX_CHUNKS_SYS (MAX_CHUNKS_SS * MAX_CHUNKS_Y)

typedef struct {
    struct osn_context *noise_context;
    chunk_slot chunk_slots[MAX_CHUNKS_SYS];
} chunk_manager;


void init_chunk_manager(chunk_manager *cm, int seed);

void draw_chunk(chunk *ch, graphics_context *c);
chunk_slot *get_chunk_slot(chunk_manager *cm, vec3i chunk_coords);

// basically gets it to update and load chunks
void chunk_manager_position_hint(chunk_manager *cm, vec3s pos);
block get_block(chunk_manager *cm, vec3l pos);
void set_block(chunk_manager *cm, vec3l pos, block b);

void draw_chunks(chunk_manager *cm, graphics_context *c);
void init_world_noise();
pick_info pick_block(chunk_manager *world, vec3s pos, vec3s facing, float max_distance);

void world_to_block_and_chunk(vec3i *chunk, vec3i *block, vec3l block_global);
void test_world();

#endif