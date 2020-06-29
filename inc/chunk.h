#ifndef CHUNK_H
#define CHUNK_H

#include <stdbool.h>
#include "simplex.h"
#include "block.h"
#include "util.h"

/*
    In this file we only deal with individual chunks
*/

#define CHUNK_RADIX 16
#define CHUNK_RADIX_2 (CHUNK_RADIX*CHUNK_RADIX)
#define CHUNK_RADIX_3 (CHUNK_RADIX*CHUNK_RADIX*CHUNK_RADIX)

// worth having these because they are used a lot
// these correspond to max in any direction
#define CHUNK_MAX (CHUNK_RADIX-1)
#define CHUNK_MAX_2 (CHUNK_RADIX * CHUNK_MAX)
#define CHUNK_MAX_3 (CHUNK_RADIX * CHUNK_MAX_2)


// The big block of data
typedef struct {
    block blocks[CHUNK_RADIX_3];
} chunk_blocks;

// Information that the chunk cares about (would be saved)
typedef struct {
    int x;
    int y;
    int z;
    bool empty; // also all_one_block is a possibility, not sure how applicable that is
    chunk_blocks *blocks;
} chunk;

void check_chunk_invariants(chunk c);

// umm so you would go "whats your +x neighbour" and it would return either the block or if there
// is an edge

// return the chunk generated
chunk generate_chunk(struct osn_context *ctx, int x, int y, int z);

// put vertex data into buffer and return number of triangles
int get_chunk_vertex_data(chunk c, float *buf);

int arr_3d_to_1d(vec3i pos);
vec3i arr_1d_to_3d(int idx);
void test_chunk();
#endif