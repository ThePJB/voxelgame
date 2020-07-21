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

#include "debug_overlay.h"

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
    window_context *wc = window_init("sick game", &w, &h, &cam);
    graphics_context *gc = graphics_init(&w, &h, &cam);

    if (argc == 2 && !strcmp(argv[1], "--test")) {
        chunk_test();
        world_test();
        test_util();
        cm_test();
        exit(0);
    } else if (argc == 3 && !strcmp(argv[1], "--profile")) {
        int n = atoi(argv[2]);
        world_benchmark(n);
        exit(0);
    }


    text_init(gc);
    //cm.world_noise = chunk_rngs_init(123456789);
    open_simplex_noise(123456789, &cm.osn);
    cm.loaded_dimensions = (vec3i) {14,9,14};
    int nchunks = cm.loaded_dimensions.x * cm.loaded_dimensions.y * cm.loaded_dimensions.z;
//    cm.gen_func = generate_flat;
    cm.gen_func = generate_v1;
    cam.pos = (vec3s) {0, 0, 0};

    cam.front = (vec3s) {0, 0, -1};

    cm_update(&cm, cam.pos); // generates and meshes chunks

    // initial load etc
    cm_load_n(&cm, cam.pos, nchunks);
    cm_light_n(&cm, cam.pos, nchunks);
    cm_mesh_n(&cm, cam.pos, nchunks);

    float last = 0;
    float dt = 0;
    
    int total_genned = 0;
    int total_lit = 0;
    int total_meshed = 0;

    int frame_counter = 0;

    while (!glfwWindowShouldClose(wc->window)) {
        frame_counter++;

        int load_amt = cm_load_n(&cm, cam.pos, 3);
        total_genned += load_amt;
        if (load_amt == 0) {
            int light_amt = cm_light_n(&cm, cam.pos, 3);
            total_lit += light_amt;
            if (light_amt == 0) {
                total_meshed += cm_mesh_n(&cm, cam.pos, 8);
            }
        }

        if (frame_counter % 60 == 0) {
            printf("loaded %d lit %d meshed %d\n", total_genned, total_lit, total_meshed);
        }
        
        
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
            draw_debug_info(dt, cam, wc, &cm);
        }            

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
