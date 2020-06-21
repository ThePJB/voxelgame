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
#include "world.h"

context c;

context *get_context() {
    return &c;
}

float min(float a, float b) { return a < b? a : b; }
float max(float a, float b) { return a > b? a : b; }

void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow* window, int width, int height); 
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

void graphics_init() {
    {
        c.w = 2560;
        c.h = 1440;
        c.mouse_lastx = c.w/2;
        c.mouse_lasty = c.h/2;

        c.wireframe = false;

        c.cam = fly_camera();
    }

    {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        c.window = glfwCreateWindow(c.w, c.h, "ya mums ya dad", NULL, NULL);

        
        if (!c.window) {
            printf("failed to make window\n");
            exit(1);
        }

        // @tidy maybe I can reorder this a bit better

        glfwMakeContextCurrent(c.window);
        glfwSetInputMode(c.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);  

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            printf("failed to initialize GLAD\n");
            exit(1);
        }

        glfwSetFramebufferSizeCallback(c.window, framebuffer_size_callback);
        glfwSetCursorPosCallback(c.window, mouse_callback);  
        glfwSetScrollCallback(c.window, scroll_callback); 
        glfwSetKeyCallback(c.window, key_callback);

        glViewport(0,0,c.w,c.h);
        glEnable(GL_DEPTH_TEST);
    }

    c.mesh_program = make_shader_program("shaders/vertex.glsl", "shaders/fragment.glsl");

    // make cube
   float vertices[] =
   #include "cube.h"

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

    c.cube.vertex_data = vertices;
    c.cube.vao = vao;
    c.cube.texture = load_texture("assets/tromp.jpg");
    c.cube.num_triangles = 12;


    c.tromp = load_texture("assets/tromp.jpg");
    c.spoderman = load_texture("assets/spoderman.jpg");


}

void graphics_teardown() {
    glfwTerminate();
}

void begin_draw(context *c) {
    glClearColor(0.3, 0.5, 0.7, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mat4s view = GLMS_MAT4_IDENTITY_INIT;
    view = glms_lookat(c->cam.pos, glms_vec3_add(c->cam.pos, c->cam.front), c->cam.up);

    mat4s projection = GLMS_MAT4_IDENTITY_INIT;
    projection = glms_perspective(glm_rad(c->cam.fovx), (float)c->w / c->h, 0.1, 1000);

    vec3s light = glms_vec3_normalize((vec3s){1,2,1});


    // send shared uniforms
    glUseProgram(c->mesh_program);
    glUniformMatrix4fv(glGetUniformLocation(c->mesh_program, "view"), 1, GL_FALSE, view.raw[0]);
    glUniformMatrix4fv(glGetUniformLocation(c->mesh_program, "projection"), 1, GL_FALSE, projection.raw[0]);
    glUniform3fv(glGetUniformLocation(c->mesh_program, "light"), 1, light.raw);
}


void draw_mesh(context *c, mesh m) {
    glUseProgram(c->mesh_program);

    //printf("draw cube texture %u vao %u num tris %u\n", m.texture, m.vao, m.num_triangles);
    // for now just do this here
    mat4s model = GLMS_MAT4_IDENTITY_INIT;
    model = glms_rotate(model, glfwGetTime(), (vec3s){1,1,1});
    m.transform = model;

    // upload mesh transform
    glUniformMatrix4fv(glGetUniformLocation(c->mesh_program, "model"), 1, GL_FALSE, m.transform.raw[0]);

    glBindTexture(GL_TEXTURE_2D, m.texture);
    glBindVertexArray(m.vao);
    glDrawArrays(GL_TRIANGLES, 0, m.num_triangles * 3);
}


void end_draw(context *c) {
    glfwSwapBuffers(c->window);
    glfwPollEvents();

    glBindVertexArray(0);
    glUseProgram(0);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    c.w = width;
    c.h = height;
    glViewport(0,0,width,height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    float xoff = xpos - c.mouse_lastx;
    float yoff = c.mouse_lasty - ypos; // reversed Y

    c.mouse_lastx = xpos;
    c.mouse_lasty = ypos;

    const float sensitivity = 0.05;
    xoff *= sensitivity;
    yoff *= sensitivity;

    c.cam.yaw += xoff;
    c.cam.pitch += yoff;

    c.cam.pitch = min(c.cam.pitch, 89);
    c.cam.pitch = max(c.cam.pitch, -89);

    vec3s direction;
    direction.x = cos(glm_rad(c.cam.yaw)) * cos(glm_rad(c.cam.pitch));
    direction.y = sin(glm_rad(c.cam.pitch));
    direction.z = sin(glm_rad(c.cam.yaw)) * cos(glm_rad(c.cam.pitch));
    c.cam.front = glms_normalize(direction);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    c.cam.fovx -= (float)yoffset;
    c.cam.fovx = min(c.cam.fovx, 179);
    c.cam.fovx = max(c.cam.fovx, 1);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        if (c.wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        c.wireframe = !c.wireframe;
    }    
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        c.show_info = !c.show_info;
    }
}
