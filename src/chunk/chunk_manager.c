#include "chunk_common.h"


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



// this is an effort to synchronize all the lighting and stuff and make it behave.
void neighbour_handshake(chunk_manager *cm, chunk *this, vec3i neighbour_pos) {
    int i = hmgeti(cm->chunk_hm, neighbour_pos);
    if (i == -1) {
        // neighbour not loaded, return
        return;
    }
    chunk *neighbour = &cm->chunk_hm[i];
    
    int this_nn = ++(this->loaded_4con_neighbours);
    int other_nn = ++(neighbour->loaded_4con_neighbours);
    
    if (this_nn == 6) {
        light_initialize_for_chunk(cm, spread(this->key));
        this->needs_remesh = true;
    }
    if (other_nn == 6) {
        light_initialize_for_chunk(cm, spread(neighbour->key));
        neighbour->needs_remesh = true;
    }
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


void cm_load_chunk(chunk_manager *cm, int x, int y, int z) {
    chunk new_chunk = chunk_initialize(x,y,z);
    hmputs(cm->chunk_hm, new_chunk);
    cm->gen_func(cm, x, y, z);

    int idx = hmgeti(cm->chunk_hm, ((vec3i){x,y,z}));
    chunk *c = &cm->chunk_hm[idx];

    /*
    neighbour_handshake(cm, c, (vec3i){x+1, y, z});
    neighbour_handshake(cm, c, (vec3i){x-1, y, z});
    neighbour_handshake(cm, c, (vec3i){x, y+1, z});
    neighbour_handshake(cm, c, (vec3i){x, y-1, z});
    neighbour_handshake(cm, c, (vec3i){x, y, z+1});
    neighbour_handshake(cm, c, (vec3i){x, y, z-1});
    */

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

void cm_lod_update(chunk_manager *cm, vec3s pos) {
    //vec3i in_chunk = world_pos_to_chunk(pos);
    vec3i in_chunk = {
        (int)pos.x >> LODMESH_LOG2,
        (int)pos.y >> LODMESH_LOG2,
        (int)pos.z >> LODMESH_LOG2,
    };

    int32_t_pair lod_min = {in_chunk.x - cm->lod_dimensions.l/2, in_chunk.z - cm->lod_dimensions.r/2};
    int32_t_pair lod_max = {in_chunk.x + cm->lod_dimensions.l/2, in_chunk.z + cm->lod_dimensions.r/2};

        // see which chunks we can unload. unload criteria
    for (int idx = 0; idx < hmlen(cm->lodmesh_hm); idx++) {
        int32_t_pair key = cm->lodmesh_hm[idx].key;
        if (key.l < lod_min.l || key.r < lod_min.r || key.l > lod_max.l || key.r > lod_max.r) {
            lodmesh_delete(cm, key.l, key.r);
        }
    }   

    for (int x = lod_min.l; x < lod_max.l; x++) {
        for (int z = lod_min.r; z < lod_max.r; z++) {
            int32_t_pair lod_pos = {x,z};
            int i = hmgeti(cm->lodmesh_hm, lod_pos);
            if (i == -1) {
                lodmesh m = lodmesh_generate(cm->osn, cm->noise_params, 16, x, z);
                hmputs(cm->lodmesh_hm, m);
            }
        }
    }

}

extern double cum_mesh_time;
extern double cum_gen_time;
extern double cum_light_time;
extern double cum_decorate_time;

extern double max_mesh_time;
extern double max_gen_time;
extern double max_light_time;
extern double max_decorate_time;

// todo priority queue and some heuristic
// or maybe hashmap
int cm_load_n(chunk_manager *cm, vec3s pos, int n) {
    vec3i in_chunk = world_pos_to_chunk(pos);
    vec3i load_min = vec3i_sub(in_chunk, vec3i_div(cm->loaded_dimensions, 2));
    vec3i load_max = vec3i_add(in_chunk, vec3i_div(cm->loaded_dimensions, 2));
    
    int amt_actually_loaded = 0;
    // does it load n per frame or n-1
    while (arrlen(cm->load_list) > 0 && amt_actually_loaded < n) {
        double tstart = glfwGetTime();
        vec3i k = arrpop(cm->load_list);

        // check that it hasnt been loaded yet and that we still want it loaded
        // (meaning its still in the loading volume)
        if (hmgeti(cm->chunk_hm, k) < 0 && vec3i_bounded_inclusive(load_max, load_min, k)) {
            cm_load_chunk(cm, spread(k));
            arrpush(cm->decorate_list, k);
            amt_actually_loaded++;
        }
        double tend = glfwGetTime();
        double dt = tend - tstart;
        
        cum_gen_time += dt;
        max_gen_time = max(max_gen_time, dt);
    }

    return amt_actually_loaded;
}

int cm_decorate_n(chunk_manager *cm, vec3s pos, int n) {
    vec3i in_chunk = world_pos_to_chunk(pos);
    vec3i load_min = vec3i_sub(in_chunk, vec3i_div(cm->loaded_dimensions, 2));
    vec3i load_max = vec3i_add(in_chunk, vec3i_div(cm->loaded_dimensions, 2));
    
    int amt_actually_loaded = 0;
    // does it load n per frame or n-1
    while (arrlen(cm->decorate_list) > 0 && amt_actually_loaded < n) {
        double tstart = glfwGetTime();
        vec3i k = arrpop(cm->decorate_list);

        // check that it hasnt been loaded yet and that we still want it loaded
        // (meaning its still in the loading volume)
        if (hmgeti(cm->chunk_hm, k) > -1) {
            chunk_decorate(cm, spread(k));
            arrpush(cm->light_list, k);
            amt_actually_loaded++;
        }
        double tend = glfwGetTime();
        double dt = tend - tstart;
        
        cum_decorate_time += dt;
        max_decorate_time = max(max_gen_time, dt);
    }

    return amt_actually_loaded;
}

int cm_light_n(chunk_manager *cm, vec3s pos, int n) {
    vec3i in_chunk = world_pos_to_chunk(pos);
    vec3i load_min = vec3i_sub(in_chunk, vec3i_div(cm->loaded_dimensions, 2));
    vec3i load_max = vec3i_add(in_chunk, vec3i_div(cm->loaded_dimensions, 2));

    int i = 0;
    int amt_actually_loaded = 0;
    // does it load n per frame or n-1
    while (arrlen(cm->light_list) > 0 && amt_actually_loaded < n) {
        double tstart = glfwGetTime();
        vec3i k = arrpop(cm->light_list);

        // check that it hasnt been loaded yet and that we still want it loaded
        // (meaning its still in the loading volume)
        if (hmgeti(cm->chunk_hm, k) > -1) {
            light_initialize_for_chunk(cm, spread(k));
            arrpush(cm->mesh_list, k);
            amt_actually_loaded++;
        }
        double tend = glfwGetTime();
        double dt = tend - tstart;
        
        cum_light_time += dt;
        max_light_time = max(max_light_time, dt);
    }

    return amt_actually_loaded;
}

int cm_mesh_n(chunk_manager *cm, vec3s pos, int n) {
    vec3i in_chunk = world_pos_to_chunk(pos);
    vec3i load_min = vec3i_sub(in_chunk, vec3i_div(cm->loaded_dimensions, 2));
    vec3i load_max = vec3i_add(in_chunk, vec3i_div(cm->loaded_dimensions, 2));

    int amt_actually_loaded = 0;
    // does it load n per frame or n-1
    while (arrlen(cm->mesh_list) > 0 && amt_actually_loaded < n) {
        double tstart = glfwGetTime();

        vec3i k = arrpop(cm->mesh_list);

        // check that it hasnt been loaded yet and that we still want it loaded
        // (meaning its still in the loading volume)
        if (hmgeti(cm->chunk_hm, k) > -1) {
            cm_mesh_chunk(cm, spread(k));
            amt_actually_loaded++;
        }
        double tend = glfwGetTime();
        double dt = tend - tstart;
        
        cum_mesh_time += dt;
        max_mesh_time = max(max_mesh_time, dt);
    }

    return amt_actually_loaded;
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

    //chunk_manager cm = {0};
    //open_simplex_noise(123456789, &cm.osn);

    lodmesh *lm_hm = {0};
    lodmesh m = {0};
    m.key.l = 1;
    m.key.r = 1;
    hmputs(lm_hm, m);

    int i = hmgeti(lm_hm, m.key);
    assert_int_equal("hmgeti 0 in", 0, i);

    assert_bool_equal("bounded inclusive 1", vec3i_bounded_inclusive((vec3i){10,10,10}, (vec3i){-10,-10,-10}, (vec3i){0,0,0}), true);
    assert_bool_equal("bounded inclusive 1b", vec3i_bounded_inclusive((vec3i){10,10,10}, (vec3i){-10,-10,-10}, (vec3i){0,20,0}), false);
    assert_bool_equal("bounded inclusive 1b", vec3i_bounded_inclusive((vec3i){10,10,10}, (vec3i){-10,-10,-10}, (vec3i){0,0,20}), false);
    assert_bool_equal("bounded inclusive 1a", vec3i_bounded_inclusive((vec3i){10,10,10}, (vec3i){-10,-10,-10}, (vec3i){20,0,0}), false);
    assert_bool_equal("bounded inclusive 2", vec3i_bounded_inclusive((vec3i){10,10,10}, (vec3i){-10,-10,-10}, (vec3i){20,20,20}), false);
    assert_bool_equal("bounded inclusive 3", vec3i_bounded_inclusive((vec3i){10,10,10}, (vec3i){0,0,0}, (vec3i){-5,-5,-5}), false);
    assert_bool_equal("bounded inclusive 4", vec3i_bounded_inclusive((vec3i){63,6,87}, (vec3i){53,-4,77}, (vec3i){58,1,82}), true);
    


}