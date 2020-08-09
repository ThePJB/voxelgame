#include "priority_queue.h"
#include "stb_ds.h"

#define parent(I) ((I-1)/2)
#define left_child(I) ((2*I)+1)
#define right_child(I) ((2*I)+2)

// minheap: parent is lower than children
// todo: maybe we could resize down. but who cares really
vec3i pq_pop(vec3i_priority_queue *pq) {
    vec3i ret = pq->elements[0].v;

    //printf("settin 0 to thing at %d\n", pq->num_items);
    pq->elements[0] = pq->elements[--pq->num_items];
    //pq->num_items--;

    // downheap
    int i = 0;
    while (1) {
        int left_child_idx = left_child(i);
        int right_child_idx = right_child(i);

        bool left_child_exists = left_child_idx < pq->num_items;
        bool right_child_exists = right_child_idx < pq->num_items;


        int lowest_child;

        if (!left_child_exists && !right_child_exists) break;

        if (left_child_exists && !right_child_exists) {
            lowest_child = left_child_idx;
        } else {
            float left_priority = pq->elements[left_child_idx].p;
            float right_priority = pq->elements[right_child_idx].p;
            if (left_priority < right_priority) {
                lowest_child = left_child_idx;
            } else {
                lowest_child = right_child_idx;
            }
        }


        // now swap parent with lowest child
        vec3i_pq_posting tmp = pq->elements[i];
        pq->elements[i] = pq->elements[lowest_child];
        pq->elements[lowest_child] = tmp;

        i = lowest_child;
    }

    return ret;
}


void pq_push(vec3i_priority_queue *pq, vec3i v, float p) {
    vec3i_pq_posting element = {v, p};

    // add an item and see if we need to increase size
    int height_to_grow_at = (1 << pq->height) - 1;
    if (pq->num_items == ((1 << pq->height) - 1)) {
        pq->height++;
        arrsetlen(pq->elements, 1 << (pq->height));
    }
    int i = pq->num_items++;
    pq->elements[i] = element;

    // then do upheap
    while (1) {
        // these conditions mean upheap is complete
        if (i == 0) break;
        if (pq->elements[parent(i)].p < pq->elements[i].p) break;

        // swap child and parent
        vec3i_pq_posting tmp = pq->elements[parent(i)];
        pq->elements[parent(i)] = pq->elements[i];
        pq->elements[i] = tmp;

        i = parent(i);
    }
}

void test_priority_queue() {
    float *test = NULL;
    arrsetlen(test, 10);

    vec3i_priority_queue pq = {0};
    pq_push(&pq, (vec3i){1,2,3}, 1);
    pq_push(&pq, (vec3i){4,5,6}, 2);
    pq_push(&pq, (vec3i){6,5,6}, 20);
    pq_push(&pq, (vec3i){7,7,7}, -10);
    assert_vec3i_equal("first pop", pq_pop(&pq), 7,7,7);
    assert_vec3i_equal("second pop", pq_pop(&pq), 1,2,3);
    assert_vec3i_equal("third pop", pq_pop(&pq), 4,5,6);
    assert_vec3i_equal("fourth pop", pq_pop(&pq), 6,5,6);
}