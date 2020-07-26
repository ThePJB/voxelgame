#include "chunk_common.h"

block_definition block_defs[NUM_BLOCKS] = {
    {
        .opaque = false,
        .luminance = 0,
    },
    {
        .opaque = true,
        .luminance = 0,
    },
    {
        .opaque = true,
        .luminance = 0,
    },
    {
        .opaque = true,
        .luminance = 0,
    },
    {
        .opaque = true,
        .luminance = 0,
    },
    {
        .opaque = true,
        .luminance = 0,
    },
    {
        .opaque = true,
        .luminance = 0,
    },
    {
        .opaque = true,
        .luminance = 0,
    },
    {
        .opaque = true,
        .luminance = 8,
    },
    {
        .opaque = true,
        .luminance = 6,
    },

};

// Converting positions
vec3i world_posl_to_chunk(int gx, int gy, int gz) {
    return (vec3i) {
        gx >> CHUNK_RADIX_LOG2,
        gy >> CHUNK_RADIX_LOG2,
        gz >> CHUNK_RADIX_LOG2,
    };
}

vec3i_pair world_posl_to_block_chunk(int gx, int gy, int gz) {
    vec3i_pair ret;
    return (vec3i_pair) {
        (vec3i) {
            mod(gx, CHUNK_RADIX),
            mod(gy, CHUNK_RADIX),
            mod(gz, CHUNK_RADIX),
        },
        (vec3i) {
            gx >> CHUNK_RADIX_LOG2,
            gy >> CHUNK_RADIX_LOG2,
            gz >> CHUNK_RADIX_LOG2,
        },
    };
}

vec3l world_block_chunk_to_posl(int bx, int by, int bz, int cx, int cy, int cz) {
    return (vec3l) {
        cx * CHUNK_RADIX + bx,
        cy * CHUNK_RADIX + by,
        cz * CHUNK_RADIX + bz,
    };
}

// -------------------- world getblock setblock

maybe_block_tag world_get_block(chunk_manager *cm, vec3l pos) {
    vec3i_pair coords = world_posl_to_block_chunk(spread(pos));
    int idx = hmgeti(cm->chunk_hm, coords.r);

    if (idx == -1) {
        // not loaded
        return (maybe_block_tag) {0, false};
    }

    return (maybe_block_tag) {cm->chunk_hm[idx].blocks[chunk_3d_to_1d(spread(coords.l))], true};  
}

void world_set_block(chunk_manager *cm, vec3l pos, block_tag new_block) {    
    vec3i_pair coords = world_posl_to_block_chunk(spread(pos));
    vec3i block_coords = coords.l;
    vec3i chunk_coords = coords.r;
    int chunk_idx = hmgeti(cm->chunk_hm, chunk_coords);
    int block_idx = chunk_3d_to_1d(spread(block_coords));


    if (chunk_idx == -1) {
        // not loaded
        return;
    }

    chunk *c = &cm->chunk_hm[chunk_idx];

    block_tag old_block = c->blocks[block_idx];
    c->blocks[block_idx] = new_block;

    uint8_t new_luminance = block_defs[new_block].luminance;
    uint8_t old_luminance = block_defs[old_block].luminance;

    bool new_opaque = block_defs[new_block].opaque;
    bool old_opaque = block_defs[old_block].opaque;
    

    // now for the light update cases

    if (new_luminance > 0 && old_luminance == 0) {
        // previously not luminous, now luminous
        light_add(cm, new_luminance, spread(pos));

    } else if (old_luminance > 0 && new_luminance == 0) {
        // previously luminous, now not
        light_delete(cm, spread(pos));

    } else if (old_opaque && !new_opaque) {
        // deleted a block that may have been obstructing light
        light_delete(cm, spread(pos));
        //cm_update_light_for_block_deletion(cm, spread(pos));

    } else if (new_opaque && !old_opaque) {
        // placed a block that may now obstruct light
        light_delete(cm, spread(pos));
        //cm_update_light_for_block_placement(cm, spread(pos));

    }

    if (block_defs[new_block].opaque) {
        world_update_surface_y(cm, spread(pos));
    } else {
        // TODO remove highest block 
    }
    
    c->needs_remesh = true;
}


maybe_int32_t world_get_surface_y(chunk_manager *cm, int32_t x, int32_t z) {
    int surface_map_idx;
    int32_t_pair surface_key = {x, z};

    if ((surface_map_idx = hmgeti(cm->surface_hm, surface_key)) != -1) {
        return (maybe_int32_t){cm->surface_hm[surface_map_idx].value, true};
    }
    
    //printf("warning: tried to get surface of unloaded chunk\n");
    return (maybe_int32_t){0, false};
}

// Here we maintain the "highest opaque block" structure
void world_update_surface_y(chunk_manager *cm, int32_t x, int32_t y, int32_t z) {
    maybe_int32_t current_surface_y = world_get_surface_y(cm, x, z);
    if (current_surface_y.ok) {
        if (current_surface_y.value < y) {
            hmput(cm->surface_hm, ((int32_t_pair){x,z}), y);
        }
    } else {
        hmput(cm->surface_hm, ((int32_t_pair){x,z}), y);
    }
}

// ----------- testing

void world_test() {
    // get set surface y
    chunk_manager cm = {0};
    world_update_surface_y(&cm, 120, 5, 120);
    assert_bool_equal("surface set 1 ok", world_get_surface_y(&cm, 120, 120).ok, true);
    assert_int_equal("surface set 1", world_get_surface_y(&cm, 120, 120).value, 5);
    world_update_surface_y(&cm, 120, 3, 120);
    assert_bool_equal("surface set 2 ok", world_get_surface_y(&cm, 120, 120).ok, true);
    assert_int_equal("surface set 2", world_get_surface_y(&cm, 120, 120).value, 5);
    world_update_surface_y(&cm, 120, 6, 120);
    assert_bool_equal("surface set 2 ok", world_get_surface_y(&cm, 120, 120).ok, true);
    assert_int_equal("surface set 2", world_get_surface_y(&cm, 120, 120).value, 6);

}

void world_benchmark(int n) {
    printf("profiling with worldsize %d*%d*%d (%d chunks)\n", n,n,n,n*n*n);
    chunk_manager cm = {0};

    double t_start = glfwGetTime();

    open_simplex_noise(123456789, &cm.osn);
    cm.loaded_dimensions = (vec3i) {n,n,n};
    cm.gen_func = generate_v1;
    //cm.gen_func = generate_flat;

    cm_update(&cm, (vec3s) {0,0,0});
    cm_load_n(&cm, (vec3s) {0,0,0}, n*n*n);

    double t_chunks_done = glfwGetTime();

    for (int i = 0; i < hmlen(cm.chunk_hm); i++) {
        light_initialize_for_chunk(&cm, spread(cm.chunk_hm[i].key));
    } 

    double t_light_done = glfwGetTime();

    for (int i = 0; i < hmlen(cm.chunk_hm); i++) {
        cm_mesh_chunk(&cm, spread(cm.chunk_hm[i].key));
    } 

    double t_mesh_done = glfwGetTime();

    float t_gen = t_chunks_done - t_start;
    float t_light = t_light_done - t_chunks_done;
    float t_mesh = t_mesh_done - t_light_done;

    printf("Generating took %f s (%f ms per chunk)\n", t_gen, 1000 * t_gen / (n*n*n) );
    printf("Lighting took %f s (%f ms per chunk)\n", t_light, 1000 * t_light / (n*n*n));
    printf("Meshing took %f s (%f ms per chunk)\n", t_mesh,  1000 * t_mesh / (n*n*n));
}