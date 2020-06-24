#include "world.h"
#include "easing.h"
#include "stdlib.h"
#include "simplex.h"

struct osn_context *ctx;

void init_world_noise() {
    open_simplex_noise(123456789, &ctx);
}

void generate_chunk(chunk *c, int x, int y, int z) {

    c->x = x;
    c->y = y;
    c->z = z;

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
    //printf("generated chunk at %d %d %d\n", x, y, z);
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
    //printf("meshed chunk, %d triangles\n", c->num_triangles);

    // now just bind the vao
    // how to clean up this stuff when i want to? good question
    glGenVertexArrays(1, &c->vao);
    
    unsigned int vao = c->vao;
    unsigned int vbo;
    glGenBuffers(1, &vbo);
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
    model = glms_translate(model, (vec3s){ch->x*CHUNK_RADIX, ch->y*CHUNK_RADIX, ch->z*CHUNK_RADIX});
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
