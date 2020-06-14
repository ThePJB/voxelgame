#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>

#include "glad.h"
#include <GLFW/glfw3.h>
#include <cglm/struct.h>
#include "shader.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow* window, int width, int height); 
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

vec3s cam_pos = (vec3s) {0,0,3};
vec3s cam_front = (vec3s) {0,0,-1};
vec3s cam_up = (vec3s) {0,1,0};

const float default_scrX = 2560;
const float default_scrY = 1440;
float scrX = default_scrX;
float scrY = default_scrY;
float lastX = default_scrX/2;
float lastY = default_scrY/2;
float pitch = 0;
float yaw = -90;
float fovx = 45;
bool wireframe = false;

float min(float a, float b) { return a < b? a : b; }
float max(float a, float b) { return a > b? a : b; }

int main(int argc, char** argv) {
    


    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(scrX, scrY, "ya mums ya dad", NULL, NULL);
    if (!window) {
        printf("failed to make window\n");
        exit(1);
    }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("failed to initialize GLAD\n");
        exit(1);
    }

    glViewport(0,0,800,600);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);  
    glfwSetScrollCallback(window, scroll_callback); 
    glfwSetKeyCallback(window, key_callback);

    glEnable(GL_DEPTH_TEST);

    // Shader stuff
    unsigned int vertex_shader = make_shader("vertex.glsl", GL_VERTEX_SHADER);
    unsigned int fragment_shader = make_shader("fragment.glsl", GL_FRAGMENT_SHADER);

    unsigned int success;
    char buf[512] = {0};
    unsigned int shader_program;
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, 512, NULL, buf);
        printf("error linking shaders: %s\n", buf);
        exit(1);
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    // cube
    /*
    float vertices[] = {
        -1, 1, 1,
        1,1,1,
        -1,-1,1,
        1,1,1,
        1,-1,1,
        -1,-1,1,
        -1,1,-1,
        1,1,-1,
        -1,-1,-1,
        1,1,-1,
        1,-1,-1,
        -1,-1,-1,
        -1,1,-1,
        -1,1,1,
        -1,-1,-1,
        -1,-1,1,
        -1,1,1,
        -1,-1,-1,
        1,1,-1,
        1,1,1,
        1,-1,-1,
        1,-1,1,
        1,1,1,
        1,-1,-1,
        -1,1,-1,
        1,1,-1,
        1,1,1,
        -1,1,-1,
        -1,1,1,
        1,1,1,
        -1,-1,-1,
        1,-1,-1,
        1,-1,1,
        -1,-1,-1,
        -1,-1,1,
        1,-1,1,
    };
    */

   float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0, 0, -1,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0, 0, -1,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0, 0, -1,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0, 0, -1,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0, 0, -1,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0, 0, -1,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0, 0, 1,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0, 0, 1,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0, 0, 1,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0, 0, 1,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 0, 0, 1,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0, 0, 1,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1, 0, 0,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, -1, 0, 0,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1, 0, 0,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1, 0, 0,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, -1, 0, 0,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1, 0, 0,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1, 0, 0,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1, 0, 0,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1, 0, 0,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1, 0, 0,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1, 0, 0,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1, 0, 0,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0, -1, 0,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0, -1, 0,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0, -1, 0,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0, -1, 0,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0, -1, 0,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0, -1, 0,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0, 1, 0,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0, 1, 0,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0, 1, 0,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0, 1, 0,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0, 1, 0,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0, 1, 0,
};

    unsigned int vao;
    glGenVertexArrays(1, &vao);

    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texture coords
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // normals
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
    glEnableVertexAttribArray(2);


    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    float last = 0;
    float dt = 0;


    // texture stuff
    // load texture
    int width, height, nrChannels;
    unsigned char *data = stbi_load("tromp.jpg", &width, &height, &nrChannels, 0); 
    unsigned int texture;
    glGenTextures(1, &texture); 
    glBindTexture(GL_TEXTURE_2D, texture);  
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    // wrap
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    // filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // mipmapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }
        float time = glfwGetTime();
        dt = time - last;
        last = time;

        const float cam_speed = 2.5 * dt;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            cam_pos = glms_vec3_add(cam_pos, glms_vec3_scale(cam_front, cam_speed));
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            cam_pos = glms_vec3_sub(cam_pos, glms_vec3_scale(cam_front, cam_speed));
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            cam_pos = glms_vec3_sub(cam_pos, glms_vec3_scale(glms_normalize(glms_vec3_cross(cam_front, cam_up)), cam_speed));
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            cam_pos = glms_vec3_add(cam_pos, glms_vec3_scale(glms_normalize(glms_vec3_cross(cam_front, cam_up)), cam_speed));
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            cam_pos = glms_vec3_add(cam_pos, glms_vec3_scale(cam_up, cam_speed));
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
            cam_pos = glms_vec3_sub(cam_pos, glms_vec3_scale(cam_up, cam_speed));

            
        

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        /*
        vec3s cam_pos = (vec3s) {0,0,3};
        vec3s cam_target = (vec3s) {0,0,0};
        vec3s cam_dir = glms_normalize(glms_vec3_sub(cam_pos, cam_target));

        vec3s up = (vec3s) {0,1,0};
        vec3s cam_right = glms_normalize(glms_vec3_cross(up, cam_dir));
        vec3s cam_up = glms_cross(cam_dir, cam_right); 
        */

        mat4s view = GLMS_MAT4_IDENTITY_INIT;
        view = glms_lookat(cam_pos, glms_vec3_add(cam_pos, cam_front), cam_up);
        //view = glms_translate(view, (vec3s){0,0,-6});

        mat4s model = GLMS_MAT4_IDENTITY_INIT;
        model = glms_rotate(model, glfwGetTime(), (vec3s){1,1,1});
        mat4s projection = GLMS_MAT4_IDENTITY_INIT;

        projection = glms_perspective(glm_rad(fovx), scrX / scrY, 0.1, 100);

        vec3s light = glms_vec3_normalize((vec3s){1,2,1});



        // send uniforms
        glUseProgram(shader_program);
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "view"), 1, GL_FALSE, view.raw[0]);
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, model.raw[0]);
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE, projection.raw[0]);
        glUniform3fv(glGetUniformLocation(shader_program, "light"), 1, light.raw);

        glBindTexture(GL_TEXTURE_2D, texture);
        glBindVertexArray(vao);
        //glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        //glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        
        glfwSwapBuffers(window);
        glfwPollEvents();

        glBindVertexArray(0);
    }

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    scrX = width;
    scrY = height;
    glViewport(0,0,width,height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    float xoff = xpos - lastX;
    float yoff = lastY - ypos; // reversed Y

    lastX = xpos;
    lastY = ypos;

    const float sensitivity = 0.05;
    xoff *= sensitivity;
    yoff *= sensitivity;

    yaw += xoff;
    pitch += yoff;

    pitch = min(pitch, 89);
    pitch = max(pitch, -89);

    vec3s direction;
    direction.x = cos(glm_rad(yaw)) * cos(glm_rad(pitch));
    direction.y = sin(glm_rad(pitch));
    direction.z = sin(glm_rad(yaw)) * cos(glm_rad(pitch));
    cam_front = glms_normalize(direction);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    fovx -= (float)yoffset;
    fovx = min(fovx, 179);
    fovx = max(fovx, 1);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        if (wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        wireframe = !wireframe;
    }
}