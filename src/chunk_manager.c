#include "graphics.h"
#include "util.h"
#include "chunk.h"
#include <stdbool.h>
#include <cglm/struct.h>

#include "noise.h"
#include "world.h"
#include "chunk_manager.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

void cm_load_chunk(chunk_manager *cm, int x, int y, int z) {
    debugf("loading chunk %d %d %d\n", x, y, z);
    chunk_slot cs = {0};
    glGenVertexArrays(1, &cs.vao);
    glGenBuffers(1, &cs.vbo);
    cs.status = CS_LOADED;
    cs.chunk = generate_chunk(cm->world_noise, x, y, z);
    mesh_chunk_slot(&cs);
    hmput(cm->chunk_slots, ((vec3i){x,y,z}), cs);
}

void cm_unload_chunk(chunk_manager *cm, int x, int y, int z) {
    debugf("unloading chunk %d %d %d\n", x, y, z);
    int cs_idx = hmgeti(cm->chunk_slots, ((vec3i){x,y,z}));
    chunk_slot cs = cm->chunk_slots[cs_idx].value;
    glDeleteBuffers(1, &cs.vbo);
    glDeleteVertexArrays(1, &cs.vao);
    if (cs.chunk.blocks != NULL) {
        free(cs.chunk.blocks);
    }
    hmdel(cm->chunk_slots, ((vec3i){x,y,z}));
}

bool bounded(vec3i upper, vec3i lower, vec3i a) {
    return (a.x <= upper.x && a.x >= lower.x &&
        a.y <= upper.y && a.y >= lower.y &&
        a.z <= upper.z && a.z >= lower.z);
}

vec3i pos_to_chunk(vec3s pos) {
    int posix = floorf(pos.x);
    int posiy = floorf(pos.y);
    int posiz = floorf(pos.z);

    if (posix < 0) posix++;
    if (posiy < 0) posiy++;
    if (posiz < 0) posiz++;

    return (vec3i) {
        .x = posix / CHUNK_RADIX,
        .y = posiy / CHUNK_RADIX,
        .z = posiz / CHUNK_RADIX,
    };
}

void cm_update(chunk_manager *cm, vec3s pos) {
    vec3i in_chunk = pos_to_chunk(pos);
    vec3i load_min = vec3i_sub(in_chunk, vec3i_div(cm->loaded_dimensions, 2));
    vec3i load_max = vec3i_add(in_chunk, vec3i_div(cm->loaded_dimensions, 2));
    //printf("enter %d %d %d\n", spread(in_chunk));
    //printf("load min %d %d %d\n", spread(load_min));
    //printf("load max %d %d %d\n", spread(load_max));
    
    // see which chunks we can unload. unload criteria
    for (int idx = 0; idx < hmlen(cm->chunk_slots); idx++) {
        chunk_slot cs = cm->chunk_slots[idx].value;
        vec3i unload_chunk_pos = (vec3i) {
            cs.chunk.x,
            cs.chunk.y,
            cs.chunk.z,
        };
        //printf("should we free %d %d %d? ", current_chunk_pos.x, current_chunk_pos.y, current_chunk_pos.z);
        if (!bounded(load_max, load_min, unload_chunk_pos) && hmgeti(cm->chunk_slots, unload_chunk_pos) >= 0) {
            cm_unload_chunk(cm, spread(unload_chunk_pos));
            //printf("yes\n");
        } else {
            //printf("no\n");
        }
    }   

    // see which chunks need loading
    for (int x = load_min.x; x < load_max.x; x++) {
        for (int y = load_min.y; y < load_max.y; y++) {
            for (int z = load_min.z; z < load_max.z; z++) {
                if (hmgeti(cm->chunk_slots, ((vec3i){x,y,z})) < 0) {
                    //printf("time to load %d %d %d\n", x, y, z);
                    arrpush(cm->load_list, ((vec3i){x,y,z}));
                    //cm_load_chunk(cm, x, y, z);
                }
            }
        }
    }
}

// todo priority queue and some heuristic
// or maybe hashmap
void cm_load_n(chunk_manager *cm, vec3s pos, int n) {
    vec3i in_chunk = pos_to_chunk(pos);
    vec3i load_min = vec3i_sub(in_chunk, vec3i_div(cm->loaded_dimensions, 2));
    vec3i load_max = vec3i_add(in_chunk, vec3i_div(cm->loaded_dimensions, 2));

    int i = 0;
    int amt_actually_loaded = 0;
    // does it load n per frame or n-1
    while (arrlen(cm->load_list) > 0 && amt_actually_loaded < n) {
        vec3i k = cm->load_list[i];

        // check that it hasnt been loaded yet and that we still want it loaded
        // (meaning its still in the loading volume)
        if (hmgeti(cm->chunk_slots, k) < 0 && bounded(load_max, load_min, in_chunk)) {
            cm_load_chunk(cm, spread(k));
            amt_actually_loaded++;
        }
        i++;
    }
    int del_amt = min(i, arrlen(cm->load_list));
    if (i > 0) {
        arrdeln(cm->load_list, 0, del_amt);    
    }
    
}