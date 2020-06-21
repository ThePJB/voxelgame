#include "world.h"
#include "stdlib.h"

void generate_chunk(chunk *c, int x, int y, int z) {
    c->x = x;
    c->y = y;
    c->z = z;

    const float filledness = 0.2;
    for (int i = 0; i < CHUNK_RADIX; i++) {
        for (int j = 0; j < CHUNK_RADIX; j++) {
            for (int k = 0; k < CHUNK_RADIX; k++) {
                if (j < 8) {
                    c->blocks[i][j][k] = (block) {BLOCK_DIRT};
                } else {
                    if ((float)rand() / RAND_MAX < filledness) {
                        c->blocks[i][j][k] = (block) {BLOCK_GRASS};
                    } else {
                        c->blocks[i][j][k] = (block) {BLOCK_AIR};
                    }
                }
            }
        }
    }
    printf("generated chunk at %d %d %d\n", x, y, z);
}


float vertices[819200] = {0};
int vertex_idx = 0;

// Currently I'll push position, texture and normals
// might turn out to be better to compute some of that stuff in place than send it over
// or it might not we'll see

#define VERT_STRIDE 8

void mesh_chunk(chunk *c) {
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
                    }
                }
            }
        }
    }

    c->num_triangles = vertex_idx / VERT_STRIDE / 3;
    printf("meshed chunk, %d triangles\n", c->num_triangles);

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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texture coords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // normals
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}

void draw_chunk(chunk *ch, context *c) {
    glUseProgram(c->mesh_program);
    mat4s model = GLMS_MAT4_IDENTITY_INIT;
    model = glms_translate(model, (vec3s){ch->x, ch->y, ch->z});
    glUniformMatrix4fv(glGetUniformLocation(c->mesh_program, "model"), 1, GL_FALSE, model.raw[0]);

    glBindTexture(GL_TEXTURE_2D, c->spoderman);
    glBindVertexArray(ch->vao);
    glDrawArrays(GL_TRIANGLES, 0, ch->num_triangles * 3);
    
}