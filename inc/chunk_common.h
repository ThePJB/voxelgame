#ifndef CHUNK_COMMON_H
#define CHUNK_COMMON_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <cglm/struct.h>

#include "simplex.h"
#include "noise.h"
#include "block.h"
#include "util.h"
#include "graphics.h"
#include "stb_ds.h"

#define CHUNK_RADIX 16
#define CHUNK_RADIX_2 (CHUNK_RADIX*CHUNK_RADIX)
#define CHUNK_RADIX_3 (CHUNK_RADIX*CHUNK_RADIX*CHUNK_RADIX)

#define CHUNK_MAX (CHUNK_RADIX-1)
#define CHUNK_MAX_2 (CHUNK_RADIX * CHUNK_MAX)
#define CHUNK_MAX_3 (CHUNK_RADIX * CHUNK_MAX_2)

// directions for getting neighbours
#define PLUS_X CHUNK_RADIX_2
#define PLUS_Y CHUNK_RADIX
#define PLUS_Z 1

#define MINUS_X -PLUS_X
#define MINUS_Y -PLUS_Y
#define MINUS_Z -PLUS_Z

#define LIGHT_EMPTY_CHUNK 255

typedef struct {
    noise2d noise_lf_heightmap;
    noise2d noise_hf_heightmap;
    noise3d noise_cliff_carver;
    noise3d noise_cave_carver;
} chunk_rngs;

// Information that the chunk cares about (would be saved)
typedef struct {
    vec3i key; // chunk pos, for hashtable
    unsigned int vao;
    unsigned int vbo;
    int num_triangles;
    bool empty; // also all_one_block is a possibility, not sure how applicable that is
    block_tag *blocks;
    uint8_t *block_light_levels;
    uint8_t *sky_light_levels;
    
    bool needs_remesh;
    int loaded_4con_neighbours;
    
} chunk;


typedef struct {
    chunk_rngs world_noise;
    chunk *chunk_hm;
    vec3i loaded_dimensions;
    vec3i *load_list;
} chunk_manager;

typedef struct {
    bool success;
    vec3l coords;
    direction normal_dir;
} pick_info;

block_definition block_defs[NUM_BLOCKS];

// low level -- chunks
chunk_rngs chunk_rngs_init(int64_t seed);
chunk chunk_generate(chunk_rngs noise, int x, int y, int z);
int chunk_3d_to_1d(vec3i pos);
vec3i chunk_1d_to_3d(int idx);
void chunk_print(chunk c);
void chunk_test();

// chunk manager
void cm_update(chunk_manager *cm, vec3s pos);           // queue up chunks to load
void cm_load_n(chunk_manager *cm, vec3s pos, int n);    // load a limited number of chunks
bool neighbour_exists(vec3i pos, int direction);
void cm_test();

// higher level -- world
void world_global_to_block_chunk(vec3i *chunk, vec3i *block, vec3l block_global);
vec3l world_block_chunk_to_global(vec3i block, vec3i chunk);
block_tag world_get_block(chunk_manager *cm, vec3l pos);
void world_set_block(chunk_manager *cm, vec3l pos, block_tag b);
vec3l world_pos_to_block_pos(vec3s pos);
vec3i world_pos_to_chunk_pos(vec3s pos);

uint8_t world_get_illumination(chunk_manager *cm, vec3l pos);
void world_set_illumination(chunk_manager *cm, vec3l pos, uint8_t illumination);
void world_draw(chunk_manager *cm, graphics_context *c);
void world_test();

// meshing
void cm_mesh_chunk(chunk_manager *cm, int x, int y, int z);

// lighting
void cm_mesh_chunk(chunk_manager *cm, int x, int y, int z);
void cm_add_light(chunk_manager *cm, uint8_t luminance, long x, long y, long z);
void cm_delete_light(chunk_manager *cm, long x, long y, long z);
void cm_update_light_for_block_deletion(chunk_manager *cm, long x, long y, long z);

// picking
pick_info pick_block(chunk_manager *world, vec3s pos, vec3s facing, float max_distance);

#endif