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
    BLOCK_WATER,
    BLOCK_LOG,
    BLOCK_LEAVES,
    NUM_BLOCKS,
} block_tag;

typedef struct {
    bool opaque;
    uint8_t luminance;
} block_definition;


static block_definition block_defs[NUM_BLOCKS] = {
    {
        .opaque = false,
        .luminance = 0,
    },
    {
        .opaque = true,
        .luminance = 0,
    },
    {
        .opaque = true,
        .luminance = 0,
    },
    {
        .opaque = true,
        .luminance = 0,
    },
    {
        .opaque = true,
        .luminance = 0,
    },
    {
        .opaque = true,
        .luminance = 0,
    },
    {
        .opaque = true,
        .luminance = 0,
    },
    {
        .opaque = true,
        .luminance = 0,
    },
    {
        .opaque = true,
        .luminance = 8,
    },
    {
        .opaque = true,
        .luminance = 6,
    },    
    {
        .opaque = false,
        .luminance = 0,
    },    
    {
        .opaque = true,
        .luminance = 0,
    },
    {
        .opaque = true,
        .luminance = 0,
    },

};

/*
maybe block defs should be, opacity: 
    OPAQUE, TRANSPARENT_PARTIAL, TRANSPARENT_FULL
and pickable
and meshing clusters of these needs to work properly

*/

#endif