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
#include "ram_usage.h"

chunk_manager cm = {0};

int main(int argc, char** argv) {
    //test_chunk();
    //exit(0);

    context *c = graphics_init();
    text_init(c);
    init_chunk_manager(&cm, 123456789);
    chunk_manager_position_hint(&cm, (vec3s){0,0,0}); // generates and meshes chunks

    c->cam.pos = (vec3s) {0, 16, 0};
    c->cam.front = (vec3s) {0, 0, -1};

    float last = 0;
    float dt = 0;
    
    while (!glfwWindowShouldClose(c->window)) {
        if (glfwGetKey(c->window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(c->window, true);
        }
        float time = glfwGetTime();
        dt = time - last;
        c->dt = dt;
        last = time;

        camera new_cam = update_camera(c->window, c->cam, dt);

        c->cam = new_cam;

        begin_draw(c);
        draw_mesh(c, c->cube); 
        draw_chunks(&cm, c);

        if (c->show_info) {
            char buf[64] = {0};
            const text_style debug_text = (text_style) {
                .scale = 1,
                .colour = (vec3s) {1,1,1},
            };
            int y = 10;
            sprintf(buf, "%.2f fps", 1.0 / c->dt);
            draw_text(buf, 10, y, debug_text);
            y += 100;

            sprintf(buf, "Facing {%.2f, %.2f, %.2f}", c->cam.front.x, c->cam.front.y, c->cam.front.z);
            draw_text(buf, 10, y, debug_text);
            y += 100;

            sprintf(buf, "Pos {%.2f, %.2f, %.2f}", c->cam.pos.x, c->cam.pos.y, c->cam.pos.z);
            draw_text(buf, 10, y, debug_text);
            y += 100;            
            
            sprintf(buf, "RAM: %lu MB", get_ram_usage()/ (1024*1024));
            draw_text(buf, 10, y, debug_text);
            y += 100;        

            sprintf(buf, "VRAM: %lu MB (remember this is total)", get_vram_usage()/ (1024));
            draw_text(buf, 10, y, debug_text);
            y += 100;

            // block coords
            vec3l bc = (vec3l) {c->cam.pos.x, c->cam.pos.y, c->cam.pos.z};

            vec3i block_coords = {0};
            vec3i chunk_coords = {0};
    
            world_to_block_and_chunk(&chunk_coords, &block_coords, bc);
            chunk_slot *cs = get_chunk_slot(&cm, chunk_coords);
            if (cs) {
                chunk *c = &cs->chunk;
                sprintf(buf, "In chunk %d %d %d, empty: %d, block coords %d %d %d", 
                    chunk_coords.x, chunk_coords.y, chunk_coords.z,
                    c->empty,
                    block_coords.x, block_coords.y, block_coords.z);
            } else {
                sprintf(buf, "not in chunk");
            }

            draw_text(buf, 10, y, debug_text);
            y += 100;

            sprintf(buf, "Block ur in: %d", get_block(&cm, bc).tag);
            draw_text(buf, 10, y, debug_text);
            y += 100;

            //set_block(&cm, bc, (block){.tag = BLOCK_DIRT});
            //printf("setting %ld %ld %ld\n", bc.x, bc.y, bc.z);
            //c->cube.x = c->cam.pos.x + c->cam.front.x * 10;
            //c->cube.y = c->cam.pos.y + c->cam.front.y * 10;
            //c->cube.z = c->cam.pos.z + c->cam.front.z * 10;

            
        }

        const text_style debug_text = (text_style) {
            .scale = 1,
            .colour = (vec3s) {1,1,1},
        };
        // reticle lol
        draw_text("+", c->w/2 - 24, c->h/2 - 24, debug_text);

        
        end_draw(c);
    }

    graphics_teardown();

    return 0;
}
