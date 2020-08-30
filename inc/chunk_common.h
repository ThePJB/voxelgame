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
//#include "stb.h"
#include "stb_ds.h"

#include "priority_queue.h"

#define LODMESH_LOG2 8
#define LODMESH_CHUNK_RADIX 256

#define CHUNK_RADIX_LOG2 4
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


#define SKY_LIGHT_FULL 16


typedef struct {
    // not affected by smoothing
    float *lf_height_amplitude;
    float *lf_height_frequency;
    
    // yes affected by smoothing
    float *hf_height_amplitude;
    float *hf_height_frequency;

    float *smooth_amplitude;
    float *smooth_frequency;

    float *cave_tendency_amplitude;
    float *cave_tendency_frequency;

    float *treeness_amplitude;
    float *treeness_frequency;

    int snow_above_height;
    int dirt_above_height;
    int sand_below_height;
    int water_below_height;

} noise2d_params;

// Information that the chunk cares about (would be saved)
typedef struct {
    vec3i key; // chunk pos, for hashtable
    unsigned int vao;
    unsigned int vbo;
    
    unsigned int water_vao;
    unsigned int water_vbo;
    // is this the kind of thing where u could have a chunk vao and voxel vbo and water vbo?
    
    int num_triangles;
    block_tag *blocks;
    uint8_t *block_light_levels;
    uint8_t *sky_light_levels;
    
    bool initial_lighting_done;
    bool needs_remesh;
    bool finished_loading;
    
    int loaded_4con_neighbours;

    int32_t chunk_seed;
    
} chunk;


typedef struct {
    int32_t_pair key;
    int32_t value;
} surface_hm_entry;


typedef struct {
    int32_t_pair key;
    int num_triangles;
    unsigned int vao;
    unsigned int vbo;

} lodmesh;

typedef struct chunk_manager {
    //chunk_rngs world_noise;
    struct osn_context* osn;
    noise2d_params noise_params;

    chunk *chunk_hm;
    surface_hm_entry *surface_hm;
    lodmesh *lodmesh_hm;

    vec3i loaded_dimensions;
    int32_t_pair lod_dimensions;

    vec3i_priority_queue load_queue;
    vec3i* decorate_list;
    vec3i* light_list;
    vec3i* mesh_list;

    void (*gen_func)(struct chunk_manager *cm, int chunk_x, int chunk_y, int chunk_z);

} chunk_manager;

typedef struct {
    bool success;
    vec3l coords;
    direction normal_dir;
} pick_info;

//block_definition block_defs[NUM_BLOCKS];

MKMAYBE(block_tag);

// low level -- chunks
//chunk_rngs chunk_rngs_init(int64_t seed);
chunk chunk_initialize(int x, int y, int z);
void chunk_generate(chunk_manager *cm, int cx, int cy, int cz);
void chunk_decorate(chunk_manager *cm, int cx, int cy, int cz);

void update_highest_block(chunk_manager *cm, int32_t x, int32_t y, int32_t z);
int chunk_3d_to_1d(int x, int y, int z);
vec3i chunk_1d_to_3d(int idx);
void chunk_print(chunk c);
void chunk_test();

float generate_height(struct osn_context *osn, float x, float z, noise2d_params p);

// chunk manager
void cm_update(chunk_manager *cm, vec3s pos);           // queue up chunks to load
int cm_load_n(chunk_manager *cm, vec3s pos, int n);    // load a limited number of chunks
bool neighbour_exists(vec3i pos, int direction);
void cm_test();

// higher level -- world data structures

// coordinate conversions
vec3i_pair world_posl_to_block_chunk(int gx, int gy, int gz);
vec3l world_block_chunk_to_posl(int bx, int by, int bz, int cx, int cy, int cz);
vec3i world_posl_to_chunk(int gx, int gy, int gz);


// access
maybe_block_tag world_get_block(chunk_manager *cm, int gx, int gy, int gz);
void world_set_block(chunk_manager *cm, int gx, int gy, int gz, block_tag b);
void world_set_block_without_lighting(chunk_manager *cm, int gx, int gy, int gz, block_tag new_block);

maybe_int32_t world_get_surface_y(chunk_manager *cm, int32_t x, int32_t z);
void world_update_surface_y(chunk_manager *cm, int32_t x, int32_t y, int32_t z);

void world_test();
void world_benchmark();

// meshing
void cm_mesh_chunk(chunk_manager *cm, int x, int y, int z);

// lighting
maybe_uint8_t light_get_block(chunk_manager *cm, vec3l pos);
void light_set_block(chunk_manager *cm, vec3l pos, uint8_t illumination);

maybe_uint8_t light_get_sky(chunk_manager *cm, vec3l pos);
void light_set_sky(chunk_manager *cm, vec3l pos, uint8_t illumination);

void light_initialize_for_chunk(chunk_manager *cm, int x, int y, int z);
void light_add(chunk_manager *cm, uint8_t luminance, long x, long y, long z);
void light_delete(chunk_manager *cm, long x, long y, long z);
void light_issue_remesh(chunk_manager *cm, vec3l pos);
void light_propagate_sky(chunk_manager *cm, int32_t x, int32_t y, int32_t z);

// picking
pick_info pick_block(chunk_manager *world, vec3s pos, vec3s facing, float max_distance);

// lodmesh
lodmesh lodmesh_generate(struct osn_context *osn, noise2d_params p, int n, int cx, int cz);
void cm_lod_update(chunk_manager *cm, vec3s pos);
void lodmesh_draw(lodmesh m, graphics_context *ctx);
void lodmesh_delete(chunk_manager *cm, int cx, int cz);

#endif