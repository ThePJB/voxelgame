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

// -------------------- world getblock setblock

block_tag world_get_block(chunk_manager *cm, vec3l pos) {
    vec3i_pair coords = world_posl_to_block_chunk(pos);
    int idx = hmgeti(cm->chunk_hm, coords.r);

    if (idx == -1) {
        // not loaded
        return BLOCK_AIR;
    }

    if (cm->chunk_hm[idx].empty) {
        // empty chunk
        return BLOCK_AIR;
    }

    return cm->chunk_hm[idx].blocks[chunk_3d_to_1d(coords.l)];  
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

    if (c->empty) {
        // its an empty chunk, we must upgrade it
        // todo
        // clear empty flag, allocate arrays etc
    }

    block_tag old_block = c->blocks[block_idx];
    c->blocks[block_idx] = new_block;

    uint8_t new_luminance = block_defs[new_block].luminance;
    uint8_t old_luminance = block_defs[old_block].luminance;

    bool new_opaque = block_defs[new_block].opaque;
    bool old_opaque = block_defs[old_block].opaque;
    


    // first make sure block luminance is set to correct value
    // (bit nasty) should be unnecessary

    // the line that makes light deletion work and the other shit break
    //world_set_illumination(cm, pos, block_defs[old_block].luminance);


    // now for the light update cases

    if (new_luminance > 0 && old_luminance == 0) {
        // previously not luminous, now luminous
        cm_add_light(cm, new_luminance, spread(pos));

    } else if (old_luminance > 0 && new_luminance == 0) {
        // previously luminous, now not
        cm_delete_light(cm, spread(pos));

    } else if (old_opaque && !new_opaque) {
        // deleted a block that may have been obstructing light
        cm_update_light_for_block_deletion(cm, spread(pos));

    } else if (new_opaque && !old_opaque) {
        // placed a block that may now obstruct light
        cm_update_light_for_block_placement(cm, spread(pos));

    }

    cm_mesh_chunk(cm, spread(chunk_coords));
}

uint8_t world_get_illumination(chunk_manager *cm, vec3l pos) {
    vec3i_pair coords = world_posl_to_block_chunk(pos);
    int idx = hmgeti(cm->chunk_hm, coords.r);
    
    if (idx < 0) {
        printf("tried getting illumination of an unloaded chunk %d %d %d\n", spread(coords.r));
        return LIGHT_EMPTY_CHUNK;
    }

    if (cm->chunk_hm[idx].empty) {
        // later would probably want this to upgrade the chunk from empty
        printf("tried getting illumination of an empty chunk\n");
        return 255;
    }

    return cm->chunk_hm[idx].block_light_levels[chunk_3d_to_1d(coords.l)];
}


// issues chunk updates including to neighbouring chunks if the block is on an edge


void world_set_illumination(chunk_manager *cm, vec3l pos, uint8_t illumination) {
    vec3i_pair coords = world_posl_to_block_chunk(pos);
    int idx = hmgeti(cm->chunk_hm, coords.r);
    
    if (idx < 0) {
        printf("tried setting illumination of an unloaded chunk\n");
        return;
    }

    if (cm->chunk_hm[idx].empty) {
        printf("tried setting illumination of an empty chunk\n");
        return;
    }
    cm_lighting_touch_block(cm, pos);
    cm->chunk_hm[idx].block_light_levels[chunk_3d_to_1d(coords.l)] = illumination;
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

// ----------- testing

void world_test() {

}