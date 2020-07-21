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
vec3i world_posl_to_chunk(vec3l pos) {
    return (vec3i) {
        floor_div(pos.x, CHUNK_RADIX),
        floor_div(pos.y, CHUNK_RADIX),
        floor_div(pos.z, CHUNK_RADIX),
    };
}

vec3i_pair world_posl_to_block_chunk(vec3l pos) {
    vec3i_pair ret;
    return (vec3i_pair) {
        (vec3i) {
            mod(pos.x, CHUNK_RADIX),
            mod(pos.y, CHUNK_RADIX),
            mod(pos.z, CHUNK_RADIX),
        },
        (vec3i) {
            floor_div(pos.x, CHUNK_RADIX),
            floor_div(pos.y, CHUNK_RADIX),
            floor_div(pos.z, CHUNK_RADIX),
        },
    };
}

vec3l world_block_chunk_to_posl(vec3i block, vec3i chunk) {
    return (vec3l) {
        chunk.x * CHUNK_RADIX + block.x,
        chunk.y * CHUNK_RADIX + block.y,
        chunk.z * CHUNK_RADIX + block.z,
    };
}

// -------------------- world getblock setblock

maybe_block_tag world_get_block(chunk_manager *cm, vec3l pos) {
    vec3i_pair coords = world_posl_to_block_chunk(pos);
    int idx = hmgeti(cm->chunk_hm, coords.r);

    if (idx == -1) {
        // not loaded
        return (maybe_block_tag) {0, false};
    }

    return (maybe_block_tag) {cm->chunk_hm[idx].blocks[chunk_3d_to_1d(coords.l)], true};  
}

void world_set_block(chunk_manager *cm, vec3l pos, block_tag new_block) {    
    vec3i_pair coords = world_posl_to_block_chunk(pos);
    vec3i block_coords = coords.l;
    vec3i chunk_coords = coords.r;
    int chunk_idx = hmgeti(cm->chunk_hm, chunk_coords);
    int block_idx = chunk_3d_to_1d(block_coords);


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




// draw

void world_draw(chunk_manager *cm, graphics_context *ctx) {
    glBindTexture(GL_TEXTURE_2D, ctx->atlas);
    glUseProgram(ctx->chunk_program);

    for (int idx = 0; idx < hmlen(cm->chunk_hm); idx++) {
        chunk *c = &cm->chunk_hm[idx];
        if (c->needs_remesh) {
            cm_mesh_chunk(cm, spread(c->key));
        }

        mat4s model = GLMS_MAT4_IDENTITY_INIT;
        model = glms_translate(model, (vec3s){
            c->key.x*CHUNK_RADIX + 0.5, 
            c->key.y*CHUNK_RADIX + 0.5, 
            c->key.z*CHUNK_RADIX + 0.5,
        });

        glUniformMatrix4fv(glGetUniformLocation(ctx->mesh_program, "model"), 1, GL_FALSE, model.raw[0]);
        glBindVertexArray(c->vao);
        glDrawArrays(GL_TRIANGLES, 0, c->num_triangles * 3);
    }
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