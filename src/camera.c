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

#include "chunk_common.h"

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

extern chunk_manager *cmp;
extern bool load_chunks;

void move(vec3s *pos, vec3s amount) {
    vec3s old_pos = *pos;
    *pos = glms_vec3_add(*pos, amount);
    if ((pos->x > 0 && old_pos.x < 0) || (int32_t)old_pos.x / CHUNK_RADIX < (int32_t)pos->x / CHUNK_RADIX) {
        //printf("+x chunk boundary\n");
        if (load_chunks) {
            cm_update(cmp, *pos);
        }
        
        cm_lod_update(cmp, *pos);
    } else if ((pos->x < 0 && old_pos.x > 0) || (int32_t)old_pos.x / CHUNK_RADIX > (int32_t)pos->x / CHUNK_RADIX) {
        //printf("-x chunk boundary\n");
        if (load_chunks) {
            cm_update(cmp, *pos);
        }
        cm_lod_update(cmp, *pos);
    } else if ((pos->y > 0 && old_pos.y < 0) || (int32_t)old_pos.y / CHUNK_RADIX < (int32_t)pos->y / CHUNK_RADIX) {
        //printf("+y chunk boundary\n");
        if (load_chunks) {
            cm_update(cmp, *pos);
        }
    } else if ((pos->y < 0 && old_pos.y > 0) || (int32_t)old_pos.y / CHUNK_RADIX > (int32_t)pos->y / CHUNK_RADIX) {
        //printf("-y chunk boundary\n");
        if (load_chunks) {
            cm_update(cmp, *pos);
        }
    } else if ((pos->z > 0 && old_pos.z < 0) || (int32_t)old_pos.z / CHUNK_RADIX < (int32_t)pos->z / CHUNK_RADIX) {
        //printf("+z chunk boundary\n");
        if (load_chunks) {
            cm_update(cmp, *pos);
        }
        cm_lod_update(cmp, *pos);
    } else if ((pos->z < 0 && old_pos.z > 0) || (int32_t)old_pos.z / CHUNK_RADIX > (int32_t)pos->z / CHUNK_RADIX) {
        //printf("-z chunk boundary\n");
        if (load_chunks) {
            cm_update(cmp, *pos);
        }
        cm_lod_update(cmp, *pos);
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

    enable_debug = false;
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
