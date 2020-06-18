#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "glad.h"
#include <GLFW/glfw3.h>
#include <cglm/struct.h>
#include "shader.h"

#include "graphics.h"
#include "world.h"

int main(int argc, char** argv) {
    
    graphics_init();
    context *c = get_context();

    float last = 0;
    float dt = 0;

    chunk ch;

    generate_chunk(&ch, 0, 0, 0);
    mesh_chunk(&ch);


    while (!glfwWindowShouldClose(c->window)) {
        if (glfwGetKey(c->window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(c->window, true);
        }
        float time = glfwGetTime();
        dt = time - last;
        last = time;

        camera new_cam = update_camera(c->window, c->cam, dt);

        c->cam = new_cam;

        begin_draw(c);
        //draw_mesh(c, c->cube); 
        draw_chunk(&ch, c);
        end_draw(c);
    }

    graphics_teardown();

    return 0;
}
