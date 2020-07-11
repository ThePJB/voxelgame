#include "chunk_common.h"

// --------------- picking

float intbound(float s, float ds) {
    if (ds < 0) {
        return intbound(-s, -ds);
    } else {
        if (ds > 0) {
            s = s - floorf(s);
        } else {
            s = s - ceilf(s);
        }
        return (1-s)/ds;
    }
}

pick_info pick_block(chunk_manager *world, vec3s pos, vec3s facing, float max_distance) {
    debugf("facing %.2f, %.2f, %.2f\n", facing.x, facing.y, facing.z);
    debugf("at %.2f, %.2f, %.2f\n", pos.x, pos.y, pos.z);

    pick_info ret = {0};
    ret.success = true;

    ret.coords = world_pos_to_block_pos(pos);

    int sx = signum(facing.x);
    int sy = signum(facing.y);
    int sz = signum(facing.z);

    float tMaxX = intbound(pos.x, facing.x);
    float tMaxY = intbound(pos.y, facing.y);
    float tMaxZ = intbound(pos.z, facing.z);

    debugf("initial tmx: %f, tmy: %.2f, tmz: %f\n", tMaxX, tMaxY, tMaxZ);
    //debugf("sx %d sy %d sz %d\n", sx, sy, sz);

    float accX = 0;
    float accY = 0;
    float accZ = 0;

    float tDeltaX = (float)sx / facing.x;
    float tDeltaY = (float)sy / facing.y;
    float tDeltaZ = (float)sz / facing.z;

    float max_squared = max_distance*max_distance;

    int n = 0;
    while (accX*accX + accY*accY + accZ*accZ <= max_squared) {
        n++;
        block_tag t = world_get_block(world, ret.coords).tag;
        debugf("x: %ld y: %ld z: %ld, t: %d\n", ret.coords.x, ret.coords.y, ret.coords.z, t);
        debugf("x dist: %.3f, y dist: %.3f, z dist: %.3f\n", accX, accY, accZ);
        //printf("tmx: %.2f, tmy: %.2f, tmz: %.2f\n", tMaxX, tMaxY, tMaxZ);
        if (t != BLOCK_AIR) {
            debugf("found block\n");
            ret.success=true;
            return ret;
        }

        if (tMaxX < tMaxY) {
            if (tMaxX < tMaxZ) {
                // X min
                ret.coords.x += sx;
                accX = tMaxX;
                tMaxX += tDeltaX;
                ret.normal_x = -sx;
                ret.normal_y = 0;
                ret.normal_z = 0;
            } else {
                // Z min
                ret.coords.z += sz;
                accZ = tMaxZ;
                tMaxZ += tDeltaZ;
                ret.normal_x = 0;
                ret.normal_y = 0;
                ret.normal_z = -sz;
            }
        } else {
            if (tMaxY < tMaxZ) {
                // Y min
                ret.coords.y += sy;
                accY = tMaxY;
                tMaxY += tDeltaY;
                ret.normal_x = 0;
                ret.normal_y = -sy;
                ret.normal_z = 0;
            } else {
                // Z min (again)
                ret.coords.z += sz;
                accZ = tMaxZ;
                tMaxZ += tDeltaZ;
                ret.normal_x = 0;
                ret.normal_y = 0;
                ret.normal_z = -sz;
            }
        }

    }
    ret.success = false;
    debugf("didnt find anything after n iters %d\n", n);
    //printf("bailed with accx: %.2f, accy: %.2f, accz: %.2f\n", accX, accY, accZ);

    return ret;
    
}