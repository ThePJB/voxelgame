#include "chunk_common.h"

#define VERT_STRIDE 8

#define MESHING_BUF_SIZE 409600 * 5
void cm_mesh_chunk(chunk_manager *cm, int x, int y, int z) {
    printf("meshing chunk %d %d %d\n", x, y, z);

    int chunk_idx = hmgeti(cm->chunk_hm, ((vec3i){x,y,z}));
    chunk *c = &cm->chunk_hm[chunk_idx];
    c->needs_remesh = false;
    float buf[MESHING_BUF_SIZE] = {0};

    if (c->empty) {
        return;
    }
    int vertex_idx = 0;
    const float cube_verts[] = 
        #include "cube.h"

    for (int idx = 0; idx < CHUNK_RADIX_3; idx++) {
        if (c->blocks[idx] == BLOCK_AIR) {
            continue;
        }

        vec3i coords = chunk_1d_to_3d(idx);

        for (direction face = 0; face < NUM_DIRS; face++) {
            // Skip occluded faces
            if (face == DIR_PX && neighbour_exists(coords, PLUS_X) && c->blocks[idx + PLUS_X] != BLOCK_AIR) {
                continue;
            } else if (face == DIR_MX && neighbour_exists(coords, MINUS_X) && c->blocks[idx + MINUS_X] != BLOCK_AIR) {
                continue;
            } else if (face == DIR_PY && neighbour_exists(coords, PLUS_Y) && c->blocks[idx + PLUS_Y] != BLOCK_AIR) {
                continue;
            } else if (face == DIR_MY && neighbour_exists(coords, MINUS_Y) && c->blocks[idx + MINUS_Y] != BLOCK_AIR) {
                continue;
            } else if (face == DIR_PZ && neighbour_exists(coords, PLUS_Z) && c->blocks[idx + PLUS_Z] != BLOCK_AIR) {
                continue;
            } else if (face == DIR_MZ && neighbour_exists(coords, MINUS_Z) && c->blocks[idx + MINUS_Z] != BLOCK_AIR) {
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
                buf[vertex_idx++] = c->blocks[idx] - 1.0; // block type 

                int light_max = 16;

                vec3i block_pos = chunk_1d_to_3d(idx);
                vec3l pos = {
                    CHUNK_RADIX * x + block_pos.x,
                    CHUNK_RADIX * y + block_pos.y,
                    CHUNK_RADIX * z + block_pos.z,
                };
                vec3l face_neighbour_pos = vec3l_add(pos, unit_vec3l[face]);
                buf[vertex_idx++] = unlerp(0, light_max, world_get_illumination(cm, face_neighbour_pos));
            }
        }
    }

    //printf("vertex idx: %d / %d\n", vertex_idx, buflen);
    c->num_triangles =  vertex_idx / (VERT_STRIDE+2) / 3;
    //printf("meshed chunk, %d triangles\n", num_triangles);

    const int num_attribs_per_vertex = 10;

    // bind vao and vertix attribs
    glBindVertexArray(c->vao);
    glBindBuffer(GL_ARRAY_BUFFER, c->vbo);
    glBufferData(GL_ARRAY_BUFFER, c->num_triangles * 3 * num_attribs_per_vertex * sizeof(float), buf, GL_STATIC_DRAW); // sizeof vertices u retard

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