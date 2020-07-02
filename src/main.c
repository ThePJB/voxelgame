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
#include "window.h"

chunk_manager cm = {0};

void draw_lookat_cube(chunk_manager *cm, vec3s cam_pos, vec3s cam_front, graphics_context *c) {
    pick_info p = pick_block(cm, cam_pos, cam_front, 9);
    if (p.success) {
        glDepthFunc(GL_LEQUAL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        draw_mesh(c,  c->cube, (vec3s) {p.coords.x + 0.5, p.coords.y + 0.5, p.coords.z + 0.5}, (vec3s){0}, 0);

        glDepthFunc(GL_LESS);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    }
}

bool enable_debug = false;

int main(int argc, char** argv) {
    int w = 2560;
    int h = 1440;
    camera cam = fly_camera();
    //test_chunk();
    //test_world();
    //exit(0);

    window_context *wc = window_init("sick game", &w, &h, &cam);
    graphics_context *gc = graphics_init(&w, &h, &cam);

    text_init(gc);
    init_chunk_manager(&cm, 123456789);
    chunk_manager_position_hint(&cm, (vec3s){0,0,0}); // generates and meshes chunks

    cam.pos = (vec3s) {0, 16, 0};
    cam.front = (vec3s) {0, 0, -1};

    float last = 0;
    float dt = 0;
    
    while (!glfwWindowShouldClose(wc->window)) {
        if (glfwGetKey(wc->window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(wc->window, true);
        }
        float time = glfwGetTime();
        dt = time - last;
        wc->dt = dt;
        last = time;

        cam = update_camera(wc->window, cam, dt);

        pre_draw(gc);
        
        draw_chunks(&cm, gc);
        
        draw_lookat_cube(&cm, cam.pos, cam.front, gc);
        //draw_mesh(c, c->cube); 

        if (wc->show_info) {
            char buf[64] = {0};
            const text_style debug_text = (text_style) {
                .scale = 1,
                .colour = (vec3s) {1,1,1},
            };
            int y = 10;
            sprintf(buf, "%.2f fps", 1.0 / wc->dt);
            draw_text(buf, 10, y, debug_text);
            y += 100;

            sprintf(buf, "Facing {%.2f, %.2f, %.2f}", cam.front.x, cam.front.y, cam.front.z);
            draw_text(buf, 10, y, debug_text);
            y += 100;

            sprintf(buf, "Pos {%.2f, %.2f, %.2f}", cam.pos.x, cam.pos.y, cam.pos.z);
            draw_text(buf, 10, y, debug_text);
            y += 100;            
            
            sprintf(buf, "RAM: %lu MB", get_ram_usage()/ (1024*1024));
            draw_text(buf, 10, y, debug_text);
            y += 100;        

            sprintf(buf, "VRAM: %lu MB (remember this is total)", get_vram_usage()/ (1024));
            draw_text(buf, 10, y, debug_text);
            y += 100;

            // block coords
            vec3l bc = (vec3l) {cam.pos.x, cam.pos.y, cam.pos.z};

            vec3i block_coords = {0};
            vec3i chunk_coords = {0};
    
            world_to_block_and_chunk(&chunk_coords, &block_coords, bc);
            chunk_slot *cs = get_chunk_slot(&cm, chunk_coords);
            if (cs) {
                chunk *c = &cs->chunk;
                sprintf(buf, "In chunk %d %d %d, empty: %d, block coords %d %d %d", 
                    chunk_coords.x, chunk_coords.y, chunk_coords.z,
                    c->empty,
                    block_coords.x, block_coords.y, block_coords.z);
            } else {
                sprintf(buf, "not in chunk");
            }

            draw_text(buf, 10, y, debug_text);
            y += 100;

            sprintf(buf, "Block ur in: %d", get_block(&cm, bc).tag);
            draw_text(buf, 10, y, debug_text);
            y += 100;

        }

        const text_style debug_text = (text_style) {
            .scale = 1,
            .colour = (vec3s) {1,1,1},
        };

        // good reticle
        const int rscale = 4;
        draw_2d_image(gc, gc->reticle, w/2 - 5*rscale, h/2 - 5*rscale, 10*rscale, 10*rscale);

        // end draw
        glBindVertexArray(0);
        glUseProgram(0);
        glfwSwapBuffers(wc->window);
        glfwPollEvents();        
    }

    glfwTerminate();

    return 0;
}
