#ifndef GRAPHICS_H
#define GRAPHICS_H

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
#include "camera.h"

typedef struct {
    double x;
    double y;
    double z;
    mat4s transform;
    unsigned int texture;
    unsigned int vao;
    float *vertex_data;
    unsigned int num_triangles;
} mesh;

typedef struct {
    // shared with window
    int *w;
    int *h;
    camera *cam;
    
    unsigned int mesh_program;
    unsigned int chunk_program;

    // test cube
    mesh cube;

    unsigned int tromp;
    unsigned int spoderman;
    unsigned int atlas;
} graphics_context;

graphics_context *graphics_init(int *w, int *h, camera *cam);

void pre_draw(graphics_context *c);

void draw_mesh(graphics_context *c, mesh m, vec3s translate, vec3s rotate_axis, float rotate_amt);
unsigned long int get_vram_usage();


#endif