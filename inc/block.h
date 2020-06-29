#ifndef BLOCK_H
#define BLOCK_H

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
    // BLOCK_UNKNOWN, todo use this, and look up block opacity
    NUM_BLOCKS,
} block_tag;

// todo pack
typedef struct {
    block_tag tag;
} block;



#endif