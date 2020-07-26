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

#include "draw.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"


chunk_manager cm = {0};
chunk_manager *cmp = &cm;

void draw_lookat_cube(vec3s cam_pos, vec3s cam_front, graphics_context *c, pick_info p) {
    
}

bool enable_debug = false;
bool load_chunks = true;

int main(int argc, char** argv) {
    int w = 2560;
    int h = 1440;


    // world gen parameters
    noise2d_params p = {0};
    arrpush(p.lf_height_amplitude, 1000);
    arrpush(p.lf_height_amplitude, 500);
    arrpush(p.lf_height_amplitude, 250);

    arrpush(p.hf_height_amplitude, 125);

    arrpush(p.hf_height_amplitude, 75);
    arrpush(p.hf_height_amplitude, 40);
    arrpush(p.hf_height_amplitude, 20);
    arrpush(p.hf_height_amplitude, 10);
/*
    arrpush(p.height_frequency, 0.0008);
    arrpush(p.height_frequency, 0.0016);
    arrpush(p.height_frequency, 0.0032);
*/
    arrpush(p.lf_height_frequency, 0.00016);
    arrpush(p.lf_height_frequency, 0.00032);
    arrpush(p.lf_height_frequency, 0.00064);

    arrpush(p.hf_height_frequency, 0.00128);

    arrpush(p.hf_height_frequency, 0.00256);
    arrpush(p.hf_height_frequency, 0.00512);
    arrpush(p.hf_height_frequency, 0.01024);
    arrpush(p.hf_height_frequency, 0.02048);

    arrpush(p.smooth_amplitude, 0.5);
    arrpush(p.smooth_amplitude, 0.25);
    arrpush(p.smooth_amplitude, 0.125);

    arrpush(p.smooth_frequency, 0.001);
    arrpush(p.smooth_frequency, 0.002);
    arrpush(p.smooth_frequency, 0.004);


    arrpush(p.cave_tendency_amplitude, 0.5);
    
    arrpush(p.cave_tendency_frequency, 0.02);


    cm.noise_params = p;

    open_simplex_noise(123456789, &cm.osn);


    camera cam = fly_camera();
    float spawn_x = 2000;
    float spawn_z = 2000;
    float spawn_y = generate_height(cm.osn, spawn_x, spawn_z, p) + 2;
    cam.pos = (vec3s) {spawn_x, spawn_y, spawn_z};
    cam.front = (vec3s) {0, 0, -1};

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
    cm.loaded_dimensions = (vec3i) {10,10, 10};
    cm.lod_dimensions = (int32_t_pair) {80, 80};
    int nchunks = cm.loaded_dimensions.x * cm.loaded_dimensions.y * cm.loaded_dimensions.z;
//    cm.gen_func = generate_flat;
    cm.gen_func = generate_v2;



    if (load_chunks) {
        cm_update(&cm, cam.pos);
    }
    cm_lod_update(&cm, cam.pos); // generates loddy boys

    for (int i = 0; i < hmlen(cm.lodmesh_hm); i++) {
        //printf("%d. vao: %d, vbo: %d, num tris: %d\n", i, cm.lodmesh_hm[i].vao, cm.lodmesh_hm[i].vbo, cm.lodmesh_hm[i].num_triangles);
    }

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

    printf("cm from main: %p\n", &cm);

    while (!glfwWindowShouldClose(wc->window)) {
        frame_counter++;

        int load_amt = cm_load_n(&cm, cam.pos, 6);
        total_genned += load_amt;
        if (load_amt == 0) {
            int light_amt = cm_light_n(&cm, cam.pos, 6);
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

        draw(gc, wc, &cm);

        glfwSwapBuffers(wc->window);
        glfwPollEvents();        
    }

    glfwTerminate();

    return 0;
}
