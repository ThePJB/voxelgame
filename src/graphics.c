#include "graphics.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "glad.h"
#include <GLFW/glfw3.h>
#include <cglm/struct.h>
#include "shader.h"
#include "texture.h"
#include "world.h"
#include "util.h"

graphics_context gc = {0};

graphics_context *graphics_init(int *w, int *h, camera *cam) {
    gc.w = w;
    gc.h = h;
    gc.cam = cam;

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("failed to initialize GLAD\n");
        exit(1);
    }

    
    glViewport(0,0,*w,*h);
    glEnable(GL_DEPTH_TEST);

    gc.mesh_program = make_shader_program("shaders/vertex.glsl", "shaders/fragment.glsl");
    gc.chunk_program = make_shader_program("shaders/chunk.vert", "shaders/chunk.frag");

    // make cube
   float vertices[] =
   #include "cube.h"

    unsigned int vao;
    glGenVertexArrays(1, &vao);

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

    gc.cube.vertex_data = vertices;
    gc.cube.vao = vao;
    gc.cube.texture = load_texture("assets/tromp.jpg");
    gc.cube.num_triangles = 12;


    // load textures
    gc.tromp = load_texture("assets/tromp.jpg");
    gc.spoderman = load_texture("assets/spoderman.jpg");
    gc.atlas = load_texture("assets/atlas.png");

    return &gc;
}

void pre_draw(graphics_context *gc) {
    glClearColor(0.3, 0.5, 0.7, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mat4s view = GLMS_MAT4_IDENTITY_INIT;
    view = glms_lookat(gc->cam->pos, glms_vec3_add(gc->cam->pos, gc->cam->front), gc->cam->up);

    mat4s projection = GLMS_MAT4_IDENTITY_INIT;
    projection = glms_perspective(glm_rad(gc->cam->fovx), (float)*gc->w / *gc->h, 0.1, 1000);

    vec3s light = glms_vec3_normalize((vec3s){1,2,1});

    // send shared uniforms
    glUseProgram(gc->mesh_program);
    glUniformMatrix4fv(glGetUniformLocation(gc->mesh_program, "view"), 1, GL_FALSE, view.raw[0]);
    glUniformMatrix4fv(glGetUniformLocation(gc->mesh_program, "projection"), 1, GL_FALSE, projection.raw[0]);
    glUniform3fv(glGetUniformLocation(gc->mesh_program, "light"), 1, light.raw);

    // send shared uniforms
    glUseProgram(gc->chunk_program);
    glUniformMatrix4fv(glGetUniformLocation(gc->chunk_program, "view"), 1, GL_FALSE, view.raw[0]);
    glUniformMatrix4fv(glGetUniformLocation(gc->chunk_program, "projection"), 1, GL_FALSE, projection.raw[0]);
    glUniform3fv(glGetUniformLocation(gc->chunk_program, "light"), 1, light.raw);
}

void draw_mesh(graphics_context *gc, mesh m, vec3s translate, vec3s rotate_axis, float rotate_amt) {
    glUseProgram(gc->mesh_program);

    mat4s model = GLMS_MAT4_IDENTITY_INIT;
    model = glms_translate(model, translate);
    model = glms_rotate(model, rotate_amt, rotate_axis);
    m.transform = model;

    // upload mesh transform
    glUniformMatrix4fv(glGetUniformLocation(gc->mesh_program, "model"), 1, GL_FALSE, m.transform.raw[0]);
    glBindTexture(GL_TEXTURE_2D, m.texture);
    glBindVertexArray(m.vao);
    glDrawArrays(GL_TRIANGLES, 0, m.num_triangles * 3);
}

#define GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX 0x9048
#define GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX 0x9049

// nvidia smi better breakdown
// this is total vram usage not just ur app
unsigned long int get_vram_usage() {
    GLint total_mem_kb = 0;
    glGetIntegerv(GL_GPU_MEM_INFO_TOTAL_AVAILABLE_MEM_NVX, 
                &total_mem_kb);

    GLint cur_avail_mem_kb = 0;
    glGetIntegerv(GL_GPU_MEM_INFO_CURRENT_AVAILABLE_MEM_NVX, 
                &cur_avail_mem_kb);

    return total_mem_kb - cur_avail_mem_kb;                
}

