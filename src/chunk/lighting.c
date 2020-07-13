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



void lighting_process_node(chunk_manager *cm, vec3l_queue *vq, 
    uint8_t current_illumination, vec3l target_pos) {

    printf("process illumination from thing with %d to pos %d %d %d\n", current_illumination, spread(target_pos));
    
    uint8_t illumination_at_target = world_get_illumination(cm, target_pos);
    block_tag block_at_target = world_get_block(cm, target_pos);

    if (block_defs[block_at_target].opaque) {
        printf("its opaque so i dont care\n");
        return;
    }

    if (illumination_at_target < current_illumination - 1) {
        printf("its darker so im updating\n");
        world_set_illumination(cm, target_pos, current_illumination - 1);
        vec3l_queue_push(vq, target_pos);
    }
    printf("not doing shit\n");
}

// this is a bit trickier because we want to zero the values after they are used for comparison so lets do that in the other func after calling this
void lighting_delete_node(chunk_manager *cm, vec3l_queue *vq,
    uint8_t current_illumination, vec3l target_pos) {

    uint8_t illumination_at_target = world_get_illumination(cm, target_pos);
    block_tag block_at_target = world_get_block(cm, target_pos);

    //
    
    if (block_defs[block_at_target].opaque || illumination_at_target == 0 || illumination_at_target == LIGHT_EMPTY_CHUNK) {
        return;
    }

    if (illumination_at_target >= current_illumination) {
        // must be illuminated in some other way
        vec3l_queue_push(&propagation_queue, target_pos);
    } else {
        vec3l_queue_push(&deletion_queue, target_pos);
    }
}

void enqueue_neighbours(chunk_manager *cm, vec3l_queue *vq, long x, long y, long z) {
    vec3l current_pos = (vec3l){x,y,z};
    vec3l n_px = current_pos; n_px.x++; vec3l_queue_push(vq, n_px);
    vec3l n_mx = current_pos; n_mx.x--; vec3l_queue_push(vq, n_mx);
    vec3l n_py = current_pos; n_py.y++; vec3l_queue_push(vq, n_py);
    vec3l n_my = current_pos; n_my.y--; vec3l_queue_push(vq, n_my);
    vec3l n_pz = current_pos; n_pz.z++; vec3l_queue_push(vq, n_pz);
    vec3l n_mz = current_pos; n_mz.z--; vec3l_queue_push(vq, n_mz);
}


void resolve_propagation(chunk_manager *cm) {
    while(vec3l_queue_len(&propagation_queue) > 0) {
        vec3l current_pos = vec3l_queue_pop(&propagation_queue);
        uint8_t current_illumination = world_get_illumination(cm, current_pos);

        // neighbours
        vec3l n_px = current_pos; n_px.x++; lighting_process_node(cm, &propagation_queue, current_illumination, n_px);
        vec3l n_mx = current_pos; n_mx.x--; lighting_process_node(cm, &propagation_queue, current_illumination, n_mx);

        vec3l n_py = current_pos; n_py.y++; lighting_process_node(cm, &propagation_queue, current_illumination, n_py);
        vec3l n_my = current_pos; n_my.y--; lighting_process_node(cm, &propagation_queue, current_illumination, n_my);

        vec3l n_pz = current_pos; n_pz.z++; lighting_process_node(cm, &propagation_queue, current_illumination, n_pz);
        vec3l n_mz = current_pos; n_mz.z--; lighting_process_node(cm, &propagation_queue, current_illumination, n_mz);
    }
}

void resolve_deletion(chunk_manager *cm) {
    while(vec3l_queue_len(&deletion_queue) > 0) {
        printf("delete queue len %d\n", vec3l_queue_len(&deletion_queue));
        vec3l current_pos = vec3l_queue_pop(&deletion_queue);
        uint8_t current_illumination = world_get_illumination(cm, current_pos);

        vec3l n_px = current_pos; n_px.x++; lighting_delete_node(cm, &propagation_queue, current_illumination, n_px);
        vec3l n_mx = current_pos; n_mx.x--; lighting_delete_node(cm, &propagation_queue, current_illumination, n_mx);

        vec3l n_py = current_pos; n_py.y++; lighting_delete_node(cm, &propagation_queue, current_illumination, n_py);
        vec3l n_my = current_pos; n_my.y--; lighting_delete_node(cm, &propagation_queue, current_illumination, n_my);

        vec3l n_pz = current_pos; n_pz.z++; lighting_delete_node(cm, &propagation_queue, current_illumination, n_pz);
        vec3l n_mz = current_pos; n_mz.z--; lighting_delete_node(cm, &propagation_queue, current_illumination, n_mz);

        world_set_illumination(cm, current_pos, 0);
    }
}

void cm_add_light(chunk_manager *cm, uint8_t luminance, long x, long y, long z) {
    vec3l new_light_pos = {x,y,z};
    world_set_illumination(cm, new_light_pos, luminance);
    vec3l_queue_push(&propagation_queue, new_light_pos);

    while (vec3l_queue_len(&propagation_queue) > 0) {
        printf("pgate queue len %d\n", vec3l_queue_len(&propagation_queue));
        vec3l current_pos = vec3l_queue_pop(&propagation_queue);
        uint8_t current_illumination = world_get_illumination(cm, current_pos);

        for (direction dir = 0; dir < NUM_DIRS; dir++) {
            printf("Dir %s: ", dir_name[dir]);
            vec3l neighbour_pos = vec3l_add(current_pos, unit_vec3l[dir]);
            block_tag neighbour_block = world_get_block(cm, neighbour_pos);
            block_definition neighbour_props = block_defs[neighbour_block];

            if (neighbour_props.opaque || neighbour_props.luminance == LIGHT_EMPTY_CHUNK) {
                printf("opaque\n");
                continue;
            }

            uint8_t neighbour_illumination = world_get_illumination(cm, neighbour_pos);
            if (neighbour_illumination < current_illumination - 1) {
                printf("pushin\n");
                world_set_illumination(cm, neighbour_pos, current_illumination-1);
                vec3l_queue_push(&propagation_queue, neighbour_pos);
            } else {
                printf(" too bright apparently neigh %u us %u\n", neighbour_illumination, current_illumination);
            }
        }
    }
    
    // todo adjust size and warn if it gets too big or whatever
    /*
    vec3l global_source_pos = {x,y,z};

    vec3l_queue_push(&propagation_queue, global_source_pos);
    world_set_illumination(cm, global_source_pos, luminance);
    resolve_propagation(cm);
    */
}

void cm_delete_light(chunk_manager *cm, long x, long y, long z) {
    /*
    vec3l current_pos = {x,y,z};

    vec3l_queue_push(&deletion_queue, current_pos);
    resolve_deletion(cm);
    resolve_propagation(cm);
    */
}

void cm_update_light_for_block_placement(chunk_manager *cm, long x, long y, long z) {

}

void cm_update_light_for_block_deletion(chunk_manager *cm, long x, long y, long z) {
    printf("block deletion\n");
    vec3l deleted_block_pos = {x,y,z};
    for (direction dir = 0; dir < NUM_DIRS; dir++) {
        vec3l neighbour_pos = vec3l_add(deleted_block_pos, unit_vec3l[dir]);
        vec3l_queue_push(&propagation_queue, neighbour_pos);
    }
    printf("pgate queue len %d\n", vec3l_queue_len(&propagation_queue));

    // see if this does the trick
    while (vec3l_queue_len(&propagation_queue) > 0) {
        printf("pgate queue len %d\n", vec3l_queue_len(&propagation_queue));
        vec3l current_pos = vec3l_queue_pop(&propagation_queue);
        uint8_t current_illumination = world_get_illumination(cm, current_pos);

        for (direction dir = 0; dir < NUM_DIRS; dir++) {
            printf("Dir %s: ", dir_name[dir]);
            vec3l neighbour_pos = vec3l_add(current_pos, unit_vec3l[dir]);
            block_tag neighbour_block = world_get_block(cm, neighbour_pos);
            block_definition neighbour_props = block_defs[neighbour_block];

            if (neighbour_props.opaque || neighbour_props.luminance == LIGHT_EMPTY_CHUNK) {
                printf("opaque\n");
                continue;
            }

            uint8_t neighbour_illumination = world_get_illumination(cm, neighbour_pos);
            if (neighbour_illumination < current_illumination - 1) {
                printf("pushin\n");
                world_set_illumination(cm, neighbour_pos, current_illumination-1);
                vec3l_queue_push(&propagation_queue, neighbour_pos);
            } else {
                printf(" too bright apparently neigh %u us %u\n", neighbour_illumination, current_illumination);
            }
        }
    }


    /*

    vec3l n_px = current_pos; n_px.x++; lighting_process_node(cm, &propagation_queue, world_get_illumination(cm, n_px), current_pos);
    vec3l n_mx = current_pos; n_mx.x--; lighting_process_node(cm, &propagation_queue, world_get_illumination(cm, n_mx), current_pos);

    vec3l n_py = current_pos; n_py.y++; lighting_process_node(cm, &propagation_queue, world_get_illumination(cm, n_py), current_pos);
    vec3l n_my = current_pos; n_my.y--; lighting_process_node(cm, &propagation_queue, world_get_illumination(cm, n_my), current_pos);

    vec3l n_pz = current_pos; n_pz.z++; lighting_process_node(cm, &propagation_queue, world_get_illumination(cm, n_pz), current_pos);
    vec3l n_mz = current_pos; n_mz.z--; lighting_process_node(cm, &propagation_queue, world_get_illumination(cm, n_mz), current_pos);

    //enqueue_neighbours(cm, &propagation_queue, x, y, z);
    //resolve_propagation(cm);
    */
}