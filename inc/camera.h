#ifndef CAMERA_H
#define CAMERA_H

#include <GLFW/glfw3.h>
#include <cglm/struct.h>

typedef enum {
    CAM_FLY,
    CAM_FPS,
    NUM_CAMS,
} camera_type;

typedef struct {
    vec3s pos;
    vec3s front;
    vec3s up;
    float pitch;
    float yaw;
    float fovx;
    camera_type type;
} camera;

camera update_camera(GLFWwindow *window, camera cam, float dt);

void print_camera(camera c);

camera fly_camera();

#endif