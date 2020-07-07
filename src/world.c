#include "world.h"
#include "easing.h"
#include <stdlib.h>
#include "simplex.h"
#include "util.h"
#include "chunk.h"

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


//#define MESHING_BUF_SIZE 819200
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

void draw_chunks(chunk_manager *cm, graphics_context *c) {
    double ox = 0.5;
    double oy = 0.5;
    double oz = 0.5;

    glBindTexture(GL_TEXTURE_2D, c->atlas);
    glUseProgram(c->chunk_program);

    for (int i = 0; i < MAX_CHUNKS_SYS; i++) {
        chunk_slot *slot = &cm->chunk_slots[i];
        chunk *chunk = &slot->chunk;

        mat4s model = GLMS_MAT4_IDENTITY_INIT;
        model = glms_translate(model, (vec3s){
            chunk->x*CHUNK_RADIX + ox, 
            chunk->y*CHUNK_RADIX + oy, 
            chunk->z*CHUNK_RADIX + oz,
        });

        glUniformMatrix4fv(glGetUniformLocation(c->mesh_program, "model"), 1, GL_FALSE, model.raw[0]);
        glBindVertexArray(slot->vao);
        glDrawArrays(GL_TRIANGLES, 0, slot->num_triangles * 3);
    }
}

void init_chunk_slot(chunk_manager *cm, int idx) {
    printf("initing chunk slot %d", idx);
    glGenVertexArrays(1, &cm->chunk_slots[idx].vao);
    glGenBuffers(1, &cm->chunk_slots[idx].vbo);
    printf("vao: %u, vbo: %u\n", cm->chunk_slots[idx].vao, cm->chunk_slots[idx].vbo);
}

void free_chunk_slot(chunk_slot *cs) {
    chunk_blocks *blocks_ptr = cs->chunk.blocks;
    printf("freeing %p\n", blocks_ptr);
    if (blocks_ptr != NULL) {
        if (cs->chunk.empty) {
            printf("chunk should not be empty\n");
        }
        printf("freeing it anyway for some reason\n");
        free(blocks_ptr);
    }
}

chunk_slot *get_chunk_slot(chunk_manager *cm, vec3i chunk_coords) {
    for (int i = 0; i < MAX_CHUNKS_SYS; i++) {
        chunk_slot *cpi = &cm->chunk_slots[i];
        if (cpi->chunk.x == chunk_coords.x && cpi->chunk.y == chunk_coords.y && cpi->chunk.z == chunk_coords.z) {
            return cpi;
        }
    }
    return NULL;
}

int cm_3d_to_1d(int x, int y, int z) {
    return MAX_CHUNKS_SS * y + MAX_CHUNKS_S * x + z;
}

#define cm_3d_to_1d_vec(X) cm_3d_to_1d(X.x, X.y, X.z)

vec3i cm_1d_to_3d(int idx) {
    return (vec3i) {
        .z = idx % MAX_CHUNKS_S,
        .y = idx / (MAX_CHUNKS_S*MAX_CHUNKS_S),
        .x = (idx / MAX_CHUNKS_S) % MAX_CHUNKS_S,
    };
}

void init_chunk_manager(chunk_manager *cm, int64_t seed) {
    cm->world_noise = create_noise2d(seed, 5, 0.01, 2, 50, 0.5);
    for (int i = 0; i < MAX_CHUNKS_SYS; i++) {
        init_chunk_slot(cm, i);
    }
}

void generate_initial(chunk_manager *cm, vec3s pos) {
    printf("gen initial noise: %p\n", cm->world_noise.osn);
    vec3i chunk_array_dimensions = {MAX_CHUNKS_S, MAX_CHUNKS_Y, MAX_CHUNKS_S};

    // chunk that pos is in
    vec3i in_chunk = from_vec3s(glms_vec3_divs(pos, CHUNK_RADIX));

    // chunk that the bottom right maximum loaded chunk would contain
    vec3i chunk_offset = vec3i_sub(in_chunk, vec3i_div(chunk_array_dimensions, 2));

    for (int idx = 0; idx < MAX_CHUNKS_SYS; idx++) {
        // world chunk position
        vec3i chunk_pos = vec3i_add(cm_1d_to_3d(idx), chunk_offset);
        //vec3i chunk_pos = cm_1d_to_3d(idx);

        cm->chunk_slots[idx].chunk = generate_chunk(&cm->world_noise, spread(chunk_pos));
        mesh_chunk_slot(&cm->chunk_slots[idx]);
    }
}

void cm_abc(int x, int y, int z) {
    if (x < 0 || x >= MAX_CHUNKS_S || y < 0 || y >= MAX_CHUNKS_Y || z < 0 || z >= MAX_CHUNKS_S) {
        printf("ABC failed %d %d %d\n", x, y, z);
        exit(1);
    }
}

void chunk_treadmill(chunk_manager *cm, direction direction) {
    vec3i bottom_corner = {-MAX_CHUNKS_S/2, -MAX_CHUNKS_Y/2, -MAX_CHUNKS_S/2};

    printf("treadmil noise: %p\n", cm->world_noise.osn);
    if (direction == DIR_PX) {
        
        //int offset_low = mod(cm->offsets.x - (MAX_CHUNKS_S/2), MAX_CHUNKS_S);
        int i = mod(cm->offsets.x, MAX_CHUNKS_S);

        for (int k = 0; k < MAX_CHUNKS_S; k++) {
            for (int j = 0; j < MAX_CHUNKS_Y; j++) {
                vec3i chunk_slot_pos = {i, j, k};
                vec3i old_chunk_world_pos = vec3i_add(chunk_slot_pos, vec3i_add(bottom_corner, cm->offsets));
                //vec3i old_chunk_world_pos = vec3i_add(chunk_slot_pos, bottom_corner);
                vec3i new_chunk_world_pos = old_chunk_world_pos;
                new_chunk_world_pos.x += MAX_CHUNKS_S;

                printf("at chunk slot pos:"); print_vec3i(chunk_slot_pos);
                printf("\nunloading chunk "); print_vec3i(old_chunk_world_pos);
                printf("\nreplacing with chunk "); print_vec3i(new_chunk_world_pos); printf("\n");

                // not 100% sure about adding offsets to the world pos
                //printf("chunk slot %d %d %d\n", i, j, k);

                cm_abc(spread(chunk_slot_pos));
                int idx = cm_3d_to_1d(spread(chunk_slot_pos));
                // free_chunk_slot(&cm->chunk_slots[idx]);

                printf("generating\n");
                

                //cm->chunk_slots[idx].chunk = generate_chunk(cm->noise_context, cm->offsets.x + MAX_CHUNKS_S/2, j + cm->offsets.y, k + cm->offsets.z);
                printf("meshing\n");
                //mesh_chunk_slot(&cm->chunk_slots[idx]);
            }
        }

        cm->offsets.x++;

    }
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

block get_block(chunk_manager *cm, vec3l pos) {
    vec3i chunk_coords;
    vec3i block_coords;
    world_to_block_and_chunk(&chunk_coords, &block_coords, pos);
    chunk_slot *cs = get_chunk_slot(cm, chunk_coords);
    if (cs) {
        chunk c = cs->chunk;
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
    chunk_slot *cs = get_chunk_slot(cm, chunk_coords);
    if (cs) {
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

void print_chunk_slot(chunk_slot *cs) {
    printf("vao: %u vbo: %u num_tris: %d chunk: ", cs->vao, cs->vbo, cs->num_triangles);
    print_chunk(cs->chunk);
}

void print_world(chunk_manager *cm) {
    for (int x = 0; x < MAX_CHUNKS_S; x++) {
        for (int y = 0; y < MAX_CHUNKS_Y; y++) {
            for (int z = 0; z < MAX_CHUNKS_S; z++) {
                //printf("x: %d y: %d z: %d cs: ", x, y, z);
                int idx = MAX_CHUNKS_SS * y + MAX_CHUNKS_S * x + z;
                printf("blocks ptr: %p\n", cm->chunk_slots[idx].chunk.blocks);
                //print_chunk_slot(&cm->chunk_slots[idx]);
                //cm->chunk_slots[idx] = gen_chunk_slot(cm->noise_context, x + bottom_corner_x, y + bottom_corner_y, z + bottom_corner_z);
            }
        }
    }
    printf("osn ptr: %p\n", cm->world_noise.osn);
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

}