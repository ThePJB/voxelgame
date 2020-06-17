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
    mat4s transform;
    unsigned int texture;
    unsigned int vao;
    float *vertex_data;
    unsigned int num_triangles;
} mesh;

typedef struct {
    int w;
    int h;
    GLFWwindow *window;
    camera cam;
    
    double mouse_lastx;
    double mouse_lasty;

    bool wireframe;

    mesh cube;
    unsigned int mesh_program;
} context;

context c;

void graphics_init();

void graphics_teardown();

void draw(context c);


#endif