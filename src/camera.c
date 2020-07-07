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

/*
#include "camera.h"
#include "util.h"
#include "world.h"
*/
// maybe camera and controls cause they are pretty tightly coupled

camera fly_camera() {
    camera cam;

    cam.pos = (vec3s) {0,0,3};
    cam.front = (vec3s) {0,0,-1};
    cam.up = (vec3s) {0,1,0};
    cam.pitch = 0;
    cam.yaw = -90;
    cam.fovx = 90;
    cam.type = CAM_FLY;

    return cam;
}

#define CHUNK_RADIX 16

extern chunk_manager *cm;

void move(vec3s *pos, vec3s amount) {
    vec3s old_pos = *pos;
    *pos = glms_vec3_add(*pos, amount);
    if ((pos->x > 0 && old_pos.x < 0) || (long int)old_pos.x / CHUNK_RADIX < (long int)pos->x / CHUNK_RADIX) {
        chunk_treadmill(cm, DIR_PX);
        printf("+x chunk boundary\n");
    } else if ((pos->x < 0 && old_pos.x > 0) || (long int)old_pos.x / CHUNK_RADIX > (long int)pos->x / CHUNK_RADIX) {
        printf("-x chunk boundary\n");
    } else if ((pos->y > 0 && old_pos.y < 0) || (long int)old_pos.y / CHUNK_RADIX < (long int)pos->y / CHUNK_RADIX) {
        printf("+y chunk boundary\n");
    } else if ((pos->y < 0 && old_pos.y > 0) || (long int)old_pos.y / CHUNK_RADIX > (long int)pos->y / CHUNK_RADIX) {
        printf("-y chunk boundary\n");
    } else if ((pos->z > 0 && old_pos.z < 0) || (long int)old_pos.z / CHUNK_RADIX < (long int)pos->z / CHUNK_RADIX) {
        printf("+z chunk boundary\n");
    } else if ((pos->z < 0 && old_pos.z > 0) || (long int)old_pos.z / CHUNK_RADIX > (long int)pos->z / CHUNK_RADIX) {
        printf("-z chunk boundary\n");
    }


}


camera update_camera(GLFWwindow* window, camera cam, float dt) {
    if (cam.type == CAM_FLY) {
        const float speed_multi = 8;
        float cam_speed = speed_multi * dt; 
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL)) {
            cam_speed *= 8;
        }

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            move(&cam.pos, glms_vec3_scale(cam.front, cam_speed));
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            move(&cam.pos, glms_vec3_scale(cam.front, -1 * cam_speed));
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            move(&cam.pos, glms_vec3_scale(glms_normalize(glms_vec3_cross(cam.front, cam.up)), -1*cam_speed));
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            move(&cam.pos, glms_vec3_scale(glms_normalize(glms_vec3_cross(cam.front, cam.up)), cam_speed));
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            move(&cam.pos, glms_vec3_scale(cam.up, cam_speed));
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            move(&cam.pos, glms_vec3_scale(cam.up, -1*cam_speed));
        }
            
    } else if (cam.type == CAM_FPS) {
        // not implemented
    }            

    return cam;
}

void print_camera(camera c) {
    printf("--------------\nCamera:\n"
    "Type: %d\n"
    "pos: [%.2f %.2f %.2f]\n"
    "front: [%.2f %.2f %.2f]\n"
    "up: [%.2f %.2f %.2f]\n"
    "pitch: %.2f yaw: %.2f fovx: %.2f\n--------------\n",
    c.type,
    c.pos.x,
    c.pos.y,
    c.pos.z,
    c.front.x,
    c.front.y,
    c.front.z,
    c.up.x,
    c.up.y,
    c.up.z,
    c.pitch, c.yaw, c.fovx);

}
