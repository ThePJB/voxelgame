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
#include "priority_queue.h"

//#define STB_DEFINE
//#include "stb.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

chunk_manager cm = {0};
chunk_manager *cmp = &cm;

const int load_amt = 5;

bool enable_debug = false;
bool load_chunks = true;
bool fast_forward = false;

double cum_mesh_time = 0;
double cum_gen_time = 0;
double cum_light_time = 0;
double cum_decorate_time = 0;

double max_mesh_time = 0;
double max_gen_time = 0;
double max_light_time = 0;
double max_decorate_time = 0;

int main(int argc, char** argv) {
    int w = 640;
    int h = 480;


    // world gen parameters
    noise2d_params p = {0};
    arrpush(p.lf_height_amplitude, 250);
    arrpush(p.lf_height_amplitude, 250);
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

    arrpush(p.treeness_amplitude, 30);
    
    arrpush(p.treeness_frequency, 0.005);

    p.snow_above_height = 300;
    p.dirt_above_height = -20;
    p.sand_below_height = -20;
    p.water_below_height = -25;

    cm.noise_params = p;

    open_simplex_noise(123456789, &cm.osn);


    camera cam = fly_camera();
    float spawn_x = 2000;
    float spawn_z = 300;
    float spawn_y = max(generate_height(cm.osn, spawn_x, spawn_z, p) + 2, cm.noise_params.water_below_height);
    cam.pos = (vec3s) {spawn_x, spawn_y, spawn_z};
    cam.front = (vec3s) {0, 0, -1};

    window_context *wc = window_init("sick game", &w, &h, &cam);
    graphics_context *gc = graphics_init(&w, &h, &cam);
    draw_context *dc = draw_init();

    if (argc == 2 && !strcmp(argv[1], "--test")) {
        chunk_test();
        world_test();
        test_util();
        cm_test();
        test_priority_queue();
        exit(0);
    } else if (argc == 3 && !strcmp(argv[1], "--profile")) {
        int n = atoi(argv[2]);
        world_benchmark(n);
        exit(0);
    }


    text_init(gc);
    //cm.world_noise = chunk_rngs_init(123456789);
    cm.loaded_dimensions = (vec3i) {16, 12, 16};
    cm.lod_dimensions = (int32_t_pair) {60, 60};
    int nchunks = cm.loaded_dimensions.x * cm.loaded_dimensions.y * cm.loaded_dimensions.z;
//    cm.gen_func = generate_flat;
    cm.gen_func = chunk_generate;



    if (load_chunks) {
        cm_update(&cm, cam.pos);
    }
    cm_lod_update(&cm, cam.pos); // generates loddy boys

    for (int i = 0; i < hmlen(cm.lodmesh_hm); i++) {
        //printf("%d. vao: %d, vbo: %d, num tris: %d\n", i, cm.lodmesh_hm[i].vao, cm.lodmesh_hm[i].vbo, cm.lodmesh_hm[i].num_triangles);
    }

    float last = 0;
    float dt = 0;
    
    int total_genned = 0;
    int total_lit = 0;
    int total_meshed = 0;

    int frame_counter = 0;
    float solar_angle = 0;

    printf("cm from main: %p\n", &cm);

    while (!glfwWindowShouldClose(wc->window)) {
        frame_counter++;
        cm_load_n(cmp, cam.pos, load_amt);

        if (frame_counter % 60 == 0) {
            //printf("loaded %d lit %d meshed %d\n", total_genned, total_lit, total_meshed);
        }
        
        if (glfwGetKey(wc->window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(wc->window, true);
        }
        if (glfwGetKey(wc->window, GLFW_KEY_5) == GLFW_PRESS) {
            fast_forward = true;
        } else {
            fast_forward = false;
        }

        float time = glfwGetTime();
        dt = time - last;
        wc->dt = dt;
        last = time;

        cam = update_camera(wc->window, cam, dt);

        if (fast_forward) {
            solar_angle += dt * 20;
        } else {
            solar_angle += dt * 1;
        }

        while (solar_angle > 360) {
            solar_angle -= 360;
        }

        //printf("solar angle %f\n", solar_angle);
        draw(dc, gc, wc, &cm, solar_angle);

        glfwSwapBuffers(wc->window);
        glfwPollEvents();        
    }

    glfwTerminate();

    printf("cum gen time: %f\n", cum_gen_time);
    printf("cum light time: %f\n", cum_light_time);
    printf("cum mesh time: %f\n", cum_mesh_time);
    printf("cum decorate time: %f\n", cum_decorate_time);

    printf("max gen time: %f\n", max_gen_time);
    printf("max light time: %f\n", max_light_time);
    printf("max mesh time: %f\n", max_mesh_time);
    printf("max decorate time: %f\n", max_decorate_time);

    return 0;
}
