#include "debug_overlay.h"

void draw_debug_info(float dt, camera cam, window_context *wc, chunk_manager *cm) {

    char buf[128] = {0};
    const text_style debug_text = (text_style) {
        .scale = 1,
        .colour = (vec3s) {1,1,1},
    };
    int y = 10;

    vec3l in_block = vec3s_to_vec3l(cam.pos);
    vec3i block_coords = world_posl_to_block_chunk(in_block).l;
    vec3i chunk_coords = world_posl_to_block_chunk(in_block).r;

    pick_info lookat = pick_block(cm, cam.pos, cam.front, 9);

    int chunk_idx = hmgeti(cm->chunk_hm, chunk_coords);


    sprintf(buf, "%.2f fps", 1.0 / wc->dt);
    draw_text(buf, 10, y, debug_text);
    y += 100;

    sprintf(buf, "Facing {%.2f, %.2f, %.2f}", cam.front.x, cam.front.y, cam.front.z);
    draw_text(buf, 10, y, debug_text);
    y += 100;

    if (lookat.success) {
        sprintf(buf, "Lookat block: {%d %d %d}, type: %d light: %u", 
            spread(lookat.coords), world_get_block(cm, lookat.coords).value,
            light_get_block(cm, lookat.coords).value);
    } else {
        sprintf(buf, "Lookat block: none");
    }
    draw_text(buf, 10, y, debug_text);
    y += 100;
        
    if (lookat.success) {
        vec3l neighbour_block_pos = vec3l_add(lookat.coords, unit_vec3l[lookat.normal_dir]);
        sprintf(buf, "Lookat face: %s, light: %u sun, %u block", 
            dir_name[lookat.normal_dir], 
            light_get_sky(cm, neighbour_block_pos).value,
            light_get_block(cm, neighbour_block_pos).value
            );
    } else {
        sprintf(buf, "Lookat face: none");
    }
    draw_text(buf, 10, y, debug_text);
    y += 100;
    
    sprintf(buf, "Pos {%.2f, %.2f, %.2f}", cam.pos.x, cam.pos.y, cam.pos.z);
    draw_text(buf, 10, y, debug_text);
    y += 100;            
    
    sprintf(buf, "RAM: %u VRAM: %u MB, ", get_ram_usage()/ (1024*1024), get_vram_usage()/ (1024));
    draw_text(buf, 10, y, debug_text);
    y += 100;        

    chunk *c = hmgetp_null(cm->chunk_hm, chunk_coords);
    
    if (c) {
        sprintf(buf, "In chunk %d %d %d, block coords %d %d %d, 4conn: %d", 
            spread(chunk_coords),
            spread(block_coords),
            c->loaded_4con_neighbours);
    } else {
        sprintf(buf, "not in chunk");
    }

    draw_text(buf, 10, y, debug_text);
    y += 100;

    maybe_block_tag in_block_type = world_get_block(cm, in_block);
    if (in_block_type.ok) {
        sprintf(buf, "Block ur in: %d", in_block_type.value);
    } else {
        sprintf(buf, "Block ur in: none");
    }
    draw_text(buf, 10, y, debug_text);
    y += 100;            
    
    extern block_tag place_block;
    sprintf(buf, "Placing: %d", place_block);
    draw_text(buf, 10, y, debug_text);
    y += 100;

    sprintf(buf, "At global block pos: %d %d %d", spread(in_block));
    draw_text(buf, 10, y, debug_text);
    y += 100;

    maybe_int32_t surface_y = world_get_surface_y(cm, in_block.x, in_block.z);
    if (surface_y.ok) {
        sprintf(buf, "surface y here: %d", surface_y.value);
    } else {
        sprintf(buf, "surface y here: unknown");
    }
    
    draw_text(buf, 10, y, debug_text);
    y += 100;

}