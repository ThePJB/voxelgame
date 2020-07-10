#include "world.h"
#include "easing.h"
#include <stdlib.h>
#include "simplex.h"
#include "util.h"
#include "chunk.h"

#include "stb_ds.h"


void draw_chunks(chunk_manager *cm, graphics_context *c) {
    double ox = 0.5;
    double oy = 0.5;
    double oz = 0.5;

    glBindTexture(GL_TEXTURE_2D, c->atlas);
    glUseProgram(c->chunk_program);

    for (int idx = 0; idx < hmlen(cm->chunk_slots); idx++) {
        chunk_slot cs = cm->chunk_slots[idx].value;

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

void world_to_block_and_chunk(vec3i *chunk, vec3i *block, vec3l block_global) {
    single_w_t_bc(&(chunk->x), &(block->x), block_global.x);
    single_w_t_bc(&(chunk->y), &(block->y), block_global.y);
    single_w_t_bc(&(chunk->z), &(block->z), block_global.z);
}

block get_block(chunk_manager *cm, vec3l pos) {
    vec3i chunk_coords;
    vec3i block_coords;
    world_to_block_and_chunk(&chunk_coords, &block_coords, pos);
    int idx = hmgeti(cm->chunk_slots, chunk_coords);

    if (idx > -1) {
        chunk c = cm->chunk_slots[idx].value.chunk;
        check_chunk_invariants(c);
        if (c.empty) {
            debugf("empty get\n");
            return (block) {BLOCK_AIR};
        }
        return c.blocks->blocks[arr_3d_to_1d(block_coords)];
    } else {
        // didnt find
        debugf("didnt find %ld %ld %ld\n", pos.x, pos.y, pos.z);
        return (block) {
            .tag = BLOCK_AIR, // todo make this separate to air so player dosnt fall lol
        };
    }    
}

void set_block(chunk_manager *cm, vec3l pos, block b) {    
    vec3i chunk_coords;
    vec3i block_coords;
    world_to_block_and_chunk(&chunk_coords, &block_coords, pos);
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
        cs->chunk.blocks->blocks[arr_3d_to_1d(block_coords)] = b;
        mesh_chunk_slot(cs);
    } else {
        // didnt find
        debugf("didnt find %ld %ld %ld\n", pos.x, pos.y, pos.z);
    }
}

// --------------- meshing

#define MESHING_BUF_SIZE 409600 * 5
void mesh_chunk_slot(chunk_slot *cs) {
    float vertices[MESHING_BUF_SIZE] = {0};

    int num_triangles = get_chunk_vertex_data(cs->chunk, vertices, MESHING_BUF_SIZE);
    cs->num_triangles = num_triangles;
    //printf("meshed chunk, %d triangles\n", num_triangles);

    // bind vao and vertix attribs
    glBindVertexArray(cs->vao);
    glBindBuffer(GL_ARRAY_BUFFER, cs->vbo);
    glBufferData(GL_ARRAY_BUFFER, num_triangles * 3 * 9 * sizeof(float), vertices, GL_STATIC_DRAW); // sizeof vertices u retard

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texture coords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // normals
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // block type
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// --------------- picking

float intbound(float s, float ds) {
    if (ds < 0) {
        return intbound(-s, -ds);
    } else {
        if (ds > 0) {
            s = s - floorf(s);
        } else {
            s = s - ceilf(s);
        }
        return (1-s)/ds;
    }
}

vec3l nearest_block_pos(vec3s pos) {
    vec3l ret;
    if (pos.x > 0) {
        ret.x = pos.x;
    } else {
        ret.x = pos.x - 1;
    }
    if (pos.y > 0) {
        ret.y = pos.y;
    } else {
        ret.y = pos.y - 1;
    }
    if (pos.z > 0) {
        ret.z = pos.z;
    } else {
        ret.z = pos.z - 1;
    }
    return ret;
}

pick_info pick_block(chunk_manager *world, vec3s pos, vec3s facing, float max_distance) {
    debugf("facing %.2f, %.2f, %.2f\n", facing.x, facing.y, facing.z);
    debugf("at %.2f, %.2f, %.2f\n", pos.x, pos.y, pos.z);

    pick_info ret = {0};
    ret.success = true;

    ret.coords = nearest_block_pos(pos);

    int sx = signum(facing.x);
    int sy = signum(facing.y);
    int sz = signum(facing.z);

    float tMaxX = intbound(pos.x, facing.x);
    float tMaxY = intbound(pos.y, facing.y);
    float tMaxZ = intbound(pos.z, facing.z);

    debugf("initial tmx: %f, tmy: %.2f, tmz: %f\n", tMaxX, tMaxY, tMaxZ);
    //debugf("sx %d sy %d sz %d\n", sx, sy, sz);

    float accX = 0;
    float accY = 0;
    float accZ = 0;

    float tDeltaX = (float)sx / facing.x;
    float tDeltaY = (float)sy / facing.y;
    float tDeltaZ = (float)sz / facing.z;

    float max_squared = max_distance*max_distance;

    int n = 0;
    while (accX*accX + accY*accY + accZ*accZ <= max_squared) {
        n++;
        block_tag t = get_block(world, ret.coords).tag;
        debugf("x: %ld y: %ld z: %ld, t: %d\n", ret.coords.x, ret.coords.y, ret.coords.z, t);
        debugf("x dist: %.3f, y dist: %.3f, z dist: %.3f\n", accX, accY, accZ);
        //printf("tmx: %.2f, tmy: %.2f, tmz: %.2f\n", tMaxX, tMaxY, tMaxZ);
        if (t != BLOCK_AIR) {
            debugf("found block\n");
            ret.success=true;
            return ret;
        }

        if (tMaxX < tMaxY) {
            if (tMaxX < tMaxZ) {
                // X min
                ret.coords.x += sx;
                accX = tMaxX;
                tMaxX += tDeltaX;
                ret.normal_x = -sx;
                ret.normal_y = 0;
                ret.normal_z = 0;
            } else {
                // Z min
                ret.coords.z += sz;
                accZ = tMaxZ;
                tMaxZ += tDeltaZ;
                ret.normal_x = 0;
                ret.normal_y = 0;
                ret.normal_z = -sz;
            }
        } else {
            if (tMaxY < tMaxZ) {
                // Y min
                ret.coords.y += sy;
                accY = tMaxY;
                tMaxY += tDeltaY;
                ret.normal_x = 0;
                ret.normal_y = -sy;
                ret.normal_z = 0;
            } else {
                // Z min (again)
                ret.coords.z += sz;
                accZ = tMaxZ;
                tMaxZ += tDeltaZ;
                ret.normal_x = 0;
                ret.normal_y = 0;
                ret.normal_z = -sz;
            }
        }

    }
    ret.success = false;
    debugf("didnt find anything after n iters %d\n", n);
    //printf("bailed with accx: %.2f, accy: %.2f, accz: %.2f\n", accX, accY, accZ);

    return ret;
    
}



// ----------- testing

void print_chunk_slot(chunk_slot *cs) {
    printf("vao: %u vbo: %u num_tris: %d chunk: ", cs->vao, cs->vbo, cs->num_triangles);
    print_chunk(cs->chunk);
}

void test_world() {
    // nearest block
    vec3l np;

    np = nearest_block_pos((vec3s){0.1, 0.9, -0.1});
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

    // 3d to 1d
    /*
    assert_int_equal("unit z", cm_3d_to_1d(0, 0, 1), 1);
    assert_int_equal("unit x", cm_3d_to_1d(1, 0, 0), MAX_CHUNKS_S);
    assert_int_equal("unit y", cm_3d_to_1d(0, 1, 0), MAX_CHUNKS_SS);
    assert_int_equal("unit xyz", cm_3d_to_1d(1, 1, 1), MAX_CHUNKS_SS + MAX_CHUNKS_S + 1);
    
    // 1d to 3d
    assert_vec3i_equal("unit z", cm_1d_to_3d(1), 0, 0, 1);
    assert_vec3i_equal("unit x", cm_1d_to_3d(MAX_CHUNKS_S), 1, 0, 0);
    assert_vec3i_equal("unit y", cm_1d_to_3d(MAX_CHUNKS_SS), 0, 1, 0);
    

    // 3d to 1d and back
    assert_int_equal("3d 1d and back 10", cm_3d_to_1d_vec(cm_1d_to_3d(10)), 10);
    assert_int_equal("3d 1d and back -10", cm_3d_to_1d_vec(cm_1d_to_3d(-10)), -10);
    assert_int_equal("3d 1d and back 0", cm_3d_to_1d_vec(cm_1d_to_3d(0)), 0);
    assert_int_equal("3d 1d and back 64", cm_3d_to_1d_vec(cm_1d_to_3d(64)), 64);

    //assert_floatequal("intbound 10", intbound())

    // etc
    */

}