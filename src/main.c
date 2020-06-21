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
#include "text.h"

int main(int argc, char** argv) {
    
    graphics_init();
    
    context *c = get_context();
    

    float last = 0;
    float dt = 0;

    chunk_manager cm = {0};
    chunk_manager_position_hint(&cm, (vec3s){0,0,0});
    


    /*
    ch.blocks[8][8][8] = (block){.tag = BLOCK_DIRT};
    ch.blocks[8][9][8] = (block){.tag = BLOCK_DIRT};
    ch.blocks[8][7][8] = (block){.tag = BLOCK_DIRT};
    ch.blocks[7][8][8] = (block){.tag = BLOCK_DIRT};
    ch.blocks[8][8][8] = (block){.tag = BLOCK_DIRT};
    */
    //mesh_chunk(&ch);

    text_init(c);

    c->cam.pos = (vec3s) {64, 64, 64};
    c->cam.front = (vec3s) {0, 0, -1};


    while (!glfwWindowShouldClose(c->window)) {
        if (glfwGetKey(c->window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(c->window, true);
        }
        float time = glfwGetTime();
        dt = time - last;
        c->dt = dt;
        last = time;

        camera new_cam = update_camera(c->window, c->cam, dt);

        c->cam = new_cam;

        begin_draw(c);
        //draw_mesh(c, c->cube); 
        //draw_chunk(&ch, c);
        draw_chunks(&cm, c);



        if (c->show_info) {
            char buf[64] = {0};
            const text_style debug_text = (text_style) {
                .scale = 1,
                .colour = (vec3s) {1,1,1},
            };
            int y = 10;
            sprintf(buf, "%.2f fps", 1.0 / c->dt);
            draw_text(buf, 10, y, debug_text);
            y += 100;

            sprintf(buf, "Facing {%.2f, %.2f, %.2f}", c->cam.front.x, c->cam.front.y, c->cam.front.z);
            draw_text(buf, 10, y, debug_text);
            y += 100;

            sprintf(buf, "Pos {%.2f, %.2f, %.2f}", c->cam.pos.x, c->cam.pos.y, c->cam.pos.z);
            draw_text(buf, 10, y, debug_text);
            y += 100;
        }

        end_draw(c);
    }

    graphics_teardown();

    return 0;
}
