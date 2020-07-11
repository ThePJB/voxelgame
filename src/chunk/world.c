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
        .luminance = 16,
    },

};

void world_draw(chunk_manager *cm, graphics_context *c) {
    double ox = 0.5;
    double oy = 0.5;
    double oz = 0.5;

    glBindTexture(GL_TEXTURE_2D, c->atlas);
    glUseProgram(c->chunk_program);

    for (int idx = 0; idx < hmlen(cm->chunk_slots); idx++) {
        chunk_slot cs = cm->chunk_slots[idx].value;
        if (cs.chunk.needs_remesh) {
            cm_mesh_chunk(cm, spread(cm->chunk_slots[idx].value.chunk));
        }

        mat4s model = GLMS_MAT4_IDENTITY_INIT;
        model = glms_translate(model, (vec3s){
            cs.chunk.x*CHUNK_RADIX + ox, 
            cs.chunk.y*CHUNK_RADIX + oy, 
            cs.chunk.z*CHUNK_RADIX + oz,
        });

        glUniformMatrix4fv(glGetUniformLocation(c->mesh_program, "model"), 1, GL_FALSE, model.raw[0]);
        glBindVertexArray(cs.vao);
        glDrawArrays(GL_TRIANGLES, 0, cs.num_triangles * 3);
    }
}


// -------------------- world getblock setblock
long single_bc_t_w(int b, int c) {
    return c * CHUNK_RADIX + b;
}

void single_w_t_bc(int *c, int *b, int g) {
    if (g < 0) {
        g -= 16;
        *c = (g+1) / CHUNK_RADIX;
        *b = (g % CHUNK_RADIX + 16) % CHUNK_RADIX;
    } else {
        *c = g / CHUNK_RADIX;
        *b = (g % CHUNK_RADIX + 16) % CHUNK_RADIX;
    }
}

vec3l world_pos_to_block_pos(vec3s pos) {
    return (vec3l) {
        floorf(pos.x),
        floorf(pos.y),
        floorf(pos.z)};
}

void world_global_to_block_chunk(vec3i *chunk, vec3i *block, vec3l block_global) {
    single_w_t_bc(&(chunk->x), &(block->x), block_global.x);
    single_w_t_bc(&(chunk->y), &(block->y), block_global.y);
    single_w_t_bc(&(chunk->z), &(block->z), block_global.z);
}

vec3l world_block_chunk_to_global(vec3i block, vec3i chunk) {
    return (vec3l) {
        single_bc_t_w(block.x, chunk.x),
        single_bc_t_w(block.y, chunk.y),
        single_bc_t_w(block.z, chunk.z),
    };
}

block world_get_block(chunk_manager *cm, vec3l pos) {
    vec3i chunk_coords;
    vec3i block_coords;
    world_global_to_block_chunk(&chunk_coords, &block_coords, pos);
    int idx = hmgeti(cm->chunk_slots, chunk_coords);

    if (idx > -1) {
        chunk c = cm->chunk_slots[idx].value.chunk;
        check_chunk_invariants(c);
        if (c.empty) {
            debugf("empty get\n");
            return (block) {BLOCK_AIR};
        }
        return c.blocks->blocks[chunk_3d_to_1d(block_coords)];
    } else {
        // didnt find
        debugf("didnt find %ld %ld %ld\n", pos.x, pos.y, pos.z);
        return (block) {
            .tag = BLOCK_AIR, // todo make this separate to air so player dosnt fall lol
        };
    }    
}

void world_set_block(chunk_manager *cm, vec3l pos, block b) {    
    vec3i chunk_coords;
    vec3i block_coords;
    world_global_to_block_chunk(&chunk_coords, &block_coords, pos);
    int idx = hmgeti(cm->chunk_slots, chunk_coords);
    
    if (idx > -1) {
        chunk_slot *cs = &cm->chunk_slots[idx].value;
        check_chunk_invariants(cs->chunk);
        if (cs->chunk.empty) {
            debugf("empty set\n");
            // not empty and allocate memory
            cs->chunk.empty = false;
            cs->chunk.blocks = calloc(sizeof(chunk_blocks), 1);
        }
        cs->chunk.blocks->blocks[chunk_3d_to_1d(block_coords)] = b;
        if (block_defs[b.tag].luminance > 0) {
            cm_add_light(cm, block_defs[b.tag].luminance, spread(pos));
        }
        cm_mesh_chunk(cm, spread(cm->chunk_slots[idx].value.chunk));
    } else {
        // didnt find
        debugf("didnt find %ld %ld %ld\n", pos.x, pos.y, pos.z);
    }
}

uint8_t world_get_illumination(chunk_manager *cm, vec3l pos) {
    vec3i chunk_pos;
    vec3i block_pos;
    world_global_to_block_chunk(&chunk_pos, &block_pos, pos);
    int idx = hmgeti(cm->chunk_slots, chunk_pos);
    
    if (idx < 0) {
        printf("tried getting illumination of an unloaded chunk\n");
        return 255;
    }

    if (cm->chunk_slots[idx].value.chunk.empty) {
        // later would probably want this to upgrade the chunk from empty
        printf("tried getting illumination of an empty chunk\n");
        return 255;
    }

    chunk c = cm->chunk_slots[idx].value.chunk;
    return c.block_light->level[chunk_3d_to_1d(block_pos)];
}

void world_set_illumination(chunk_manager *cm, vec3l pos, uint8_t illumination) {
    printf("setting illumination at %ld %ld %ld to %u\n", spread(pos), illumination);
    vec3i chunk_pos;
    vec3i block_pos;
    world_global_to_block_chunk(&chunk_pos, &block_pos, pos);
    int idx = hmgeti(cm->chunk_slots, chunk_pos);
    
    if (idx < 0) {
        printf("tried setting illumination of an unloaded chunk\n");
        return;
    }

    if (cm->chunk_slots[idx].value.chunk.empty) {
        printf("tried setting illumination of an empty chunk\n");
        return;
    }

    chunk c = cm->chunk_slots[idx].value.chunk;
    c.block_light->level[chunk_3d_to_1d(block_pos)] = illumination;
    cm->chunk_slots[idx].value.chunk.needs_remesh = true;
}


// ----------- testing

void chunk_print_slot(chunk_slot *cs) {
    printf("vao: %u vbo: %u num_tris: %d chunk: ", cs->vao, cs->vbo, cs->num_triangles);
    chunk_print(cs->chunk);
}

void world_test() {
    // nearest block
    vec3l np;

    np = world_pos_to_block_pos((vec3s){0.1, 0.9, -0.1});
    assert_int_equal("np x 0.1", np.x, 0);
    assert_int_equal("np y 0.9", np.y, 0);
    assert_int_equal("np z -0.1", np.z, -1);
    
    // wtbc
    int chunk_ord = 0;
    int block_ord = 0;

    single_w_t_bc(&chunk_ord, &block_ord, 0);
    assert_int_equal("0 chunk", chunk_ord, 0);
    assert_int_equal("0 block", block_ord, 0);

    single_w_t_bc(&chunk_ord, &block_ord, 1);
    assert_int_equal("1 chunk", chunk_ord, 0);
    assert_int_equal("1 block", block_ord, 1);

    single_w_t_bc(&chunk_ord, &block_ord, -1);
    assert_int_equal("-1 chunk", chunk_ord, -1);
    assert_int_equal("-1 block", block_ord, 15);

    single_w_t_bc(&chunk_ord, &block_ord, 15);
    assert_int_equal("15 chunk", chunk_ord, 0);
    assert_int_equal("15 block", block_ord, 15);

    single_w_t_bc(&chunk_ord, &block_ord, 16);
    assert_int_equal("16 chunk", chunk_ord, 1);
    assert_int_equal("16 block", block_ord, 0);

    single_w_t_bc(&chunk_ord, &block_ord, -15);
    assert_int_equal("-15 chunk", chunk_ord, -1);
    assert_int_equal("-15 block", block_ord, 1);

    single_w_t_bc(&chunk_ord, &block_ord, -16);
    assert_int_equal("-16 chunk", chunk_ord, -1);
    assert_int_equal("-16 block", block_ord, 0);

    single_w_t_bc(&chunk_ord, &block_ord, -17);
    assert_int_equal("-17 chunk", chunk_ord, -2);
    assert_int_equal("-17 block", block_ord, 15);



    // intbound
    assert_float_equal("intbound 1", intbound(-1.5, 1), 0.5);
    assert_float_equal("intbound 2", intbound(-1.6, 1), 0.6);
    assert_float_equal("intbound 3", intbound(-1.6, 0.5), 1.2);

    assert_float_equal("intbound 4", intbound(1.5, -1), 0.5);
    assert_float_equal("intbound 5", intbound(-1.6, -1), 0.4);
    assert_float_equal("intbound 6", intbound(-1.6, -0.5), 0.8);

    assert_float_equal("intbound 7", intbound(1.5, 1), 0.5);
    assert_float_equal("intbound 8", intbound(1.6, 1), 0.4);
    assert_float_equal("intbound 9", intbound(1.6, 0.5), 0.8);

}