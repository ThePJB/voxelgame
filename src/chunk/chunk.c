#include "chunk_common.h"

void chunk_print(chunk c) {
    printf("x: %d y: %d z: %d\n blocks ptr: %p\n", spread(c.key), c.blocks);
}

chunk_rngs chunk_rngs_init(int64_t seed) {
    const int vec = 987234;
    return (chunk_rngs) {
        .noise_lf_heightmap = n2d_create(seed, 2, 0.01, 2, 50, 0.5),
        .noise_hf_heightmap = n2d_create(seed*vec, 3, 0.04, 2, 12.5, 0.5),
        .noise_cliff_carver = n3d_create(seed*vec*vec, 3, 0.03, 2, 20, 0.5),
        .noise_cave_carver = n3d_create(seed*vec*vec*vec, 4, 0.05, 2, 10, 0.7),
    };
}

vec3i chunk_1d_to_3d(int idx) {
    vec3i ret;
    ret.x = idx / CHUNK_RADIX_2;
    ret.y = (idx / CHUNK_RADIX) % CHUNK_RADIX;
    ret.z = idx % CHUNK_RADIX;
    return ret;
}

int chunk_3d_to_1d(vec3i pos) {
    return pos.x * CHUNK_RADIX_2 + pos.y * CHUNK_RADIX + pos.z;
}

// for checking if neighbour exists
// maybe not super efficient but who cares
bool neighbour_exists(vec3i pos, int direction) {
    if (direction == PLUS_X) {
        return pos.x < CHUNK_MAX;
    } else if (direction == MINUS_X) {
        return pos.x > 0;
    } else if (direction == PLUS_Y) {
        return pos.y < CHUNK_MAX;
    } else if (direction == MINUS_Y) {
        return pos.y > 0;
    } else if (direction == PLUS_Z) {
        return pos.z < CHUNK_MAX;
    } else if (direction == MINUS_Z) {
        return pos.z > 0;
    }
    printf("shouldnt happen, direction %d\n", direction);
    exit(1);
    return false; // shouldnt happen
}

chunk chunk_generate(chunk_rngs noise, int x, int y, int z) {
    block_tag *blocks = calloc(sizeof(block_tag), CHUNK_RADIX_3);
    uint8_t *block_light = calloc(sizeof(uint8_t), CHUNK_RADIX_3);
    uint8_t *sky_light = calloc(sizeof(uint8_t), CHUNK_RADIX_3);
    chunk c = {0};
    c.blocks = blocks;
    c.sky_light_levels = sky_light;
    c.block_light_levels = block_light;

    c.key = (vec3i) {x,y,z};

    float chunk_x = x*CHUNK_RADIX;
    float chunk_y = y*CHUNK_RADIX;
    float chunk_z = z*CHUNK_RADIX;

    for (int idx = 0; idx < CHUNK_RADIX_3; idx++) {
        vec3i block_pos = chunk_1d_to_3d(idx);

        double block_x = chunk_x + block_pos.x;
        double block_y = chunk_y + block_pos.y;
        double block_z = chunk_z + block_pos.z;

        float height_low = n2d_sample(&noise.noise_lf_heightmap, block_x, block_z);
        float height_high = n2d_sample(&noise.noise_hf_heightmap, block_x, block_z);
        float cliff_carve_density = n3d_sample(&noise.noise_cliff_carver, block_x, block_y, block_z);
        float cave_carve_density = n3d_sample(&noise.noise_cliff_carver, block_x, block_y, block_z);

        float cave_cutoff = remap(64, -80, -10, 2, block_y);

        if (cave_carve_density < cave_cutoff) {
            blocks[idx] = BLOCK_AIR;
            continue;
        }

        float height;
        if (cliff_carve_density < 1) {
            height = height_low;
        } else {
            height = height_low + height_high;
        }

        // grass and stuff
        if (block_y < height - 4) {
            blocks[idx] = BLOCK_STONE;
        } else if (block_y < height - 0.5) {
            // not stone layers
            if (height > -25) {
                blocks[idx] = BLOCK_DIRT;
            } else {
                blocks[idx] = BLOCK_SAND;
            }
        } else if (block_y < height + 0.5) {
            // top layer
            if (height > 40) {
                blocks[idx] = BLOCK_SNOW;    
            } else if (height > -25) {
                blocks[idx] = BLOCK_GRASS;
            } else {
                blocks[idx] = BLOCK_SAND;
            }
        } else {
            blocks[idx] = BLOCK_AIR;
            sky_light[idx] = SKY_LIGHT_FULL;
            // we could fix up the lighting from here - have an array that remembers the lowest sky light
            // maybe if we iterate from high to low

        }
    }

    return c;
}

void chunk_test() {
    assert_int_equal("0 corner", 0, chunk_3d_to_1d((vec3i){0,0,0}));
    assert_int_equal("r3 corner", CHUNK_MAX_3 + CHUNK_MAX_2 + CHUNK_MAX, chunk_3d_to_1d((vec3i){CHUNK_MAX,CHUNK_MAX,CHUNK_MAX}));
    assert_int_equal("r1 corner", CHUNK_MAX, chunk_3d_to_1d((vec3i){0, 0,CHUNK_MAX}));
    assert_int_equal("r11 corner", CHUNK_MAX + 1, chunk_3d_to_1d((vec3i){0, 1, 0}));
    assert_int_equal("r2 corner", CHUNK_MAX_2, chunk_3d_to_1d((vec3i){0,CHUNK_MAX,0}));
    assert_int_equal("r2+1 corner", CHUNK_MAX*CHUNK_RADIX + CHUNK_MAX, chunk_3d_to_1d((vec3i){0,CHUNK_MAX,CHUNK_MAX}));
    assert_int_equal("r2+1+1 corner", CHUNK_RADIX_2 + CHUNK_MAX_2 + CHUNK_MAX, chunk_3d_to_1d((vec3i){1,CHUNK_MAX,CHUNK_MAX}));

    vec3i pos3d = chunk_1d_to_3d(0);
    assert_int_equal("0x", 0, pos3d.x);
    assert_int_equal("0y", 0, pos3d.y);
    assert_int_equal("0z", 0, pos3d.z);

    pos3d = chunk_1d_to_3d(CHUNK_MAX);
    assert_int_equal("rx", 0, pos3d.x);
    assert_int_equal("ry", 0, pos3d.y);
    assert_int_equal("rz", CHUNK_MAX, pos3d.z);

    pos3d = chunk_1d_to_3d(CHUNK_RADIX * CHUNK_MAX);
    assert_int_equal("2rx", 0, pos3d.x);
    assert_int_equal("2ry", CHUNK_MAX, pos3d.y);
    assert_int_equal("2rz", 0, pos3d.z);

    pos3d = chunk_1d_to_3d(CHUNK_RADIX_2 * CHUNK_MAX);
    assert_int_equal("3rx", CHUNK_MAX, pos3d.x);
    assert_int_equal("3ry", 0, pos3d.y);
    assert_int_equal("3rz", 0, pos3d.z);

    vec3i pos = (vec3i) {5,5,5};
    printf("idx %d (%d %d %d)\n", CHUNK_RADIX_2 + CHUNK_RADIX + 4, pos3d.x, pos3d.y, pos3d.z);
    assert_int_equal("neighbours middle nx", 1, neighbour_exists(pos, PLUS_X));
    assert_int_equal("neighbours middle ny", 1, neighbour_exists(pos, MINUS_Y));
    assert_int_equal("neighbours middle nz", 1, neighbour_exists(pos, MINUS_Z));
    assert_int_equal("neighbours middle px", 1, neighbour_exists(pos, PLUS_X));
    assert_int_equal("neighbours middle py", 1, neighbour_exists(pos, PLUS_Y));
    assert_int_equal("neighbours middle pz", 1, neighbour_exists(pos, PLUS_Z));

    assert_int_equal("neighbours edge nx", 0, neighbour_exists((vec3i) {0, 3, 3}, MINUS_X));
    assert_int_equal("neighbours edge enx", 1, neighbour_exists((vec3i) {0, 3, 3}, PLUS_X));
    assert_int_equal("neighbours edge px", 0, neighbour_exists((vec3i) {CHUNK_MAX, 3, 3}, PLUS_X));    
    assert_int_equal("neighbours edge epx", 1, neighbour_exists((vec3i) {CHUNK_MAX, 3, 3}, MINUS_X));    
    assert_int_equal("neighbours edge ny", 0, neighbour_exists((vec3i) {3, 0, 3}, MINUS_Y));
    assert_int_equal("neighbours edge eny", 1, neighbour_exists((vec3i) {3, 0, 3}, PLUS_Y));
    assert_int_equal("neighbours edge py", 0, neighbour_exists((vec3i) {3, CHUNK_MAX, 3}, PLUS_Y));
    assert_int_equal("neighbours edge epy", 1, neighbour_exists((vec3i) {3, CHUNK_MAX, 3}, MINUS_Y));
    assert_int_equal("neighbours edge nz", 0, neighbour_exists((vec3i) {3, 3, 0}, MINUS_Z));
    assert_int_equal("neighbours edge enz", 1, neighbour_exists((vec3i) {3, 3, 0}, PLUS_Z));
    assert_int_equal("neighbours edge pz", 0, neighbour_exists((vec3i) {3, 3, CHUNK_MAX}, PLUS_Z));
    assert_int_equal("neighbours edge epz", 1, neighbour_exists((vec3i) {3, 3, CHUNK_MAX}, MINUS_Z));
}