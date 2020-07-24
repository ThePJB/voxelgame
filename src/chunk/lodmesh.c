#include "chunk_common.h"


// its a fair abstraction to think about 1 triangle at a time
float *push_triangle(float *buf, vec3s v1, vec3s v2, vec3s v3, vec3s v1c, vec3s v2c, vec3s v3c) {
    vec3s a = glms_vec3_sub(v3, v2);
    vec3s b = glms_vec3_sub(v1, v2);
    vec3s n = glms_normalize(glms_vec3_cross(a,b));

    // push vertices
    arrpush(buf, v1.x);
    arrpush(buf, v1.y);
    arrpush(buf, v1.z);
    arrpush(buf, n.x);
    arrpush(buf, n.y);
    arrpush(buf, n.z);
    arrpush(buf, v1c.x);
    arrpush(buf, v1c.y);
    arrpush(buf, v1c.z);

    arrpush(buf, v2.x);
    arrpush(buf, v2.y);
    arrpush(buf, v2.z);
    arrpush(buf, n.x);
    arrpush(buf, n.y);
    arrpush(buf, n.z);
    arrpush(buf, v2c.x);
    arrpush(buf, v2c.y);
    arrpush(buf, v2c.z);

    arrpush(buf, v3.x);
    arrpush(buf, v3.y);
    arrpush(buf, v3.z);
    arrpush(buf, n.x);
    arrpush(buf, n.y);
    arrpush(buf, n.z);
    arrpush(buf, v3c.x);
    arrpush(buf, v3c.y);
    arrpush(buf, v3c.z);

    return buf;
}


/*
Generates a low def mesh for a chunk by sampling the height function n+1 points a side
Equivalently, makes N^2 quads
*/
lodmesh lodmesh_generate(struct osn_context *osn, noise2d_params p, int n, int cx, int cz) {
    float *buf = 0;

    // figure out x and z of each coordinate
    float ox = CHUNK_RADIX * cx;
    float oz = CHUNK_RADIX * cz;


    for (int x = 0; x < n; x++) {
        for (int z = 0; z < n; z++) {
            vec3s colour = {0.3, 0.9, 0.4};



            vec3s v1 = {0};
            vec3s v2 = {0};
            vec3s v3 = {0};

            // lower triangle
            v2.x = ox + ((float)(x+1)*CHUNK_RADIX) / n;
            v2.z = oz + (float)(z*CHUNK_RADIX) / n;
            v2.y = generate_height(osn, v2.x, v2.z, p);

            v1.x = ox + ((float)x*CHUNK_RADIX) / n;
            v1.z = oz + ((float)z*CHUNK_RADIX) / n;
            v1.y = generate_height(osn, v1.x, v1.z, p);

            v3.x = ox + ((float)x*CHUNK_RADIX) / n;
            v3.z = oz + ((float)(z+1)*CHUNK_RADIX) / n;
            v3.y = generate_height(osn, v3.x, v3.z, p);

            buf = push_triangle(buf, v2, v1, v3, colour, colour, colour);

            // upper triangle
            v2.x = ox + ((float)x+1)*CHUNK_RADIX / n;
            v2.z = oz + ((float)z)*CHUNK_RADIX / n;
            v2.y = generate_height(osn, v2.x, v2.z, p);

            v1.x = ox + ((float)x + 1)*CHUNK_RADIX / n;
            v1.z = oz + ((float)z + 1)*CHUNK_RADIX / n;
            v1.y = generate_height(osn, v1.x, v1.z, p);

            v3.x = ox + ((float)x)*CHUNK_RADIX / n;
            v3.z = oz + ((float)z+1)*CHUNK_RADIX / n;
            v3.y = generate_height(osn, v3.x, v3.z, p);

            buf = push_triangle(buf, v1, v2, v3, colour, colour, colour);
        }
    }

    lodmesh m = {0};
    m.data = buf;
    m.key = (int32_t_pair) {cx,cz};
    m.should_draw = true;

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

    // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    // colour
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    //printf("lodmesh gen end %d\n", m.num_triangles);

    return m;
}

// probably should have a custom mesh type, custom shader
// colour and normals? for appearance i anticipate making it kinda green and doing facing ratio
// for starters anyway

// maybe some uniforms as arguments or something
void lodmesh_draw(lodmesh m, graphics_context *ctx) {
    glUseProgram(ctx->lodmesh_program);
    mat4s model = GLMS_MAT4_IDENTITY_INIT;

    glUniformMatrix4fv(glGetUniformLocation(ctx->lodmesh_program, "model"), 1, GL_FALSE, model.raw[0]);
    glBindVertexArray(m.vao);
    glDrawArrays(GL_TRIANGLES, 0, m.num_triangles * 3);

}

void lodmesh_delete(chunk_manager *cm, int cx, int cz) {

}


// lodmesh_delete