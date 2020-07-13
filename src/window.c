#include "graphics.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "glad.h"
#include <GLFW/glfw3.h>
#include <cglm/struct.h>
#include "shader.h"
#include "texture.h"
#include "util.h"
#include "window.h"
#include "noise.h"
#include "camera.h"

#include "chunk_common.h"

window_context wc = {0};

/*
The separation here is that this is mostly GLFW and input stuff
graphics will be kept a bit cleaner
most stuff doesnt depend on this but does on graphics
this depends on most stuff for input handling i think 
*/

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow* window, int width, int height); 
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

window_context *window_init(char *title, int *w, int *h, camera *cam) {
    wc.w = w;
    wc.h = h;
    wc.cam = cam;
    wc.wireframe = false;
    
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(*w, *h, title, NULL, NULL);
    wc.window = window;

    if (!window) {
        printf("failed to make window\n");
        exit(1);
    }

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);  
    glfwSetScrollCallback(window, scroll_callback); 
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    return &wc;
}

block_tag place_block = 0;
extern chunk_manager *cmp;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        if (wc.wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        wc.wireframe = !wc.wireframe;
    }    
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        wc.show_info = !wc.show_info;
    }
    if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
        n2d_reseed(&cmp->world_noise.noise_lf_heightmap, rand());
        n2d_reseed(&cmp->world_noise.noise_hf_heightmap, rand());
        n3d_reseed(&cmp->world_noise.noise_cliff_carver, rand());
        cm_update(cmp, (vec3s){0,0,0});
    }
    if (key == GLFW_KEY_4 && action == GLFW_PRESS) {
        printf("meshing\n");
        vec3i chunk_pos = world_pos_to_chunk_pos(wc.cam->pos);
        cm_mesh_chunk(cmp, spread(chunk_pos));
    }
    if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
        place_block = (place_block + (NUM_BLOCKS - 1)) % NUM_BLOCKS;
        printf("placing %d\n", place_block);
    }
    if (key == GLFW_KEY_E && action == GLFW_PRESS) {
        place_block = (place_block + 1) % NUM_BLOCKS;
        printf("placing %d\n", place_block);
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_1 && action == GLFW_PRESS) {

        //enable_debug = true;

        pick_info p = pick_block(cmp, wc.cam->pos, wc.cam->front, 9);
        vec3l new_coords = vec3l_add(p.coords, unit_vec3l[p.normal_dir]);
        if (p.success) world_set_block(cmp, new_coords, place_block);
        
        //enable_debug = false;

    } else if (button == GLFW_MOUSE_BUTTON_2 && action == GLFW_PRESS) {
        printf("rmb\n");
        pick_info p = pick_block(cmp, wc.cam->pos, wc.cam->front, 9);
        if (p.success) world_set_block(cmp, p.coords,BLOCK_AIR);
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    float xoff = xpos - wc.mouse_lastx;
    float yoff = wc.mouse_lasty - ypos; // reversed Y

    wc.mouse_lastx = xpos;
    wc.mouse_lasty = ypos;

    const float sensitivity = 0.05;
    xoff *= sensitivity;
    yoff *= sensitivity;

    wc.cam->yaw += xoff;
    wc.cam->pitch += yoff;

    wc.cam->pitch = min(wc.cam->pitch, 89);
    wc.cam->pitch = max(wc.cam->pitch, -89);

    vec3s direction;
    direction.x = cos(glm_rad(wc.cam->yaw)) * cos(glm_rad(wc.cam->pitch));
    direction.y = sin(glm_rad(wc.cam->pitch));
    direction.z = sin(glm_rad(wc.cam->yaw)) * cos(glm_rad(wc.cam->pitch));
    wc.cam->front = glms_normalize(direction);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    wc.cam->fovx -= (float)yoffset;
    wc.cam->fovx = min(wc.cam->fovx, 179);
    wc.cam->fovx = max(wc.cam->fovx, 1);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    *(wc.w) = width;
    *(wc.h) = height;
    glViewport(0,0,width,height);
}