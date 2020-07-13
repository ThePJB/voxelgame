#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "glad.h"
#include <GLFW/glfw3.h>
#include <cglm/struct.h>
#include "shader.h"

#include "graphics.h"
#include "text.h"
#include "window.h"
#include "stb_ds.h"

#include "chunk_common.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"


chunk_manager cm = {0};
chunk_manager *cmp = &cm;

void draw_lookat_cube(vec3s cam_pos, vec3s cam_front, graphics_context *c, pick_info p) {
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
    //chunk_test();
    //world_test();
    //test_util();
    //cm_test();
    //exit(0);

    window_context *wc = window_init("sick game", &w, &h, &cam);
    graphics_context *gc = graphics_init(&w, &h, &cam);

    text_init(gc);
    cm.world_noise = chunk_rngs_init(123456789);
    cm.loaded_dimensions = (vec3i) {8, 4, 8};

    cam.pos = (vec3s) {0, 0, 0};

    cam.front = (vec3s) {0, 0, -1};

    cm_update(&cm, cam.pos); // generates and meshes chunks

    float last = 0;
    float dt = 0;
    
    while (!glfwWindowShouldClose(wc->window)) {
        cm_load_n(&cm, cam.pos, 20);
        if (glfwGetKey(wc->window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(wc->window, true);
        }
        float time = glfwGetTime();
        dt = time - last;
        wc->dt = dt;
        last = time;

        cam = update_camera(wc->window, cam, dt);

        pre_draw(gc);
        
        world_draw(&cm, gc);

        pick_info lookat = pick_block(&cm, cam.pos, cam.front, 9);
        draw_lookat_cube(cam.pos, cam.front, gc, lookat);

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

            if (lookat.success) {
                sprintf(buf, "Lookat block: {%d %d %d}, type: %d", spread(lookat.coords), world_get_block(&cm, lookat.coords));
                draw_text(buf, 10, y, debug_text);
                y += 100;
                
                vec3l neighbour_block_pos = vec3l_add(lookat.coords, unit_vec3l[lookat.normal_dir]);
                sprintf(buf, "Lookat face: %s, light: %u", dir_name[lookat.normal_dir], world_get_illumination(&cm, neighbour_block_pos));
                draw_text(buf, 10, y, debug_text);
                y += 100;
            } else {
                sprintf(buf, "Lookat block: none");
                draw_text(buf, 10, y, debug_text);
                y += 100;

                sprintf(buf, "Lookat face: none");
                draw_text(buf, 10, y, debug_text);
                y += 100;
            }

            sprintf(buf, "Pos {%.2f, %.2f, %.2f}", cam.pos.x, cam.pos.y, cam.pos.z);
            draw_text(buf, 10, y, debug_text);
            y += 100;            
            
            sprintf(buf, "RAM: %lu VRAM: %lu MB, ", get_ram_usage()/ (1024*1024), get_vram_usage()/ (1024));
            draw_text(buf, 10, y, debug_text);
            y += 100;        

            // block coords
            vec3l bc = (vec3l) {cam.pos.x, cam.pos.y, cam.pos.z};

            vec3i block_coords = {0};
            vec3i chunk_coords = {0};
    
            world_global_to_block_chunk(&chunk_coords, &block_coords, bc);
            int idx = hmgeti(cm.chunk_hm, chunk_coords);
            
            if (idx > -1) {
                chunk c = cm.chunk_hm[idx];
                sprintf(buf, "In chunk %d %d %d, empty: %d, block coords %d %d %d, 4conn: %d", 
                    chunk_coords.x, chunk_coords.y, chunk_coords.z,
                    c.empty,
                    block_coords.x, block_coords.y, block_coords.z,
                    c.loaded_4con_neighbours);
            } else {
                sprintf(buf, "not in chunk");
            }

            draw_text(buf, 10, y, debug_text);
            y += 100;

            sprintf(buf, "Block ur in: %d", world_get_block(&cm, bc));
            draw_text(buf, 10, y, debug_text);
            y += 100;

        }

        const text_style debug_text = (text_style) {
            .scale = 1,
            .colour = (vec3s) {1,1,1},
        };

        // good reticle
        const int rscale = 2;
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
