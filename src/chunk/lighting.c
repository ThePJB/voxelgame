#include "chunk_common.h"

vec3l propagation_queue_array[CHUNK_RADIX_3];
vec3l_queue propagation_queue = {
    .items = propagation_queue_array,
    .start = 0,
    .end = 0,
    .size = CHUNK_RADIX_3,
};

vec3l deletion_queue_array[CHUNK_RADIX_3];
vec3l_queue deletion_queue = {
    .items = deletion_queue_array,
    .start = 0,
    .end = 0,
    .size = CHUNK_RADIX_3,
};

vec3l sunlight_queue_array[CHUNK_RADIX_3];
vec3l_queue sunlight_queue = {
    .items = sunlight_queue_array,
    .start = 0,
    .end = 0,
    .size = CHUNK_RADIX_3,
};

vec3l sunlight_deletion_queue_array[CHUNK_RADIX_3];
vec3l_queue sunlight_deletion_queue = {
    .items = sunlight_deletion_queue_array,
    .start = 0,
    .end = 0,
    .size = CHUNK_RADIX_3,
};

void finish_propagating(chunk_manager *cm) {
    while (vec3l_queue_len(&propagation_queue) > 0) {
        vec3l current_pos = vec3l_queue_pop(&propagation_queue);
        uint8_t current_illumination = light_get_block(cm, current_pos).value;
        if (current_illumination == 0) {
            continue;
        }

        for (direction dir = 0; dir < NUM_DIRS; dir++) {
            vec3l neighbour_pos = vec3l_add(current_pos, unit_vec3l[dir]);
            block_tag neighbour_block = world_get_block(cm, neighbour_pos).value;
            block_definition neighbour_props = block_defs[neighbour_block];

            if (neighbour_props.opaque || neighbour_props.luminance == 255) {
                continue;
            }

            uint8_t neighbour_illumination = light_get_block(cm, neighbour_pos).value;
            if (neighbour_illumination < current_illumination - 1) {
                light_set_block(cm, neighbour_pos, current_illumination - 1);
                vec3l_queue_push(&propagation_queue, neighbour_pos);
            }
        }
    }
}

void finish_propagating_sunlight(chunk_manager *cm) {
    while (vec3l_queue_len(&sunlight_queue) > 0) {
        vec3l current_pos = vec3l_queue_pop(&sunlight_queue);
        maybe_uint8_t current_sunlight = light_get_sky(cm, current_pos);
        if (current_sunlight.ok == false ||
            current_sunlight.value == 0) {
            continue;
        }

        for (direction dir = 0; dir < NUM_DIRS; dir++) {
            vec3l neighbour_pos = vec3l_add(current_pos, unit_vec3l[dir]);
            block_tag neighbour_block = world_get_block(cm, neighbour_pos).value;
            block_definition neighbour_props = block_defs[neighbour_block];
            maybe_uint8_t neighbour_sunlight = light_get_sky(cm, neighbour_pos);

            if (neighbour_sunlight.ok == false ||
                neighbour_props.opaque) {
                continue;
            }

            // downward propagating sunlight case
            if (dir == DIR_MY && current_sunlight.value == SKY_LIGHT_FULL) {
                light_set_sky(cm, neighbour_pos, current_sunlight.value);
                vec3l_queue_push(&sunlight_queue, neighbour_pos);
                //printf("sunlight propagate down\n");
            } else if (neighbour_sunlight.value < current_sunlight.value - 1) {
                light_set_sky(cm, neighbour_pos, current_sunlight.value - 1);
                vec3l_queue_push(&sunlight_queue, neighbour_pos);
            }
        }
    }
}

void finish_deleting(chunk_manager *cm) {
    while(vec3l_queue_len(&deletion_queue) > 0) {
        vec3l current_pos = vec3l_queue_pop(&deletion_queue);
        maybe_uint8_t current_illumination = light_get_block(cm, current_pos);
        if (current_illumination.ok == false) {
            printf("warning: trying to delete light but couldnt get it");
            continue;
        }

        for (direction dir = 0; dir < NUM_DIRS; dir++) {
            vec3l neighbour_pos = vec3l_add(current_pos, unit_vec3l[dir]);
            block_tag neighbour_block = world_get_block(cm, neighbour_pos).value;
            maybe_uint8_t neighbour_illumination = light_get_block(cm, neighbour_pos);

            if (block_defs[neighbour_block].luminance > 0) {
                // if neighbour is a light we need to add it to propagation queue
                vec3l_queue_push(&propagation_queue, neighbour_pos);
                continue;
            }

            if (block_defs[neighbour_block].opaque || 
                neighbour_illumination.ok == false ||
                neighbour_illumination.value == 0) {
                    continue;
            }

            if (neighbour_illumination.value >= current_illumination.value) {
                // must be illuminated in some other way
                vec3l_queue_push(&propagation_queue, neighbour_pos);
            } else {
                vec3l_queue_push(&deletion_queue, neighbour_pos);
            }
        }

        light_set_block(cm, current_pos, 0);
    }
}

void finish_deleting_sunlight(chunk_manager *cm) {
    while(vec3l_queue_len(&sunlight_deletion_queue) > 0) {
        vec3l current_pos = vec3l_queue_pop(&sunlight_deletion_queue);
        uint8_t current_sunlight = light_get_sky(cm, current_pos).value;

        for (direction dir = 0; dir < NUM_DIRS; dir++) {
            vec3l neighbour_pos = vec3l_add(current_pos, unit_vec3l[dir]);
            block_tag neighbour_block = world_get_block(cm, neighbour_pos).value;
            maybe_uint8_t neighbour_sunlight = light_get_sky(cm, neighbour_pos);

            if (block_defs[neighbour_block].opaque || 
                neighbour_sunlight.ok == false || 
                neighbour_sunlight.value == 0) {
                continue;
            }

            // this block will be darker, so we have to delete it and let propagation re run
            if ((current_sunlight == SKY_LIGHT_FULL && dir == DIR_MY) ||
                neighbour_sunlight.value < current_sunlight) {
                vec3l_queue_push(&sunlight_deletion_queue, neighbour_pos);
            } else {
                // block is illuminated some other way
                vec3l_queue_push(&sunlight_queue, neighbour_pos);
            }
        }

        light_set_sky(cm, current_pos, 0);
    }
}

void light_add(chunk_manager *cm, uint8_t luminance, long x, long y, long z) {
    vec3l new_light_pos = {x,y,z};
    light_set_block(cm, new_light_pos, luminance);
    vec3l_queue_push(&propagation_queue, new_light_pos);

    finish_propagating(cm);
}

void light_delete(chunk_manager *cm, long x, long y, long z) {
    vec3l current_pos = {x,y,z};

    vec3l_queue_push(&deletion_queue, current_pos);
    finish_deleting(cm);
    finish_propagating(cm);

    vec3l_queue_push(&sunlight_deletion_queue, current_pos);
    finish_deleting_sunlight(cm);

    extern bool test_disable_del_propagate_sun;
    if (!test_disable_del_propagate_sun) {
        finish_propagating_sunlight(cm);
    }
}


void light_propagate_sky(chunk_manager *cm, int32_t x, int32_t y, int32_t z) {
    vec3l pos = {x,y,z};
    vec3l_queue_push(&sunlight_queue, pos);
    finish_propagating_sunlight(cm);
}

void light_issue_remesh(chunk_manager *cm, vec3l pos) {
    vec3i_pair coords = world_posl_to_block_chunk(pos);
    if (coords.l.x == 0) {
        vec3i update_coords = vec3i_add(coords.r, unit_vec3i[DIR_MX]);
        chunk *c = hmgetp(cm->chunk_hm, update_coords);
        c->needs_remesh = true;
    } else if (coords.l.x == 15) {
        vec3i update_coords = vec3i_add(coords.r, unit_vec3i[DIR_PX]);
        chunk *c = hmgetp(cm->chunk_hm, update_coords);
        c->needs_remesh = true;
    }
    if (coords.l.y == 0) {
        vec3i update_coords = vec3i_add(coords.r, unit_vec3i[DIR_MY]);
        chunk *c = hmgetp(cm->chunk_hm, update_coords);
        c->needs_remesh = true;
    } else if (coords.l.y == 15) {
        vec3i update_coords = vec3i_add(coords.r, unit_vec3i[DIR_PY]);
        chunk *c = hmgetp(cm->chunk_hm, update_coords);
        c->needs_remesh = true;
    }
    if (coords.l.z == 0) {
        vec3i update_coords = vec3i_add(coords.r, unit_vec3i[DIR_MZ]);
        chunk *c = hmgetp(cm->chunk_hm, update_coords);
        c->needs_remesh = true;
    } else if (coords.l.z == 15) {
        vec3i update_coords = vec3i_add(coords.r, unit_vec3i[DIR_PZ]);
        chunk *c = hmgetp(cm->chunk_hm, update_coords);
        c->needs_remesh = true;
    }
    chunk *c = hmgetp(cm->chunk_hm, coords.r);
    c->needs_remesh = true;
}


// So this is to perform the initial lighting calcs for a chunk if its generated
// - adds all the light values of each light source and updates
// - if at highest point in chunk and its surface chunk, perform skylight calculations
//      actually thats not that correct becasue 
void light_initialize_for_chunk(chunk_manager *cm, int x, int y, int z) {
    
    vec3i chunk_pos = {x,y,z};

    int chunk_idx = hmgeti(cm->chunk_hm, chunk_pos);
    
    if (chunk_idx == -1) {
        printf("trying to fix lighting of nonexistent chunk?\n");
        return;
    } 

    for (int bx = 0; bx < CHUNK_RADIX; bx++) {
        int32_t global_block_x = bx + x * CHUNK_RADIX;
        for (int bz = 0; bz < CHUNK_RADIX; bz++) {
            int32_t global_block_z = bz + z * CHUNK_RADIX;

            int32_t surface_y = world_get_surface_y(cm, global_block_x, global_block_z).value;

            for (int by = 0; by < CHUNK_RADIX; by++) {
                int32_t global_block_y = by + y * CHUNK_RADIX;

                vec3i block_pos = (vec3i){bx,by,bz};
                //print_vec3i(block_pos);
                int idx = chunk_3d_to_1d(block_pos);
                //printf("fix lighting index %d\n", idx);


                //printf("%d\n", chunk_idx);
                chunk *c = &cm->chunk_hm[chunk_idx];
                block_tag block = c->blocks[idx];
                //printf("%u\n", (uint16_t)block);

                uint8_t lum = block_defs[block].luminance;
                if (lum > 0) {
                    vec3l world_block_pos = world_block_chunk_to_posl(block_pos, (vec3i){x,y,z});
                    light_add(cm, lum, global_block_x, global_block_y, global_block_z);
                }

                if (global_block_y > surface_y) {
                    c->sky_light_levels[idx] = SKY_LIGHT_FULL;
                }
            }

            // so we run the sunlight propagation algorithm at the highest y point in this chunk,
            // if this chunk is the one with the surface (highest opauqe block)
            if (floor_div(surface_y, CHUNK_RADIX) == y) {
                light_propagate_sky(cm, global_block_x, CHUNK_MAX, global_block_z);
            }
        }
    }
}



// basic structure access



void light_set_block(chunk_manager *cm, vec3l pos, uint8_t illumination) {
    vec3i_pair coords = world_posl_to_block_chunk(pos);
    int idx = hmgeti(cm->chunk_hm, coords.r);
    
    if (idx < 0) {
        //printf("tried setting block light of an unloaded chunk\n");
        return;
    }

    //light_issue_remesh(cm, pos);
    cm->chunk_hm[idx].block_light_levels[chunk_3d_to_1d(coords.l)] = illumination;
}

void light_set_sky(chunk_manager *cm, vec3l pos, uint8_t illumination) {
    vec3i_pair coords = world_posl_to_block_chunk(pos);
    int idx = hmgeti(cm->chunk_hm, coords.r);
    
    if (idx < 0) {
        //printf("tried setting sky light of an unloaded chunk\n");
        return;
    }

    //light_issue_remesh(cm, pos);
    cm->chunk_hm[idx].sky_light_levels[chunk_3d_to_1d(coords.l)] = illumination;
}

maybe_uint8_t light_get_block(chunk_manager *cm, vec3l pos) {

    vec3i_pair coords = world_posl_to_block_chunk(pos);

    int idx = hmgeti(cm->chunk_hm, coords.r);
    
    if (idx < 0) {
        //printf("tried getting illumination of an unloaded chunk %d %d %d\n", spread(coords.r));
        return (maybe_uint8_t) {255, false};
    }

    return (maybe_uint8_t) {cm->chunk_hm[idx].block_light_levels[chunk_3d_to_1d(coords.l)], true};
}

maybe_uint8_t light_get_sky(chunk_manager *cm, vec3l pos) {

    vec3i_pair coords = world_posl_to_block_chunk(pos);

    int idx = hmgeti(cm->chunk_hm, coords.r);
    if (idx < 0) {
        //printf("tried getting illumination of an unloaded chunk %d %d %d\n", spread(coords.r));
        return (maybe_uint8_t) {255, false};
    }

    return (maybe_uint8_t) {cm->chunk_hm[idx].sky_light_levels[chunk_3d_to_1d(coords.l)], true};
}

/*
maybe_uint8_t light_get_block(chunk_manager *cm, vec3l pos) {
    static vec3i last_called_with_chunk = {INT32_MAX, INT32_MAX, INT32_MAX}; // im assuming this only ran once
    static int idx = -1;

    vec3i_pair coords = world_posl_to_block_chunk(pos);
    if (coords.r.x == last_called_with_chunk.x && 
        coords.r.y == last_called_with_chunk.y &&
        coords.r.z == last_called_with_chunk.z) {

        // hit means we dont need to look chunk up in hashtable
        

        } else {
            idx = hmgeti(cm->chunk_hm, coords.r);
            last_called_with_chunk = coords.r;
        }
    
    if (idx < 0) {
        //printf("tried getting illumination of an unloaded chunk %d %d %d\n", spread(coords.r));
        return (maybe_uint8_t) {255, false};
    }

    return (maybe_uint8_t) {cm->chunk_hm[idx].block_light_levels[chunk_3d_to_1d(coords.l)], true};
}

maybe_uint8_t light_get_sky(chunk_manager *cm, vec3l pos) {
    static vec3i last_called_with_chunk = {INT32_MAX, INT32_MAX, INT32_MAX}; // im assuming this only ran once
    static int idx = -1;

    vec3i_pair coords = world_posl_to_block_chunk(pos);
    if (coords.r.x == last_called_with_chunk.x && 
        coords.r.y == last_called_with_chunk.y &&
        coords.r.z == last_called_with_chunk.z) {

        // hit means we dont need to look chunk up in hashtable
        

        } else {
            idx = hmgeti(cm->chunk_hm, coords.r);
            last_called_with_chunk = coords.r;
        }
    if (idx < 0) {
        //printf("tried getting illumination of an unloaded chunk %d %d %d\n", spread(coords.r));
        return (maybe_uint8_t) {255, false};
    }

    return (maybe_uint8_t) {cm->chunk_hm[idx].sky_light_levels[chunk_3d_to_1d(coords.l)], true};
}
*/