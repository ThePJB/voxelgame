#include "chunk_common.h"

void neighbour_handshake(chunk_manager *cm, chunk *this, vec3i neighbour_pos) {
    chunk *np;
    int i = hmgeti(cm->chunk_hm, neighbour_pos);
    if (i == -1) {
        // neighbour not loaded, return
        return;
    }
    np = &cm->chunk_hm[i];
    
    int this_nn = ++(this->loaded_4con_neighbours);
    int other_nn = ++(np->loaded_4con_neighbours);
    
    if (this_nn == 6) {
        cm_mesh_chunk(cm, spread(this->key));
        chunk_fix_lighting(cm, spread(this->key));
    }
    if (other_nn == 6) {
        cm_mesh_chunk(cm, spread(neighbour_pos));
        chunk_fix_lighting(cm, spread(this->key));
    }
}

void cm_load_chunk(chunk_manager *cm, int x, int y, int z) {
    debugf("loading chunk %d %d %d\n", x, y, z);
    chunk new_chunk = chunk_generate(cm, cm->world_noise, x, y, z);
    hmputs(cm->chunk_hm, new_chunk);

    int idx = hmgeti(cm->chunk_hm, ((vec3i){x,y,z}));
    chunk *c = &cm->chunk_hm[idx];

    glGenVertexArrays(1, &c->vao);
    glGenBuffers(1, &c->vbo);

    c->block_light_levels = calloc(CHUNK_RADIX_3, sizeof(uint8_t));

    neighbour_handshake(cm, c, (vec3i){x+1, y, z});
    neighbour_handshake(cm, c, (vec3i){x-1, y, z});
    neighbour_handshake(cm, c, (vec3i){x, y+1, z});
    neighbour_handshake(cm, c, (vec3i){x, y-1, z});
    neighbour_handshake(cm, c, (vec3i){x, y, z+1});
    neighbour_handshake(cm, c, (vec3i){x, y, z-1});

}

void neighbour_unhandshake(chunk_manager *cm, vec3i neighbour_pos) {
    chunk *np;
    int i = hmgeti(cm->chunk_hm, neighbour_pos);
    if (i == -1) {
        // neighbour not loaded, return
        return;
    }
    np = &cm->chunk_hm[i];
    np->loaded_4con_neighbours--; 
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
    neighbour_unhandshake(cm, (vec3i){x+1, y, z});
    neighbour_unhandshake(cm, (vec3i){x-1, y, z});
    neighbour_unhandshake(cm, (vec3i){x, y+1, z});
    neighbour_unhandshake(cm, (vec3i){x, y-1, z});
    neighbour_unhandshake(cm, (vec3i){x, y, z+1});
    neighbour_unhandshake(cm, (vec3i){x, y, z-1});

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
    vec3i akey = (vec3i){1,2,3};
    chunk *hm = NULL;
    chunk a = {0};
    a.vao = 123;
    a.key = akey;
    hmputs(hm, a);
    /*
    a.vao = 999;
    a.key = (vec3i){4,5,6};
    hmputs(hm, a);
    a.key = (vec3i){7,8,9};
    hmputs(hm, a);
    */

    printf("km 0 key: %d %d %d\n", spread(hm[0].key));
    printf("km 1 key: %d %d %d\n", spread(hm[1].key));
    printf("km 2 key: %d %d %d\n", spread(hm[2].key));

    assert_int_equal("hmgeti 123 0", 0, hmgeti(hm, akey));
    assert_int_equal("hmgets 123 vao", 123, hmgets(hm, akey).vao);


}