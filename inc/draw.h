#ifndef DRAW_H
#define DRAW_H

#include "graphics.h"
#include "chunk_common.h"

// The difference between this and graphics is that this is allowed to
// depend on game logic stuff whereas graphics is strictly a leaf node

typedef struct {
    
    unsigned int sun_program;
    unsigned int sun_vao;
    unsigned int sun_vbo;

    unsigned int skybox_program;
    unsigned int skybox_vao;
    unsigned int skybox_vbo;

} draw_context;

draw_context *draw_init();
void draw(draw_context *dc, graphics_context *gc, window_context *wc, chunk_manager *cm, float solar_angle);

#endif