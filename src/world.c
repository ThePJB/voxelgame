#include "world.h"
#include "easing.h"
#include <stdlib.h>
#include "simplex.h"
#include "util.h"

struct osn_context *ctx;

void init_world_noise(int seed) {
    open_simplex_noise(seed, &ctx);
}


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

void generate_chunk(chunk *c, int x, int y, int z) {
    chunk_blocks *blocks = calloc(sizeof(block), CHUNK_RADIX*CHUNK_RADIX*CHUNK_RADIX);
    c->blocks = blocks;
    c->empty = true;

    c->x = x;
    c->y = y;
    c->z = z;



    float chunk_x = x*CHUNK_RADIX;
    float chunk_y = y*CHUNK_RADIX;
    float chunk_z = z*CHUNK_RADIX;

    const float amplitude = 50;

    for (int i = 0; i < CHUNK_RADIX; i++) {
        for (int k = 0; k < CHUNK_RADIX; k++) {
            double block_x = chunk_x + i;
            double block_z = chunk_z + k;

            double scale = 0.01;
            double v = 539847;

            float height = 
                amplitude * open_simplex_noise2(ctx, scale*block_x, scale*block_z) +
                amplitude/2 * open_simplex_noise2(ctx, v+2*scale*block_x, v+2*scale*block_z) +
                amplitude/4 * open_simplex_noise2(ctx, 2*v+4*scale*block_x, 2*v+4*scale*block_z) +
                amplitude/8 * open_simplex_noise2(ctx, 3*v+8*scale*block_x, 3*v+8*scale*block_z);


            for (int j = 0; j < CHUNK_RADIX; j++) {
                float block_y = chunk_y + j;

                // grass and stuff
                if (block_y < height - 4) {
                    blocks->blocks[i][j][k] = (block) {BLOCK_STONE};
                } else if (block_y < height - 0.5) {
                    // not stone layers
                    if (height > -25) {
                        blocks->blocks[i][j][k] = (block) {BLOCK_DIRT};
                    } else {
                        blocks->blocks[i][j][k] = (block) {BLOCK_SAND};
                    }
                } else if (block_y < height + 0.5) {
                    // top layer
                    if (height > 40) {
                        blocks->blocks[i][j][k] = (block) {BLOCK_SNOW};    
                    } else if (height > -25) {
                        blocks->blocks[i][j][k] = (block) {BLOCK_GRASS};
                    } else {
                        blocks->blocks[i][j][k] = (block) {BLOCK_SAND};
                    }
                } else {
                    blocks->blocks[i][j][k] = (block) {BLOCK_AIR};
                }

                // check empty
                if (blocks->blocks[i][j][k].tag != BLOCK_AIR) {
                    c->empty = false;
                }
            }
        }
    }

    if (c->empty) {
        free(c->blocks);
        c->blocks = NULL;
        printf("generated empty chunk at %d %d %d\n", x, y, z);
    } else {
        printf("generated chunk at %d %d %d\n", x, y, z);
    }
}


float vertices[819200] = {0};

// Currently I'll push position, texture and normals
// might turn out to be better to compute some of that stuff in place than send it over
// or it might not we'll see

#define VERT_STRIDE 8

int mesh_chunk(chunk *c, int vao, int vbo) {
    if (c->empty) {
        return 0;
    }
    int vertex_idx = 0;

    const float cube_verts[] = 
    #include "cube.h"
    // for now we are gonna do the dirty and generate geometry for every single block
    // next easiest is for a face to check if its occluded
    // then follow that with greedy meshing

    for (int i = 0; i < CHUNK_RADIX; i++) {
        for (int j = 0; j < CHUNK_RADIX; j++) {
            for (int k = 0; k < CHUNK_RADIX; k++) {

                if (c->blocks->blocks[i][j][k].tag == BLOCK_AIR) {
                    continue;
                }

                for (int face = 0; face < 6; face++) {
                    
                    // faces are numbered, -+z, -+x, -+y
                    if (face == 0 && k != 0 && c->blocks->blocks[i][j][k-1].tag != BLOCK_AIR) {
                        continue;
                    }             
                    
                    if (face == 1 && k != CHUNK_RADIX-1 && c->blocks->blocks[i][j][k+1].tag != BLOCK_AIR) {
                        continue;
                    }
                    if (face == 2 && i != 0 && c->blocks->blocks[i-1][j][k].tag != BLOCK_AIR) {
                        continue;
                    }
                    if (face == 3 && i != CHUNK_RADIX-1 && c->blocks->blocks[i+1][j][k].tag != BLOCK_AIR) {
                        continue;
                    }                    
                    if (face == 4 && j != 0 && c->blocks->blocks[i][j-1][k].tag != BLOCK_AIR) {
                        continue;
                    }
                    if (face == 5 && j != CHUNK_RADIX-1 && c->blocks->blocks[i][j+1][k].tag != BLOCK_AIR) {
                        continue;
                    }
                    

                    // now do vert faces
                    for (int cube_vert = 0; cube_vert < 6; cube_vert++) {

                        // walk the cube verts
                        const float *current_vert = &(cube_verts[face * 6 * VERT_STRIDE + cube_vert * VERT_STRIDE]);
                        
                        // push to the vertex buffer
                        vertices[vertex_idx++] = current_vert[0] + i; // x pos
                        vertices[vertex_idx++] = current_vert[1] + j; // y pos 
                        vertices[vertex_idx++] = current_vert[2] + k; // z pos
                        vertices[vertex_idx++] = current_vert[3]; // normal x
                        vertices[vertex_idx++] = current_vert[4]; // normal y
                        vertices[vertex_idx++] = current_vert[5]; // normal z
                        vertices[vertex_idx++] = current_vert[6]; // normal u
                        vertices[vertex_idx++] = current_vert[7]; // normal 
                        vertices[vertex_idx++] = c->blocks->blocks[i][j][k].tag - 1.0; // block type 
                    }
                }
            }
        }
    }

    int num_triangles = vertex_idx / (VERT_STRIDE+1) / 3;
    printf("meshed chunk, %d triangles\n", num_triangles);

    // now just bind the vao
    // how to clean up this stuff when i want to? good question

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

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

    return num_triangles;
}

void draw_chunks(chunk_manager *cm, context *c) {
    float ox = 0.5;
    float oy = 0.5;
    float oz = 0.5;

    glBindTexture(GL_TEXTURE_2D, c->atlas);
    glUseProgram(c->chunk_program);


    for (int i = 0; i < MAX_CHUNKS_SSS; i++) {
        mat4s model = GLMS_MAT4_IDENTITY_INIT;
        chunk_slot *slot = &cm->chunk_slots[i];
        chunk *chunk = &slot->c;
        model = glms_translate(model, (vec3s){chunk->x*CHUNK_RADIX + ox, chunk->y*CHUNK_RADIX + oy, chunk->z*CHUNK_RADIX + oz});
        glUniformMatrix4fv(glGetUniformLocation(c->mesh_program, "model"), 1, GL_FALSE, model.raw[0]);

        glBindVertexArray(slot->vao);
        glDrawArrays(GL_TRIANGLES, 0, slot->num_triangles * 3);
    }
}

// coords in chunk manager and coords of chunk
// idk if a better way to streamline this
// in fact initing vs hinting will be a separate thing
void init_chunk_slot(chunk_manager *cm, int x, int y, int z, int cx, int cy, int cz) {
    chunk_slot *cs = &cm->chunk_slots[MAX_CHUNKS_SS * x + MAX_CHUNKS_S * y + z];
    glGenVertexArrays(1, &cs->vao);
    glGenBuffers(1, &cs->vbo);
    generate_chunk(&cs->c, cx, cy, cz);
    cs->num_triangles = mesh_chunk(&cs->c, cs->vao, cs->vbo);
}

void chunk_manager_position_hint(chunk_manager *cm, vec3s pos) {
    int bottom_corner_x = pos.x/CHUNK_RADIX - MAX_CHUNKS_S/2;
    int bottom_corner_y = pos.y/CHUNK_RADIX - MAX_CHUNKS_S/2;
    int bottom_corner_z = pos.z/CHUNK_RADIX - MAX_CHUNKS_S/2;
    for (int x = 0; x < MAX_CHUNKS_S; x++) {
        for (int y = 0; y < MAX_CHUNKS_S; y++) {
            for (int z = 0; z < MAX_CHUNKS_S; z++) {
                init_chunk_slot(cm, x, y, z, x + bottom_corner_x, y + bottom_corner_y, z + bottom_corner_z);
            }
        }
    }
}

void test_wtbc(int x, int y, int z) {
    vec3i chunk_coords;
    vec3i block_coords;
    world_to_block_and_chunk(&chunk_coords, &block_coords, (vec3l) {x,y,z});
    printf("%d %d %d lands in chunk %d %d %d and block %d %d %d\n",
        x, y, z, chunk_coords.x, chunk_coords.y, chunk_coords.z,
        block_coords.x, block_coords.y, block_coords.z);
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


    for (int i = 0; i < MAX_CHUNKS_SSS; i++) {
        chunk *cpi = &cm->chunk_slots[i].c;
        if (cpi->empty) {
            return (block) {BLOCK_AIR};
        }

        if (cpi->x == chunk_coords.x && cpi->y == chunk_coords.y && cpi->z == chunk_coords.z) {
            // its this chunk
            return cpi->blocks->blocks[block_coords.x][block_coords.y][block_coords.z];
        }
    }

    // didnt find
    printf("didnt find %ld %ld %ld\n", pos.x, pos.y, pos.z);
    return (block) {
        .tag = BLOCK_AIR, // todo make this separate to air so player dosnt fall lol
    };
}

void set_block(chunk_manager *cm, vec3l pos, block b) {    
    vec3i chunk_coords;
    vec3i block_coords;
    world_to_block_and_chunk(&chunk_coords, &block_coords, pos);


    for (int i = 0; i < MAX_CHUNKS_SSS; i++) {
        chunk *cpi = &cm->chunk_slots[i].c;

        if (cpi->x == chunk_coords.x && cpi->y == chunk_coords.y && cpi->z == chunk_coords.z) {
            // its this chunk
            if (cpi->empty) {
                printf("empty\n");
                // not empty and allocate memory
                cpi->empty = false;
                cpi->blocks = calloc(sizeof(block), CHUNK_RADIX*CHUNK_RADIX*CHUNK_RADIX);
            }

            cpi->blocks->blocks[block_coords.x][block_coords.y][block_coords.z] = b;
            mesh_chunk(cpi, cm->chunk_slots[i].vao, cm->chunk_slots[i].vbo);
            return;
        }
    }


    // didnt find
    printf("didnt find %ld %ld %ld\n", pos.x, pos.y, pos.z);
}



/*
float intbound(float s, float ds) { 
    return (ds > 0? ceil(s)-s: s-floor(s)) / abs(ds); 
}
*/

pick_info pick_block(chunk_manager *world, vec3s pos, vec3s facing, float max_distance) {
/*
    printf("%.2f\n", intbound(-1.5, 1));
    printf("%.2f\n", intbound(-1.6, 1));
    printf("%.2f\n", intbound(-1.6, 0.5));

    printf("%.2f\n", intbound(1.5, -1));
    printf("%.2f\n", intbound(1.6, -1));
    printf("%.2f\n", intbound(1.6, -0.5));

    printf("%.2f\n", intbound(1.5, 1));
    printf("%.2f\n", intbound(1.6, 1));
    printf("%.2f\n", intbound(1.6, 0.5));
    */

    printf("facing %.2f, %.2f, %.2f\n", facing.x, facing.y, facing.z);

    pick_info ret = {0};
    ret.success = true;

    ret.coords = nearest_block_pos(pos);

    int sx = signum(facing.x);
    int sy = signum(facing.y);
    int sz = signum(facing.z);

    float tMaxX = intbound(pos.x, facing.x);
    float tMaxY = intbound(pos.y, facing.y);
    float tMaxZ = intbound(pos.z, facing.z);

    //printf("initial tmx: %.2f, tmy: %.2f, tmz: %.2f\n", tMaxX, tMaxY, tMaxZ);
    printf("sx %d sy %d sz %d\n", sx, sy, sz);

    float accX = 0;
    float accY = 0;
    float accZ = 0;

    float tDeltaX = (float)sx / facing.x;
    float tDeltaY = (float)sy / facing.y;
    float tDeltaZ = (float)sz / facing.z;

    float max_squared = max_distance*max_distance;

    // error on direction 0,0,0

    int n = 0;
    // accX*accX + accY*accY + accZ*accZ <= max_squared
    while (accX*accX + accY*accY + accZ*accZ <= max_squared) {
        n++;
        block_tag t = get_block(world, ret.coords).tag;
        printf("x: %ld y: %ld z: %ld, t: %d\n", ret.coords.x, ret.coords.y, ret.coords.z, t);
        //printf("tmx: %.2f, tmy: %.2f, tmz: %.2f\n", tMaxX, tMaxY, tMaxZ);
        if (t != BLOCK_AIR) {
            printf("not air\n");
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
                accY += tMaxY;
                tMaxY += tDeltaY;
                ret.normal_x = 0;
                ret.normal_y = -sy;
                ret.normal_z = 0;
            } else {
                // Z min (again)
                ret.coords.z += sz;
                accZ += tMaxZ;
                tMaxZ += tDeltaZ;
                ret.normal_x = 0;
                ret.normal_y = 0;
                ret.normal_z = -sz;
            }
        }

    }
    ret.success = false;
    //printf("didnt find anything after n iters %d\n", n);
    //printf("bailed with accx: %.2f, accy: %.2f, accz: %.2f\n", accX, accY, accZ);

    return ret;
    
}