#include "camera.h"

// maybe camera and controls cause they are pretty tightly coupled

camera fly_camera() {
    camera cam;

    cam.pos = (vec3s) {0,0,3};
    cam.front = (vec3s) {0,0,-1};
    cam.up = (vec3s) {0,1,0};
    cam.pitch = 0;
    cam.yaw = -90;
    cam.fovx = 45;
    cam.type = CAM_FLY;

    return cam;
}


camera update_camera(GLFWwindow* window, camera cam, float dt) {
    if (cam.type == CAM_FLY) {
        const speed_multi = 32;
        float cam_speed = speed_multi * dt; 

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            cam.pos = glms_vec3_add(cam.pos, glms_vec3_scale(cam.front, cam_speed));
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cam.pos = glms_vec3_sub(cam.pos, glms_vec3_scale(cam.front, cam_speed));
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cam.pos = glms_vec3_sub(cam.pos, glms_vec3_scale(glms_normalize(glms_vec3_cross(cam.front, cam.up)), cam_speed));
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cam.pos = glms_vec3_add(cam.pos, glms_vec3_scale(glms_normalize(glms_vec3_cross(cam.front, cam.up)), cam_speed));
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            cam.pos = glms_vec3_add(cam.pos, glms_vec3_scale(cam.up, cam_speed));
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
            cam.pos = glms_vec3_sub(cam.pos, glms_vec3_scale(cam.up, cam_speed));
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
