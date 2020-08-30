#include "chunk_common.h"

#define VERT_STRIDE 8

#define MESHING_BUF_SIZE 409600 * 5

const int direction_to_3d_offset[] = {
    PLUS_X,
    MINUS_X,
    PLUS_Y,
    MINUS_Y,
    PLUS_Z,
    MINUS_Z
};

void cm_mesh_chunk(chunk_manager *cm, int x, int y, int z) {
    //printf("meshing chunk %d %d %d\n", x, y, z);

    int chunk_idx = hmgeti(cm->chunk_hm, ((vec3i){x,y,z}));
    chunk *c = &cm->chunk_hm[chunk_idx];
    c->needs_remesh = false;
    float buf[MESHING_BUF_SIZE] = {0};
    float water_buf[MESHING_BUF_SIZE] = {0};

    int vertex_idx = 0;
    const float cube_verts[] = 
        #include "cube.h"

    for (int idx = 0; idx < CHUNK_RADIX_3; idx++) {
        if (c->blocks[idx] == BLOCK_AIR) {
            continue;
        }

        vec3i block_local_coords = chunk_1d_to_3d(idx);

        for (direction face = 0; face < NUM_DIRS; face++) {
            // Skip occluded faces
            int neighbour_idx = idx + direction_to_3d_offset[face];
            if (neighbour_exists(block_local_coords, direction_to_3d_offset[face])) {
                block_definition this_properties = block_defs[c->blocks[idx]];
                block_definition that_properties = block_defs[c->blocks[neighbour_idx]];

                if (c->blocks[idx] == c->blocks[neighbour_idx]) {
                    continue;
                } else if (this_properties.opaque && that_properties.opaque) {
                    continue;
                }

            } 
            
            
            for (int cube_vert = 0; cube_vert < 6; cube_vert++) {

                // walk the cube verts
                const float *current_vert = &(cube_verts[face * 6 * VERT_STRIDE + cube_vert * VERT_STRIDE]);
                
                // push to the vertex buffer
                buf[vertex_idx++] = current_vert[0] + block_local_coords.x; // x pos
                buf[vertex_idx++] = current_vert[1] + block_local_coords.y; // y pos 
                buf[vertex_idx++] = current_vert[2] + block_local_coords.z; // z pos
                buf[vertex_idx++] = current_vert[3]; // normal x
                buf[vertex_idx++] = current_vert[4]; // normal y
                buf[vertex_idx++] = current_vert[5]; // normal z
                buf[vertex_idx++] = current_vert[6]; // normal u
                buf[vertex_idx++] = current_vert[7]; // normal 
                buf[vertex_idx++] = c->blocks[idx] - 1.0; // block type 

                float ao = 0.5;
                buf[vertex_idx++] = ao;

                int light_max = 16;

                vec3i block_pos = chunk_1d_to_3d(idx);
                vec3l pos = {
                    CHUNK_RADIX * x + block_pos.x,
                    CHUNK_RADIX * y + block_pos.y,
                    CHUNK_RADIX * z + block_pos.z,
                };
                vec3l face_neighbour_pos = vec3l_add(pos, unit_vec3l[face]);
                uint8_t block_light = 0;
                uint8_t sky_light = SKY_LIGHT_FULL;
                maybe_uint8_t mblock_light = light_get_block(cm, face_neighbour_pos);
                if (mblock_light.ok) {
                    block_light = mblock_light.value;
                }
                maybe_uint8_t msky_light = light_get_sky(cm, face_neighbour_pos);
                if (msky_light.ok) {
                    sky_light = msky_light.value;
                }
                buf[vertex_idx++] = unlerp(0, light_max, block_light);
                buf[vertex_idx++] = unlerp(0, light_max, sky_light);
            }
        }
    }

    //printf("vertex idx: %d / %d\n", vertex_idx, buflen);
    c->num_triangles =  vertex_idx / (VERT_STRIDE+3) / 3;
    //printf("meshed chunk, %d triangles\n", num_triangles);

    const int num_attribs_per_vertex = 12;

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

    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, num_attribs_per_vertex * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(5);    
    
    glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, num_attribs_per_vertex * sizeof(float), (void*)(11 * sizeof(float)));
    glEnableVertexAttribArray(6);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}