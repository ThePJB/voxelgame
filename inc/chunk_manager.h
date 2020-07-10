#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include "chunk.h"

/*
matters pertaining to loading and unloading chunks
*/

typedef enum {
    CS_LOADED,
    CS_TO_LOAD,
} cs_status;

// generated information for 
typedef struct {
    unsigned int vao;
    unsigned int vbo;
    int num_triangles;
    chunk chunk;
    cs_status status;
} chunk_slot;

typedef struct {
    chunk_rngs world_noise;
    struct {vec3i key; chunk_slot value; } *chunk_slots;
    vec3i loaded_dimensions;
    vec3i *load_list;
} chunk_manager;

void cm_update(chunk_manager *cm, vec3s pos);

void cm_load_n(chunk_manager *cm, vec3s pos, int n);


#endif