#include "graphics.h"
#include "debug_overlay.h"
#include "chunk_common.h"
#include "draw.h"

draw_context dc;

const int triangles_in_cube = 12;
const int triangles_in_square = 2;

draw_context *draw_init() {

    // initialize skybox
    dc.skybox_program = make_shader_program("shaders/skybox.vert", "shaders/skybox.frag");

    glGenVertexArrays(1, &dc.skybox_vao);
    glGenBuffers(1, &dc.skybox_vbo);

    float vertices[] =
    #include "cube_just_pos.h"

    glBindVertexArray(dc.skybox_vao);
    glBindBuffer(GL_ARRAY_BUFFER, dc.skybox_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // initialize sun
    dc.sun_program = make_shader_program("shaders/sun.vert", "shaders/sun.frag");

    glGenVertexArrays(1, &dc.sun_vao);
    glGenBuffers(1, &dc.sun_vbo);

    float sun_vertices[] = {
        0,0,10,
        0,1,10,
        1,0,10,

        1,1,10,
        0,1,10,
        1,0,10,
    };

    glBindVertexArray(dc.sun_vao);
    glBindBuffer(GL_ARRAY_BUFFER, dc.sun_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(sun_vertices), sun_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return &dc;
}


void draw(draw_context *dc, graphics_context *gc, window_context *wc, chunk_manager *cm, float solar_angle) {
    const float twilight_angle = 12;
    
    vec3s light_dir = (vec3s) {
        0,
        sinf(DEG_TO_RAD * solar_angle),
        cosf(DEG_TO_RAD * solar_angle),
    };
    
    float dayness;
    if (solar_angle < 180) {
        dayness = 1;
    } else if (solar_angle < 180 + twilight_angle) {
        dayness = remap(180, 180 + twilight_angle, 1, 0, solar_angle);
    } else if (solar_angle < (360 - twilight_angle)) {
        dayness = 0;
    } else {
        dayness = remap((360 - twilight_angle), 360, 0, 1, solar_angle);
    }

    //printf("dayness %f\n", dayness);

    // I should probably compute all time of day trig uniforms etc here and pass it in
    // as uniform so its not bug prone
    // call it dayness 0..1



    glClearColor(0.3, 0.5, 0.7, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mat4s view = GLMS_MAT4_IDENTITY_INIT;
    view = glms_lookat(gc->cam->pos, glms_vec3_add(gc->cam->pos, gc->cam->front), gc->cam->up);

    mat4s projection = GLMS_MAT4_IDENTITY_INIT;
    projection = glms_perspective(glm_rad(gc->cam->fovx), (float)*gc->w / *gc->h, 0.1, 30000);

    // Draw skybox
    mat4s skybox_view = glms_lookat(GLMS_VEC3_ZERO, gc->cam->front, gc->cam->up);

    glUseProgram(dc->skybox_program);
    glUniformMatrix4fv(glGetUniformLocation(dc->skybox_program, "view"), 1, GL_FALSE, skybox_view.raw[0]);
    glUniformMatrix4fv(glGetUniformLocation(dc->skybox_program, "projection"), 1, GL_FALSE, projection.raw[0]);
    glUniform1f(glGetUniformLocation(dc->skybox_program, "solar_angle"), solar_angle);
    glUniform1f(glGetUniformLocation(dc->skybox_program, "dayness"), dayness);
    glBindVertexArray(dc->skybox_vao);
    glDrawArrays(GL_TRIANGLES, 0, triangles_in_cube * 3);

    glClear(GL_DEPTH_BUFFER_BIT);

    // draw sun
    mat4s sun_model = glms_rotate(GLMS_MAT4_IDENTITY, solar_angle * DEG_TO_RAD, (vec3s){-1,0,0});

    glUseProgram(dc->sun_program);
    glUniformMatrix4fv(glGetUniformLocation(dc->sun_program, "model"), 1, GL_FALSE, sun_model.raw[0]);
    glUniformMatrix4fv(glGetUniformLocation(dc->sun_program, "view"), 1, GL_FALSE, skybox_view.raw[0]);
    glUniformMatrix4fv(glGetUniformLocation(dc->sun_program, "projection"), 1, GL_FALSE, projection.raw[0]);
    glUniform1f(glGetUniformLocation(dc->sun_program, "solar_angle"), solar_angle);
    glBindVertexArray(dc->sun_vao);
    glDrawArrays(GL_TRIANGLES, 0, triangles_in_square * 3);

    glClear(GL_DEPTH_BUFFER_BIT);

    // Draw LOD meshes
    glUseProgram(gc->lodmesh_program);
    glEnable(GL_CULL_FACE);
    glUniformMatrix4fv(glGetUniformLocation(gc->lodmesh_program, "view"), 1, GL_FALSE, view.raw[0]);
    glUniformMatrix4fv(glGetUniformLocation(gc->lodmesh_program, "projection"), 1, GL_FALSE, projection.raw[0]);
    glUniform3fv(glGetUniformLocation(gc->lodmesh_program, "player_pos"), 1, gc->cam->pos.raw);
    glUniform3fv(glGetUniformLocation(gc->lodmesh_program, "light_dir"), 1, light_dir.raw);
    glUniform1f(glGetUniformLocation(gc->lodmesh_program, "dayness"), dayness);


    for (int idx = 0; idx < hmlen(cm->lodmesh_hm); idx++) {
        lodmesh *m = &cm->lodmesh_hm[idx];

        mat4s model = GLMS_MAT4_IDENTITY_INIT;

        glBindVertexArray(m->vao);
        glDrawArrays(GL_TRIANGLES, 0, m->num_triangles * 3);
    }

    glDisable(GL_CULL_FACE);
    glClear(GL_DEPTH_BUFFER_BIT);


    vec3s light = glms_vec3_normalize((vec3s){1,2,1}); // todo remove

    // send shared uniforms
    glUseProgram(gc->mesh_program);
    glUniformMatrix4fv(glGetUniformLocation(gc->mesh_program, "view"), 1, GL_FALSE, view.raw[0]);
    glUniformMatrix4fv(glGetUniformLocation(gc->mesh_program, "projection"), 1, GL_FALSE, projection.raw[0]);
    glUniform3fv(glGetUniformLocation(gc->mesh_program, "light"), 1, light.raw);



    // Draw chunks
    glBindTexture(GL_TEXTURE_2D, gc->atlas);
    glUseProgram(gc->chunk_program);
    glUniformMatrix4fv(glGetUniformLocation(gc->chunk_program, "view"), 1, GL_FALSE, view.raw[0]);
    glUniformMatrix4fv(glGetUniformLocation(gc->chunk_program, "projection"), 1, GL_FALSE, projection.raw[0]);
    glUniform1f(glGetUniformLocation(gc->chunk_program, "dayness"), dayness);

    for (int idx = 0; idx < hmlen(cm->chunk_hm); idx++) {
        chunk *c = &cm->chunk_hm[idx];
        if (c->needs_remesh) {
            cm_mesh_chunk(cm, spread(c->key));
        }

        mat4s model = GLMS_MAT4_IDENTITY_INIT;
        model = glms_translate(model, (vec3s){
            c->key.x*CHUNK_RADIX + 0.5, 
            c->key.y*CHUNK_RADIX + 0.5, 
            c->key.z*CHUNK_RADIX + 0.5,
        });

        glUniformMatrix4fv(glGetUniformLocation(gc->chunk_program, "model"), 1, GL_FALSE, model.raw[0]);
        glBindVertexArray(c->vao);
        glDrawArrays(GL_TRIANGLES, 0, c->num_triangles * 3);
    }

    // Draw lookat cube
    pick_info lookat = pick_block(cm, gc->cam->pos, gc->cam->front, 9);
    if (lookat.success) {
        glDepthFunc(GL_LEQUAL);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

        draw_mesh(gc,  gc->cube, (vec3s) {
            lookat.coords.x + 0.5, 
            lookat.coords.y + 0.5, 
            lookat.coords.z + 0.5}, 
            (vec3s){0}, 0);

        glDepthFunc(GL_LESS);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // Draw reticle
    const int rscale = 2;
    draw_2d_image(gc, gc->reticle, *wc->w/2 - 5*rscale, *wc->h/2 - 5*rscale, 10*rscale, 10*rscale);

    // Draw debug info
    if (wc->show_info) {
        draw_debug_info(wc->dt, *gc->cam, wc, cm);
    }

    glBindVertexArray(0);
    glUseProgram(0);
}