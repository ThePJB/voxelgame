#include "world.h"
#include "easing.h"
#include "stdlib.h"
#include "simplex.h"

struct osn_context *ctx;

void init_world_noise(int seed) {
    open_simplex_noise(seed, &ctx);
}

void generate_chunk(chunk *c, int x, int y, int z) {

    c->x = x;
    c->y = y;
    c->z = z;

    glGenVertexArrays(1, &c->vao);
    glGenBuffers(1, &c->vbo);

    float chunk_x = x*CHUNK_RADIX;
    float chunk_y = y*CHUNK_RADIX;
    float chunk_z = z*CHUNK_RADIX;

    const float amplitude = 48;

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

                if (block_y < height - 0.5) {
                    c->blocks[i][j][k] = (block) {BLOCK_DIRT};
                } else if (block_y < height + 0.5) {
                    c->blocks[i][j][k] = (block) {BLOCK_GRASS};
                } else {
                    c->blocks[i][j][k] = (block) {BLOCK_AIR};
                }
            }
        }
    }
    printf("generated chunk at %d %d %d\n", x, y, z);
}

void generate_chunk_random(chunk *c, int x, int y, int z) {
    c->x = x;
    c->y = y;
    c->z = z;

    float chunk_x = x*CHUNK_RADIX;
    float chunk_y = y*CHUNK_RADIX;
    float chunk_z = z*CHUNK_RADIX;

    float fill0_at = 64;
    float fill1_at = -64;
    float fill_divisor = fill0_at - fill1_at;

    float filledness = 1 - (chunk_y + fill0_at)/fill_divisor;
    filledness = slow_start4(filledness);

    for (int i = 0; i < CHUNK_RADIX; i++) {
        for (int j = 0; j < CHUNK_RADIX; j++) {
            for (int k = 0; k < CHUNK_RADIX; k++) {
                if ((float)rand() / RAND_MAX < filledness) {
                        c->blocks[i][j][k] = (block) {BLOCK_GRASS};
                } else {
                        c->blocks[i][j][k] = (block) {BLOCK_AIR};
                }
            }
        }
    }
    //printf
}

float vertices[819200] = {0};

// Currently I'll push position, texture and normals
// might turn out to be better to compute some of that stuff in place than send it over
// or it might not we'll see

#define VERT_STRIDE 8

void mesh_chunk(chunk *c) {
    int vertex_idx = 0;

    const float cube_verts[] = 
    #include "cube.h"
    // for now we are gonna do the dirty and generate geometry for every single block
    // next easiest is for a face to check if its occluded
    // then follow that with greedy meshing

    for (int i = 0; i < CHUNK_RADIX; i++) {
        for (int j = 0; j < CHUNK_RADIX; j++) {
            for (int k = 0; k < CHUNK_RADIX; k++) {

                if (c->blocks[i][j][k].tag == BLOCK_AIR) {
                    continue;
                }

                for (int face = 0; face < 6; face++) {
                    
                    // faces are numbered, -+z, -+x, -+y
                    if (face == 0 && k != 0 && c->blocks[i][j][k-1].tag != BLOCK_AIR) {
                        continue;
                    }             
                    
                    if (face == 1 && k != CHUNK_RADIX-1 && c->blocks[i][j][k+1].tag != BLOCK_AIR) {
                        continue;
                    }
                    if (face == 2 && i != 0 && c->blocks[i-1][j][k].tag != BLOCK_AIR) {
                        continue;
                    }
                    if (face == 3 && i != CHUNK_RADIX-1 && c->blocks[i+1][j][k].tag != BLOCK_AIR) {
                        continue;
                    }                    
                    if (face == 4 && j != 0 && c->blocks[i][j-1][k].tag != BLOCK_AIR) {
                        continue;
                    }
                    if (face == 5 && j != CHUNK_RADIX-1 && c->blocks[i][j+1][k].tag != BLOCK_AIR) {
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
                        vertices[vertex_idx++] = c->blocks[i][j][k].tag - 1.0; // block type 
                    }
                }
            }
        }
    }

    c->num_triangles = vertex_idx / (VERT_STRIDE+1) / 3;
    printf("meshed chunk, %d triangles\n", c->num_triangles);

    // now just bind the vao
    // how to clean up this stuff when i want to? good question


    
    unsigned int vao = c->vao;
    unsigned int vbo = c->vbo;
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

}

void draw_chunk(chunk *ch, context *c) {
    mat4s model = GLMS_MAT4_IDENTITY_INIT;
    model = glms_translate(model, (vec3s){ch->x*CHUNK_RADIX + 0.5, ch->y*CHUNK_RADIX + 0.5, ch->z*CHUNK_RADIX + 0.5});
    glUniformMatrix4fv(glGetUniformLocation(c->mesh_program, "model"), 1, GL_FALSE, model.raw[0]);

    glBindVertexArray(ch->vao);
    glDrawArrays(GL_TRIANGLES, 0, ch->num_triangles * 3);
    
}

void draw_chunks(chunk_manager *cm, context *c) {
    glBindTexture(GL_TEXTURE_2D, c->atlas);
    glUseProgram(c->chunk_program);


    for (int i = 0; i < MAX_CHUNKS_SSS; i++) {
        draw_chunk(cm->chunk_pointers[i], c);
    }
}


void chunk_manager_position_hint(chunk_manager *cm, vec3s pos) {
    int bottom_corner_x = pos.x/CHUNK_RADIX - MAX_CHUNKS_S/2;
    int bottom_corner_y = pos.y/CHUNK_RADIX - MAX_CHUNKS_S/2;
    int bottom_corner_z = pos.z/CHUNK_RADIX - MAX_CHUNKS_S/2;
    for (int x = 0; x < MAX_CHUNKS_S; x++) {
        for (int y = 0; y < MAX_CHUNKS_S; y++) {
            for (int z = 0; z < MAX_CHUNKS_S; z++) {
                chunk *new_chunk = calloc(1, sizeof(chunk));
                generate_chunk(new_chunk, x + bottom_corner_x, y + bottom_corner_y, z + bottom_corner_z);
                mesh_chunk(new_chunk);
                cm->chunk_pointers[MAX_CHUNKS_SS * x + MAX_CHUNKS_S * y + z] = new_chunk;
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
        chunk *cpi = cm->chunk_pointers[i];
        if (cpi->x == chunk_coords.x && cpi->y == chunk_coords.y && cpi->z == chunk_coords.z) {
            // its this chunk
            return cpi->blocks[block_coords.x][block_coords.y][block_coords.z];
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
        chunk *cpi = cm->chunk_pointers[i];
        if (cpi->x == chunk_coords.x && cpi->y == chunk_coords.y && cpi->z == chunk_coords.z) {
            // its this chunk
            cpi->blocks[block_coords.x][block_coords.y][block_coords.z] = b;
            mesh_chunk(cpi);
            return;
        }
    }


    // didnt find
    printf("didnt find %ld %ld %ld\n", pos.x, pos.y, pos.z);
}


long int signum(float x) {
    if (x > 0) {
        return 1;
    } else {
        return -1;
    }
}

int mod(int val, int modulus) {
    // ok i think this is because its fake modulus (divisor)
    return (val % modulus + modulus) % modulus;
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

    ret.coords = (vec3l) {pos.x, pos.y, pos.z};

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
    while (n < 10) {
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