#include "chunk_common.h"

float *push_vertex(float *buf, vec3s v, vec3s n, noise2d_params p) {

    vec3s c;

    if (v.y > p.snow_above_height) {
        c = (vec3s) {0.9, 0.9, 0.9};
    } else if (v.y <= p.water_below_height) {
        c = (vec3s) {0.2, 0.4, 1};
    } else if (v.y < p.sand_below_height) {
        c = (vec3s) {0.7, 0.7, 0.1};
    } else {
        // grass
        c = (vec3s) {0.4, 0.8, 0.5};
    }

    arrpush(buf, v.x);
    arrpush(buf, v.y);
    arrpush(buf, v.z);

    arrpush(buf, c.x);
    arrpush(buf, c.y);
    arrpush(buf, c.z);

    arrpush(buf, n.x);
    arrpush(buf, n.y);
    arrpush(buf, n.z);

    return buf;
}

vec3s calc_normal(vec3s v1, vec3s v2, vec3s v3) {
    vec3s a = glms_vec3_sub(v3, v2);
    vec3s b = glms_vec3_sub(v1, v2);
    vec3s n = glms_normalize(glms_vec3_cross(a,b));

    return n;
}

/*
Generates a low def mesh for a chunk by sampling the height function n+1 points a side
Equivalently, makes N^2 quads
*/
lodmesh lodmesh_generate(struct osn_context *osn, noise2d_params p, int s, int cx, int cz) {
    float *buf = 0;

    // figure out x and z of each coordinate
    float ox = LODMESH_CHUNK_RADIX * cx;
    float oz = LODMESH_CHUNK_RADIX * cz;


    for (int x = 0; x < s; x++) {
        for (int z = 0; z < s; z++) {

            vec3s v1 = {0};
            vec3s v2 = {0};
            vec3s v3 = {0};
            
            vec3s v1c = {0};
            vec3s v2c = {0};
            vec3s v3c = {0};

            // lower triangle
            v1.x = ox + ((float)x*LODMESH_CHUNK_RADIX) / s;
            v1.z = oz + ((float)z*LODMESH_CHUNK_RADIX) / s;
            v1.y = generate_height(osn, v1.x, v1.z, p);

            v2.x = ox + ((float)(x+1)*LODMESH_CHUNK_RADIX) / s;
            v2.z = oz + (float)(z*LODMESH_CHUNK_RADIX) / s;
            v2.y = generate_height(osn, v2.x, v2.z, p);

            v3.x = ox + ((float)x*LODMESH_CHUNK_RADIX) / s;
            v3.z = oz + ((float)(z+1)*LODMESH_CHUNK_RADIX) / s;
            v3.y = generate_height(osn, v3.x, v3.z, p);

            v1.y = max(v1.y, p.water_below_height);
            v2.y = max(v2.y, p.water_below_height);
            v3.y = max(v3.y, p.water_below_height);

            vec3s n = calc_normal(v2, v1, v3);
            buf = push_vertex(buf, v2, n, p);
            buf = push_vertex(buf, v1, n, p);
            buf = push_vertex(buf, v3, n, p);
            
            // upper triangle
            v1.x = ox + ((float)(x+1)*LODMESH_CHUNK_RADIX) / s;
            v1.z = oz + ((float)(z+1)*LODMESH_CHUNK_RADIX) / s;
            v1.y = generate_height(osn, v1.x, v1.z, p);

/*
            v2.x = ox + ((float)(x+1)*LODMESH_CHUNK_RADIX) / s;
            v2.z = oz + (float)(z*LODMESH_CHUNK_RADIX) / s;
            v2.y = generate_height(osn, v2.x, v2.z, p);

            v3.x = ox + ((float)x*LODMESH_CHUNK_RADIX) / s;
            v3.z = oz + ((float)(z+1)*LODMESH_CHUNK_RADIX) / s;
            v3.y = generate_height(osn, v3.x, v3.z, p);
*/
            v1.y = max(v1.y, p.water_below_height);
//            v2.y = max(v2.y, p.water_below_height);
//            v3.y = max(v3.y, p.water_below_height);

            n = calc_normal(v1, v2, v3);
            buf = push_vertex(buf, v1, n, p);
            buf = push_vertex(buf, v2, n, p);
            buf = push_vertex(buf, v3, n, p);

        }
    }

    lodmesh m = {0};
    m.key = (int32_t_pair) {cx,cz};

    const int attribs_per_vertex = 9;

    const int floats_per_triangle = attribs_per_vertex*3;
    m.num_triangles = arrlen(buf) / floats_per_triangle;

    // upload to gpu
    glGenVertexArrays(1, &m.vao);
    glGenBuffers(1, &m.vbo);

    glBindVertexArray(m.vao);
    glBindBuffer(GL_ARRAY_BUFFER, m.vbo);
    glBufferData(GL_ARRAY_BUFFER, arrlen(buf) * sizeof(float), buf, GL_STATIC_DRAW);

    const float stride = attribs_per_vertex * sizeof(float);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // colour
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(2);

    // normal
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    arrfree(buf);

    return m;
}

void lodmesh_draw(lodmesh m, graphics_context *ctx) {
    glUseProgram(ctx->lodmesh_program);
    glBindVertexArray(m.vao);
    glDrawArrays(GL_TRIANGLES, 0, m.num_triangles * 3);
}

void lodmesh_delete(chunk_manager *cm, int cx, int cz) {
    int32_t_pair key = {cx, cz};
    lodmesh m = hmgets(cm->lodmesh_hm, key);

    glDeleteBuffers(1, &m.vbo);
    glDeleteVertexArrays(1, &m.vao);
    hmdel(cm->lodmesh_hm, key);
}