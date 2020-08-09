#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include "util.h"

typedef struct {
    vec3i v;
    float p;
} vec3i_pq_posting;

typedef struct {
    vec3i_pq_posting* elements;
    int height;
    int num_items;
} vec3i_priority_queue;

void pq_push(vec3i_priority_queue *pq, vec3i v, float p);
vec3i pq_pop(vec3i_priority_queue *pq);
void test_priority_queue();

#endif