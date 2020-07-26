#include "graphics.h"
#include "debug_overlay.h"
#include "chunk_common.h"

void draw(graphics_context *gc, window_context *wc, chunk_manager *cm) {

    glClearColor(0.3, 0.5, 0.7, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mat4s view = GLMS_MAT4_IDENTITY_INIT;
    view = glms_lookat(gc->cam->pos, glms_vec3_add(gc->cam->pos, gc->cam->front), gc->cam->up);

    mat4s projection = GLMS_MAT4_IDENTITY_INIT;
    projection = glms_perspective(glm_rad(gc->cam->fovx), (float)*gc->w / *gc->h, 0.1, 30000);

    vec3s light = glms_vec3_normalize((vec3s){1,2,1}); // todo remove

    // send shared uniforms
    glUseProgram(gc->mesh_program);
    glUniformMatrix4fv(glGetUniformLocation(gc->mesh_program, "view"), 1, GL_FALSE, view.raw[0]);
    glUniformMatrix4fv(glGetUniformLocation(gc->mesh_program, "projection"), 1, GL_FALSE, projection.raw[0]);
    glUniform3fv(glGetUniformLocation(gc->mesh_program, "light"), 1, light.raw);

    glUseProgram(gc->chunk_program);
    glUniformMatrix4fv(glGetUniformLocation(gc->chunk_program, "view"), 1, GL_FALSE, view.raw[0]);
    glUniformMatrix4fv(glGetUniformLocation(gc->chunk_program, "projection"), 1, GL_FALSE, projection.raw[0]);
    glUniform3fv(glGetUniformLocation(gc->chunk_program, "light"), 1, light.raw);
    
    // Draw LOD meshes
    glUseProgram(gc->lodmesh_program);
    glEnable(GL_CULL_FACE);
    glUniformMatrix4fv(glGetUniformLocation(gc->lodmesh_program, "view"), 1, GL_FALSE, view.raw[0]);
    glUniformMatrix4fv(glGetUniformLocation(gc->lodmesh_program, "projection"), 1, GL_FALSE, projection.raw[0]);
    glUniform3fv(glGetUniformLocation(gc->lodmesh_program, "player_pos"), 1, gc->cam->pos.raw);

    for (int idx = 0; idx < hmlen(cm->lodmesh_hm); idx++) {
        lodmesh *m = &cm->lodmesh_hm[idx];

        mat4s model = GLMS_MAT4_IDENTITY_INIT;

        glBindVertexArray(m->vao);
        glDrawArrays(GL_TRIANGLES, 0, m->num_triangles * 3);
    }

    glDisable(GL_CULL_FACE);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Draw chunks
    glBindTexture(GL_TEXTURE_2D, gc->atlas);
    glUseProgram(gc->chunk_program);
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

        glUniformMatrix4fv(glGetUniformLocation(gc->mesh_program, "model"), 1, GL_FALSE, model.raw[0]);
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