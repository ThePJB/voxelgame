#ifndef WINDOW_H
#define WINDOW_H

#include "camera.h"

typedef struct {
    // shared with graphics
    int *w;
    int *h;
    camera *cam;

    GLFWwindow *window;

    double mouse_lastx;
    double mouse_lasty;

    float dt;

    bool wireframe;
    bool show_info;
    
} window_context;

window_context *window_init(char *title, int *w, int *h, camera *cam);

#endif