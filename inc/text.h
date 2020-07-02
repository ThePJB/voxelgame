#ifndef TEXT_H
#define TEXT_H

#include "graphics.h"
#include <cglm/struct.h>


typedef struct {
    unsigned int texture;
    int size_x;
    int size_y;
    int bearing_x;
    int bearing_y;
    unsigned int advance;
} ft_char;

typedef struct {
    float scale;
    vec3s colour;
} text_style;
// maybe font

void text_init(graphics_context *c);
void draw_text(const char* text, float x, float y, text_style style);

#endif