#include "world.h"
#include "easing.h"
#include <stdlib.h>
#include "simplex.h"
#include "util.h"
#include "chunk.h"

#include "stb_ds.h"

block_definition block_defs[] = {
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

void draw_chunks(chunk_manager *cm, graphics_context *c) {
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

void world_to_block_and_chunk(vec3i *chunk, vec3i *block, vec3l block_global) {
    single_w_t_bc(&(chunk->x), &(block->x), block_global.x);
    single_w_t_bc(&(chunk->y), &(block->y), block_global.y);
    single_w_t_bc(&(chunk->z), &(block->z), block_global.z);
}

vec3l block_and_chunk_to_world(vec3i block, vec3i chunk) {
    return (vec3l) {
        single_bc_t_w(block.x, chunk.x),
        single_bc_t_w(block.y, chunk.y),
        single_bc_t_w(block.z, chunk.z),
    };
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
        if (block_defs[b.tag].luminance > 0) {
            cm_add_light(cm, block_defs[b.tag].luminance, spread(pos));
        }
        cm_mesh_chunk(cm, spread(cm->chunk_slots[idx].value.chunk));
    } else {
        // didnt find
        debugf("didnt find %ld %ld %ld\n", pos.x, pos.y, pos.z);
    }
}

uint8_t cm_get_illumination(chunk_manager *cm, vec3l pos) {
    vec3i chunk_pos;
    vec3i block_pos;
    world_to_block_and_chunk(&chunk_pos, &block_pos, pos);
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
    return c.block_light->level[arr_3d_to_1d(block_pos)];
}

void cm_set_illumination(chunk_manager *cm, vec3l pos, uint8_t illumination) {
    printf("setting illumination at %ld %ld %ld to %u\n", spread(pos), illumination);
    vec3i chunk_pos;
    vec3i block_pos;
    world_to_block_and_chunk(&chunk_pos, &block_pos, pos);
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
    c.block_light->level[arr_3d_to_1d(block_pos)] = illumination;
    cm->chunk_slots[idx].value.chunk.needs_remesh = true;
}

void lighting_process_node(chunk_manager *cm, vec3l_queue *vq, 
    uint8_t current_illumination, vec3l target_pos) {
    
    uint8_t illumination_at_target = cm_get_illumination(cm, target_pos);
    block block_at_target = get_block(cm, target_pos);

    if (block_defs[block_at_target.tag].opaque) {
        return;
    }

    if (illumination_at_target < current_illumination - 1) {
        cm_set_illumination(cm, target_pos, current_illumination - 1);
        vec3l_queue_push(vq, target_pos);
    }
}

void cm_add_light(chunk_manager *cm, uint8_t luminance, long x, long y, long z) {
    // todo adjust size and warn if it gets too big or whatever
    vec3l bfqueue[CHUNK_RADIX_3];
    vec3l_queue vq = {
        .items = bfqueue,
        .start = 0,
        .end = 0,
        .size = CHUNK_RADIX_3,
    };

    vec3l global_source_pos = {x,y,z};

    vec3l_queue_push(&vq, global_source_pos);
    cm_set_illumination(cm, global_source_pos, luminance);

    printf("queue len %d\n", vec3l_queue_len(&vq));
    while(vec3l_queue_len(&vq) > 0) {
        printf("a");
        vec3l current_pos = vec3l_queue_pop(&vq);
        uint8_t current_illumination = cm_get_illumination(cm, current_pos);

        // neighbours
        vec3l n_px = current_pos; n_px.x++; lighting_process_node(cm, &vq, current_illumination, n_px);
        vec3l n_mx = current_pos; n_mx.x--; lighting_process_node(cm, &vq, current_illumination, n_mx);

        vec3l n_py = current_pos; n_py.y++; lighting_process_node(cm, &vq, current_illumination, n_py);
        vec3l n_my = current_pos; n_my.y--; lighting_process_node(cm, &vq, current_illumination, n_my);

        vec3l n_pz = current_pos; n_pz.z++; lighting_process_node(cm, &vq, current_illumination, n_pz);
        vec3l n_mz = current_pos; n_mz.z--; lighting_process_node(cm, &vq, current_illumination, n_mz);
        printf("queue len %d\n", vec3l_queue_len(&vq));
    }
}


// --------------- meshing

#define FACE_MINUS_Z 0
#define FACE_PLUS_Z 1
#define FACE_MINUS_X 2
#define FACE_PLUS_X 3
#define FACE_MINUS_Y 4
#define FACE_PLUS_Y 5

#define VERT_STRIDE 8

// directions for getting neighbours
#define PLUS_X CHUNK_RADIX_2
#define PLUS_Y CHUNK_RADIX
#define PLUS_Z 1

#define MINUS_X -PLUS_X
#define MINUS_Y -PLUS_Y
#define MINUS_Z -PLUS_Z


#define MESHING_BUF_SIZE 409600 * 5
void cm_mesh_chunk(chunk_manager *cm, int x, int y, int z) {
    int chunk_idx = hmgeti(cm->chunk_slots, ((vec3i){x,y,z}));
    chunk_slot *cs = &cm->chunk_slots[chunk_idx].value;
    cs->chunk.needs_remesh = false;
    float buf[MESHING_BUF_SIZE] = {0};

    chunk c = cs->chunk;

    if (c.empty) {
        return;
    }
    int vertex_idx = 0;
    const float cube_verts[] = 
        #include "cube.h"

    for (int idx = 0; idx < CHUNK_RADIX_3; idx++) {
        if (c.blocks->blocks[idx].tag == BLOCK_AIR) {
            continue;
        }

        vec3i coords = arr_1d_to_3d(idx);

        for (int face = 0; face < 6; face++) {
            // Skip occluded faces
            if (face == FACE_PLUS_X && neighbour_exists(coords, PLUS_X) && c.blocks->blocks[idx + PLUS_X].tag != BLOCK_AIR) {
                continue;
            } else if (face == FACE_MINUS_X && neighbour_exists(coords, MINUS_X) && c.blocks->blocks[idx + MINUS_X].tag != BLOCK_AIR) {
                continue;
            } else if (face == FACE_PLUS_Y && neighbour_exists(coords, PLUS_Y) && c.blocks->blocks[idx + PLUS_Y].tag != BLOCK_AIR) {
                continue;
            } else if (face == FACE_MINUS_Y && neighbour_exists(coords, MINUS_Y) && c.blocks->blocks[idx + MINUS_Y].tag != BLOCK_AIR) {
                continue;
            } else if (face == FACE_PLUS_Z && neighbour_exists(coords, PLUS_Z) && c.blocks->blocks[idx + PLUS_Z].tag != BLOCK_AIR) {
                continue;
            } else if (face == FACE_MINUS_Z && neighbour_exists(coords, MINUS_Z) && c.blocks->blocks[idx + MINUS_Z].tag != BLOCK_AIR) {
                continue;
            }

            for (int cube_vert = 0; cube_vert < 6; cube_vert++) {

                // walk the cube verts
                const float *current_vert = &(cube_verts[face * 6 * VERT_STRIDE + cube_vert * VERT_STRIDE]);
                
                // push to the vertex buffer
                buf[vertex_idx++] = current_vert[0] + coords.x; // x pos
                buf[vertex_idx++] = current_vert[1] + coords.y; // y pos 
                buf[vertex_idx++] = current_vert[2] + coords.z; // z pos
                buf[vertex_idx++] = current_vert[3]; // normal x
                buf[vertex_idx++] = current_vert[4]; // normal y
                buf[vertex_idx++] = current_vert[5]; // normal z
                buf[vertex_idx++] = current_vert[6]; // normal u
                buf[vertex_idx++] = current_vert[7]; // normal 
                buf[vertex_idx++] = c.blocks->blocks[idx].tag - 1.0; // block type 

                int light_max = 16;

                vec3i chunk_pos = {cs->chunk.x, cs->chunk.y, cs->chunk.z};
                vec3i block_pos = arr_1d_to_3d(idx);
                vec3l pos = block_and_chunk_to_world(block_pos, chunk_pos);
                if (face == FACE_PLUS_X) {
                    pos.x++;
                    buf[vertex_idx++] = unlerp(0, light_max, cm_get_illumination(cm, pos));
                } else if (face == FACE_MINUS_X) {
                    pos.x--;
                    buf[vertex_idx++] = unlerp(0, light_max, cm_get_illumination(cm, pos));                    
                } else if (face == FACE_PLUS_Y) {
                    pos.y++;
                    buf[vertex_idx++] = unlerp(0, light_max, cm_get_illumination(cm, pos));                    
                } else if (face == FACE_MINUS_Y) {
                    pos.y--;
                    buf[vertex_idx++] = unlerp(0, light_max, cm_get_illumination(cm, pos));                    
                } else if (face == FACE_PLUS_Z) {
                    pos.z++;
                    buf[vertex_idx++] = unlerp(0, light_max, cm_get_illumination(cm, pos));                    
                } else if (face == FACE_MINUS_Z) {
                    pos.z--;
                    buf[vertex_idx++] = unlerp(0, light_max, cm_get_illumination(cm, pos));                    
                }

                /*
                if (face == FACE_PLUS_X) {
                    if (neighbour_exists(coords, PLUS_X)) {
                        buf[vertex_idx++] = unlerp(0, light_max, (float)c.block_light->level[idx + PLUS_X]);
                    } else {
                        pos.x++;
                        buf[vertex_idx++] = unlerp(0, light_max, cm_get_illumination(cm, pos));
                    }
                } else if (face == FACE_MINUS_X) {
                    buf[vertex_idx++] = unlerp(0, light_max, (float)c.block_light->level[idx + MINUS_X]);
                } else if (face == FACE_PLUS_Y) {
                    buf[vertex_idx++] = unlerp(0, light_max, (float)c.block_light->level[idx + PLUS_Y]);
                } else if (face == FACE_MINUS_Y) {
                    buf[vertex_idx++] = unlerp(0, light_max, (float)c.block_light->level[idx + MINUS_Y]);
                } else if (face == FACE_PLUS_Z) {
                    buf[vertex_idx++] = unlerp(0, light_max, (float)c.block_light->level[idx + PLUS_Z]);
                } else if (face == FACE_MINUS_Z) {
                    buf[vertex_idx++] = unlerp(0, light_max, (float)c.block_light->level[idx + MINUS_Z]);
                }
                */
            }
        }
    }

    //printf("vertex idx: %d / %d\n", vertex_idx, buflen);
    cs->num_triangles =  vertex_idx / (VERT_STRIDE+2) / 3;
    //printf("meshed chunk, %d triangles\n", num_triangles);

    const int num_attribs_per_vertex = 10;

    // bind vao and vertix attribs
    glBindVertexArray(cs->vao);
    glBindBuffer(GL_ARRAY_BUFFER, cs->vbo);
    glBufferData(GL_ARRAY_BUFFER, cs->num_triangles * 3 * num_attribs_per_vertex * sizeof(float), buf, GL_STATIC_DRAW); // sizeof vertices u retard

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, num_attribs_per_vertex * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texture coords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, num_attribs_per_vertex * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // normals
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, num_attribs_per_vertex * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // block type
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, num_attribs_per_vertex * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // light level
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, num_attribs_per_vertex * sizeof(float), (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(4);

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

}