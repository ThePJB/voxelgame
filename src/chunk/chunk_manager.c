#include "chunk_common.h"

void cm_load_chunk(chunk_manager *cm, int x, int y, int z) {
    debugf("loading chunk %d %d %d\n", x, y, z);
    chunk new_chunk = chunk_generate(cm->world_noise, x, y, z);
    hmputs(cm->chunk_hm, new_chunk);

    int idx = hmgeti(cm->chunk_hm, ((vec3i){x,y,z}));
    chunk *c = &cm->chunk_hm[idx];

    glGenVertexArrays(1, &c->vao);
    glGenBuffers(1, &c->vbo);

    if (!c->empty) {
        c->block_light_levels = calloc(CHUNK_RADIX_3, sizeof(uint8_t));
    }

    int neighbour_idx;
    if (neighbour_idx = hmgeti(cm->chunk_hm, ((vec3i){x+1, y, z})) > 0) {
        printf("4con before %d\n", cm->chunk_hm[neighbour_idx].loaded_4con_neighbours);
        int nn = cm->chunk_hm[neighbour_idx].loaded_4con_neighbours++;
        printf("4con after %d\n", nn);
        if (nn == 6) {
            cm_mesh_chunk(cm, x+1, y, z);
        }
    }
    if (neighbour_idx = hmgeti(cm->chunk_hm, ((vec3i){x-1, y, z})) > 0) {
        cm->chunk_hm[neighbour_idx].loaded_4con_neighbours++;
        c->loaded_4con_neighbours++;
        if (cm->chunk_hm[neighbour_idx].loaded_4con_neighbours == 6) {
            cm_mesh_chunk(cm, x-1, y, z);
        }
    }
    if (neighbour_idx = hmgeti(cm->chunk_hm, ((vec3i){x, y+1, z})) > 0) {
        cm->chunk_hm[neighbour_idx].loaded_4con_neighbours++;
        c->loaded_4con_neighbours++;
        if (cm->chunk_hm[neighbour_idx].loaded_4con_neighbours == 6) {
            cm_mesh_chunk(cm, x, y+1, z);
        }
    }
    if (neighbour_idx = hmgeti(cm->chunk_hm, ((vec3i){x-1, y-1, z})) > 0) {
        cm->chunk_hm[neighbour_idx].loaded_4con_neighbours++;
        c->loaded_4con_neighbours++;
        if (cm->chunk_hm[neighbour_idx].loaded_4con_neighbours == 6) {
            cm_mesh_chunk(cm, x, y-1, z);
        }
    }
    if (neighbour_idx = hmgeti(cm->chunk_hm, ((vec3i){x, y, z+1})) > 0) {
        cm->chunk_hm[neighbour_idx].loaded_4con_neighbours++;
        c->loaded_4con_neighbours++;
        if (cm->chunk_hm[neighbour_idx].loaded_4con_neighbours == 6) {
            cm_mesh_chunk(cm, x, y, z + 1);
        }
    }
    if (neighbour_idx = hmgeti(cm->chunk_hm, ((vec3i){x, y, z-1})) > 0) {
        cm->chunk_hm[neighbour_idx].loaded_4con_neighbours++;
        c->loaded_4con_neighbours++;
        if (cm->chunk_hm[neighbour_idx].loaded_4con_neighbours == 6) {
            cm_mesh_chunk(cm, x, y, z - 1);
        }
    }
}

void cm_unload_chunk(chunk_manager *cm, int x, int y, int z) {
    debugf("unloading chunk %d %d %d\n", x, y, z);
    int cs_idx = hmgeti(cm->chunk_hm, ((vec3i){x,y,z}));
    chunk *c = &cm->chunk_hm[cs_idx];
    glDeleteBuffers(1, &c->vbo);
    glDeleteVertexArrays(1, &c->vao);
    if (c->blocks != NULL) {
        free(c->blocks);
        free(c->block_light_levels);
        // sky light levels as well
    }
    int neighbour_idx;
    if (neighbour_idx = hmgeti(cm->chunk_hm, ((vec3i){x+1, y, z})) > 0) {
        cm->chunk_hm[neighbour_idx].loaded_4con_neighbours--;
    }
    if (neighbour_idx = hmgeti(cm->chunk_hm, ((vec3i){x-1, y, z})) > 0) {
        cm->chunk_hm[neighbour_idx].loaded_4con_neighbours--;
    }
    if (neighbour_idx = hmgeti(cm->chunk_hm, ((vec3i){x, y+1, z})) > 0) {
        cm->chunk_hm[neighbour_idx].loaded_4con_neighbours--;
    }
    if (neighbour_idx = hmgeti(cm->chunk_hm, ((vec3i){x-1, y-1, z})) > 0) {
        cm->chunk_hm[neighbour_idx].loaded_4con_neighbours--;
    }
    if (neighbour_idx = hmgeti(cm->chunk_hm, ((vec3i){x, y, z+1})) > 0) {
        cm->chunk_hm[neighbour_idx].loaded_4con_neighbours--;
    }
    if (neighbour_idx = hmgeti(cm->chunk_hm, ((vec3i){x, y, z-1})) > 0) {
        cm->chunk_hm[neighbour_idx].loaded_4con_neighbours--;
    }

    hmdel(cm->chunk_hm, ((vec3i){x,y,z}));
}

bool vec3i_bounded_inclusive(vec3i upper, vec3i lower, vec3i a) {
    return (a.x <= upper.x && a.x >= lower.x &&
        a.y <= upper.y && a.y >= lower.y &&
        a.z <= upper.z && a.z >= lower.z);
}

vec3i world_pos_to_chunk(vec3s pos) {
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
    vec3i in_chunk = world_pos_to_chunk(pos);
    vec3i load_min = vec3i_sub(in_chunk, vec3i_div(cm->loaded_dimensions, 2));
    vec3i load_max = vec3i_add(in_chunk, vec3i_div(cm->loaded_dimensions, 2));
    
    // see which chunks we can unload. unload criteria
    for (int idx = 0; idx < hmlen(cm->chunk_hm); idx++) {
        chunk c = cm->chunk_hm[idx];
        vec3i unload_chunk_pos = c.key;
        
        if (!vec3i_bounded_inclusive(load_max, load_min, unload_chunk_pos) && hmgeti(cm->chunk_hm, unload_chunk_pos) >= 0) {
            cm_unload_chunk(cm, spread(unload_chunk_pos));
        }
    }   

    // see which chunks need loading
    for (int x = load_min.x; x < load_max.x; x++) {
        for (int y = load_min.y; y < load_max.y; y++) {
            for (int z = load_min.z; z < load_max.z; z++) {
                if (hmgeti(cm->chunk_hm, ((vec3i){x,y,z})) < 0) {
                    arrpush(cm->load_list, ((vec3i){x,y,z}));
                }
            }
        }
    }
}

// todo priority queue and some heuristic
// or maybe hashmap
void cm_load_n(chunk_manager *cm, vec3s pos, int n) {
    vec3i in_chunk = world_pos_to_chunk(pos);
    vec3i load_min = vec3i_sub(in_chunk, vec3i_div(cm->loaded_dimensions, 2));
    vec3i load_max = vec3i_add(in_chunk, vec3i_div(cm->loaded_dimensions, 2));

    int i = 0;
    int amt_actually_loaded = 0;
    // does it load n per frame or n-1
    while (arrlen(cm->load_list) > 0 && amt_actually_loaded < n) {
        vec3i k = cm->load_list[i];

        // check that it hasnt been loaded yet and that we still want it loaded
        // (meaning its still in the loading volume)
        if (hmgeti(cm->chunk_hm, k) < 0 && vec3i_bounded_inclusive(load_max, load_min, in_chunk)) {
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

void cm_test() {
    chunk *hm;
    chunk a = {0};
    a.key = (vec3i){1,2,3};
    hmputs(hm, a);
    a.key = (vec3i){4,5,6};
    hmputs(hm, a);
    a.key = (vec3i){7,8,9};
    hmputs(hm, a);

    int idx789 = hmgeti(hm, ((vec3i){7,8,9}));
    assert_int_equal("hm 789", idx789, 2);
    int idx456 = hmgeti(hm, ((vec3i){4,5,6}));
    assert_int_equal("hm 456", idx456, 1);

}