#ifndef BLOCK_H
#define BLOCK_H

#include <stdbool.h>
#include <stdint.h>

typedef enum __attribute__ ((__packed__)) {
    BLOCK_AIR,
    BLOCK_GRASS,
    BLOCK_DIRT,
    BLOCK_STONE,
    BLOCK_TILE,
    BLOCK_BRICK,
    BLOCK_SNOW,
    BLOCK_SAND,
    BLOCK_PLANKS,
    BLOCK_GEMS,
    // BLOCK_UNKNOWN, todo use this, and look up block opacity
    NUM_BLOCKS,
} block_tag;

typedef struct {
    bool opaque;
    uint8_t luminance;
} block_definition;



#endif