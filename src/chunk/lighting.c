#include "chunk_common.h"

void lighting_process_node(chunk_manager *cm, vec3l_queue *vq, 
    uint8_t current_illumination, vec3l target_pos) {
    
    uint8_t illumination_at_target = world_get_illumination(cm, target_pos);
    block block_at_target = world_get_block(cm, target_pos);

    if (block_defs[block_at_target.tag].opaque) {
        return;
    }

    if (illumination_at_target < current_illumination - 1) {
        world_set_illumination(cm, target_pos, current_illumination - 1);
        vec3l_queue_push(vq, target_pos);
    }
}

void cm_add_light(chunk_manager *cm, uint8_t luminance, long x, long y, long z) {
    // todo adjust size and warn if it gets too big or whatever
    vec3l bfqueue[CHUNK_RADIX_3];
    vec3l_queue vq = {
        .items = bfqueue,
        .start = 0,
        .end = 0,
        .size = CHUNK_RADIX_3,
    };

    vec3l global_source_pos = {x,y,z};

    vec3l_queue_push(&vq, global_source_pos);
    world_set_illumination(cm, global_source_pos, luminance);

    printf("queue len %d\n", vec3l_queue_len(&vq));
    while(vec3l_queue_len(&vq) > 0) {
        printf("a");
        vec3l current_pos = vec3l_queue_pop(&vq);
        uint8_t current_illumination = world_get_illumination(cm, current_pos);

        // neighbours
        vec3l n_px = current_pos; n_px.x++; lighting_process_node(cm, &vq, current_illumination, n_px);
        vec3l n_mx = current_pos; n_mx.x--; lighting_process_node(cm, &vq, current_illumination, n_mx);

        vec3l n_py = current_pos; n_py.y++; lighting_process_node(cm, &vq, current_illumination, n_py);
        vec3l n_my = current_pos; n_my.y--; lighting_process_node(cm, &vq, current_illumination, n_my);

        vec3l n_pz = current_pos; n_pz.z++; lighting_process_node(cm, &vq, current_illumination, n_pz);
        vec3l n_mz = current_pos; n_mz.z--; lighting_process_node(cm, &vq, current_illumination, n_mz);
        printf("queue len %d\n", vec3l_queue_len(&vq));
    }
}