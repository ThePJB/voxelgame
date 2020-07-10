#ifndef WORLD_H
#define WORLD_H

#include "graphics.h"
#include "util.h"
#include "chunk.h"
#include <stdbool.h>
#include <cglm/struct.h>

#include "noise.h"
#include "chunk_manager.h"


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

void draw_chunk(chunk *ch, graphics_context *c);
chunk_slot *get_chunk_slot(chunk_manager *cm, vec3i chunk_coords);

// basically gets it to update and load chunks
block get_block(chunk_manager *cm, vec3l pos);
void set_block(chunk_manager *cm, vec3l pos, block b);

void draw_chunks(chunk_manager *cm, graphics_context *c);
void init_world_noise();
pick_info pick_block(chunk_manager *world, vec3s pos, vec3s facing, float max_distance);

void world_to_block_and_chunk(vec3i *chunk, vec3i *block, vec3l block_global);

void test_world();

void mesh_chunk_slot(chunk_slot *cs);

#endif