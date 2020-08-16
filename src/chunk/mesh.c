#include "chunk_common.h"

#define VERT_STRIDE 8

#define MESHING_BUF_SIZE 409600 * 5


typedef struct {
    float pos[3];
    
    float normal[3];

    float texu;
    float texv;

    float block_type;

    float light_block;
    float light_sky;

    float ao;
} chunk_vert;

#define CHUNK_VERTEX_ATTRS (sizeof(chunk_vert)/sizeof(float))

typedef struct {
    chunk_vert verts[4];
} chunk_quad;

const int light_max = 16;


chunk_quad emit_quad(chunk_manager *cm, chunk *c, int block_idx, direction face) {
    chunk_quad ret = {0};

    int dir_num = face / 2;
    int dir_sign = (face % 2 == 0) * 2 - 1;

    float vert_pos[3];
    vert_pos[dir_num] = 0.5 * dir_sign; // if its +x we fix x plane to +0.5 etc

    vec3i block_local_coords = chunk_1d_to_3d(block_idx);
    vec3l pos = {
        CHUNK_RADIX * c->key.x + block_local_coords.x,
        CHUNK_RADIX * c->key.y + block_local_coords.y,
        CHUNK_RADIX * c->key.z + block_local_coords.z,
    };
    vec3l face_neighbour_pos = vec3l_add(pos, unit_vec3l[face]);

    for (int i = 0; i < 4; i++) {
        int sign_m = (i == 1 || i == 2) * 2 - 1;
        int sign_n = (i == 0 || i == 1) * 2 - 1;

        vert_pos[(dir_num + 1) % 3] = sign_m * 0.5;
        vert_pos[(dir_num + 2) % 3] = sign_n * 0.5;

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

        ret.verts[i] = (chunk_vert) {
            .pos[0] = vert_pos[0] + block_local_coords.x,
            .pos[1] = vert_pos[1] + block_local_coords.y,
            .pos[2] = vert_pos[2] + block_local_coords.z,

            .normal[0] = unit_vec3s[face].raw[0],
            .normal[1] = unit_vec3s[face].raw[1],
            .normal[2] = unit_vec3s[face].raw[2],

            .block_type = c->blocks[block_idx] - 1.0,

            .light_block = unlerp(0, light_max, block_light),
            .light_sky = unlerp(0, light_max, sky_light),

            .ao = 0.5,
        };
    }

    // I honestly dont remember why this
    if (face == DIR_PX || face == DIR_MX) {
        for (int i = 0; i < 4; i++) {
            ret.verts[i].texu = 0;
            ret.verts[i].texv = 0;
        }   
    } else if (face == DIR_PY) {
        for (int i = 0; i < 4; i++) {
            ret.verts[i].texu = 1;
            ret.verts[i].texv = 0;
        }   
    } else if (face == DIR_MY) {
        for (int i = 0; i < 4; i++) {
            ret.verts[i].texu = -1;
            ret.verts[i].texv = 0;
        }   
    } else if (face == DIR_PZ) {
        for (int i = 0; i < 4; i++) {
            ret.verts[i].texu = 0;
            ret.verts[i].texv = 1;
        }   
    } else if (face == DIR_MZ) {
        for (int i = 0; i < 4; i++) {
            ret.verts[i].texu = 0;
            ret.verts[i].texv = -1;
        }   
    }

/*
    printf("dir %s, block %d\n", dir_name[face], c->blocks[block_idx]);
    
    chunk_vert v = ret.verts[0];
    printf("pos %f %f %f\n normal %f %f %f\n texu %f\n texv %f\n block %f\n light %f %f\n ao %f\n",
    v.pos[0], v.pos[1], v.pos[2], v.normal[0], v.normal[1], v.normal[2], v.texu, v.texv, v.block_type, v.light_block, v.light_sky, v.ao);
    
    printf("\n");
    v = ret.verts[1];
    printf("pos %f %f %f\n normal %f %f %f\n texu %f\n texv %f\n block %f\n light %f %f\n ao %f\n",
    v.pos[0], v.pos[1], v.pos[2], v.normal[0], v.normal[1], v.normal[2], v.texu, v.texv, v.block_type, v.light_block, v.light_sky, v.ao);
    
    printf("\n");
    v = ret.verts[2];
    printf("pos %f %f %f\n normal %f %f %f\n texu %f\n texv %f\n block %f\n light %f %f\n ao %f\n",
    v.pos[0], v.pos[1], v.pos[2], v.normal[0], v.normal[1], v.normal[2], v.texu, v.texv, v.block_type, v.light_block, v.light_sky, v.ao);

    printf("\n");
    v = ret.verts[3];
    printf("pos %f %f %f\n normal %f %f %f\n texu %f\n texv %f\n block %f\n light %f %f\n ao %f\n",
    v.pos[0], v.pos[1], v.pos[2], v.normal[0], v.normal[1], v.normal[2], v.texu, v.texv, v.block_type, v.light_block, v.light_sky, v.ao);

    exit(1);
    */

    return ret;
}

// so I want to work in terms of quads and then just have a fn that turns the quad into two triangles
// why dont i just have a get neighbour with fallback


const int direction_to_3d_offset[] = {
    PLUS_X,
    MINUS_X,
    PLUS_Y,
    MINUS_Y,
    PLUS_Z,
    MINUS_Z
};

// returns how many blocks touch this vertex basically
// unless left and right not not corner, sure
int vertex_ao(bool left, bool corner, bool right) {
    if (left && right) {
        return 0;
    }
    return 3 - (left + corner + right);
}

int mesh_push_vertex(int vertex_idx, float *vertex_buffer, chunk_vert data) {
    for (int i = 0; i < CHUNK_VERTEX_ATTRS; i++) {
        vertex_buffer[vertex_idx + i] = ((float*)(&data))[i];
    }
    return vertex_idx + CHUNK_VERTEX_ATTRS;
}

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

            chunk_quad q = emit_quad(cm, c, idx, face);

            // push quad to vertex array
            // 0, 1, 3, 1, 2, 3 access pattern im pretty sure
            vertex_idx = mesh_push_vertex(vertex_idx, buf, q.verts[0]);
            vertex_idx = mesh_push_vertex(vertex_idx, buf, q.verts[1]);
            vertex_idx = mesh_push_vertex(vertex_idx, buf, q.verts[3]);
            vertex_idx = mesh_push_vertex(vertex_idx, buf, q.verts[1]);
            vertex_idx = mesh_push_vertex(vertex_idx, buf, q.verts[2]);
            vertex_idx = mesh_push_vertex(vertex_idx, buf, q.verts[3]);

        }
    }

    //printf("vertex idx: %d / %d\n", vertex_idx, buflen);
    c->num_triangles =  vertex_idx / (VERT_STRIDE+2) / 3;
    //printf("meshed chunk, %d triangles\n", num_triangles);


    // bind vao and vertix attribs
    glBindVertexArray(c->vao);
    glBindBuffer(GL_ARRAY_BUFFER, c->vbo);
    glBufferData(GL_ARRAY_BUFFER, c->num_triangles * 3 * CHUNK_VERTEX_ATTRS * sizeof(float), buf, GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, CHUNK_VERTEX_ATTRS * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texture coords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, CHUNK_VERTEX_ATTRS * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // normals
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, CHUNK_VERTEX_ATTRS * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // block type
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, CHUNK_VERTEX_ATTRS * sizeof(float), (void*)(8 * sizeof(float)));
    glEnableVertexAttribArray(3);

    // light level block
    glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, CHUNK_VERTEX_ATTRS * sizeof(float), (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(4);

    // light level sky
    glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, CHUNK_VERTEX_ATTRS * sizeof(float), (void*)(10 * sizeof(float)));
    glEnableVertexAttribArray(5);

    // ambient occlusion level
    glVertexAttribPointer(6, 1, GL_FLOAT, GL_FALSE, CHUNK_VERTEX_ATTRS * sizeof(float), (void*)(11 * sizeof(float)));
    glEnableVertexAttribArray(6);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}