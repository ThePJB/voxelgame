#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <cglm/struct.h>

#include "util.h"

//#define STB_DEFINE
//#include "stb.h"

vec3i unit_vec3i[NUM_DIRS] = {
    (vec3i) {1,0,0},
    (vec3i) {-1,0,0},
    (vec3i) {0,1,0},
    (vec3i) {0,-1,0},
    (vec3i) {0,0,1},
    (vec3i) {0,0,-1},
};

vec3l unit_vec3l[NUM_DIRS] = {
    (vec3l) {1,0,0},
    (vec3l) {-1,0,0},
    (vec3l) {0,1,0},
    (vec3l) {0,-1,0},
    (vec3l) {0,0,1},
    (vec3l) {0,0,-1},
};

vec3s unit_vec3s[NUM_DIRS] = {
    (vec3s) {1,0,0},
    (vec3s) {-1,0,0},
    (vec3s) {0,1,0},
    (vec3s) {0,-1,0},
    (vec3s) {0,0,1},
    (vec3s) {0,0,-1},
};

char *dir_name[NUM_DIRS] = {
    "+X",
    "-X",
    "+Y",
    "-Y",
    "+Z",
    "-Z",
};

int floor_div(int a, int b) {
    int d = a / b;
    return d * b == a ? d : d - ((a < 0) ^ (b < 0));
}

int32_t fast_floorf(float x) {
    return (int)x - (x < (int)x);
}

int32_t fast_floord(double x) {
    return (int)x - (x < (int)x);
}

int signum(float x) {
    // return 2*(x>0)-1
    if (x > 0) {
        return 1;
    } else {
        return -1;
    }
}

float min(float a, float b) { return a < b? a : b; }
float max(float a, float b) { return a > b? a : b; }

void print_vec3i(vec3i a) {
    printf("{%d %d %d}", spread(a));
}

#define vec3i_binary_op(OP) .x = a.x OP b.x, .y = a.y OP b.y, .z = a.z OP b.z,

vec3i vec3i_add(vec3i a, vec3i b) { return (vec3i) {vec3i_binary_op(+)}; }
vec3i vec3i_sub(vec3i a, vec3i b) { return (vec3i) {vec3i_binary_op(-)}; }

#define vec3i_num_binary_op(OP) .x = a.x OP b, .y = a.y OP b, .z = a.z OP b,

vec3i vec3i_mul(vec3i a, int b) { return (vec3i) {vec3i_num_binary_op(*)}; }
vec3i vec3i_div(vec3i a, int b) { return (vec3i) {vec3i_num_binary_op(/)}; }

#define vec3l_binary_op(OP) .x = a.x OP b.x, .y = a.y OP b.y, .z = a.z OP b.z,

vec3l vec3l_add(vec3l a, vec3l b) { return (vec3l) {vec3l_binary_op(+)}; }
vec3l vec3l_sub(vec3l a, vec3l b) { return (vec3l) {vec3l_binary_op(-)}; }

#define vec3l_num_binary_op(OP) .x = a.x OP b, .y = a.y OP b, .z = a.z OP b,

vec3l vec3l_mul(vec3l a, int b) { return (vec3l) {vec3l_num_binary_op(*)}; }
vec3l vec3l_div(vec3l a, int b) { return (vec3l) {vec3l_num_binary_op(/)}; }

int mod(int val, int modulus) {
    // ok i think this is because its fake modulus (divisor)
    return (val % modulus + modulus) % modulus;
}

const float epsilion = 0.000001;
bool fequals(float a, float b) {
    if (a == b) return true;

    if (fabs(a - b) < epsilion) {
        return true;
    }
    return false;
}

// todo make it say true false
void assert_bool_equal(char *desc, bool a, bool b) {
    if (a == b) {
        printf("\033[032m");
    } else {
        printf("\033[031m");
    }
    printf("%s \t", desc);
    if (a == b) {
        printf("%d == %d \t -- \t pass", a, b);
    } else {
        printf("%d != %d \t -- \t fail", a, b);
    }
    printf("\n\033[037m");
}

void assert_int_equal(char *desc, int a, int b) {
    if (a == b) {
        printf("\033[032m");
    } else {
        printf("\033[031m");
    }
    printf("%s \t", desc);
    if (a == b) {
        printf("%d == %d \t -- \t pass", a, b);
    } else {
        printf("%d != %d \t -- \t fail", a, b);
    }
    printf("\n\033[037m");
}

void assert_vec3i_equal(char *desc, vec3i a, int bx, int by, int bz) {
    if (a.x == bx && a.y == by && a.z == bz) {
        printf("\033[032m");
    } else {
        printf("\033[031m");
    }
    printf("%s \t", desc);
    if (a.x == bx && a.y == by && a.z == bz) {
        printf("{%d %d %d} == {%d %d %d}\t -- \t pass", a.x, a.y, a.z, bx, by, bz);
    } else {
        printf("{%d %d %d} != {%d %d %d}\t -- \t pass", a.x, a.y, a.z, bx, by, bz);        
    }
    printf("\n\033[037m");
}

void assert_float_equal(char *desc, float a, float b) {
    if (fequals(a,b)) {
        printf("\033[032m");
    } else {
        printf("\033[031m");
    }
    printf("%s \t", desc);
    if (fequals(a,b)) {
        printf("%f == %f \t -- \t pass", a, b);
    } else {
        printf("%f != %f \t -- \t fail", a, b);
    }
    printf("\n\033[037m");
}


typedef struct {
    unsigned long size,resident,share,text,lib,data,dt;
} statm_t;

#define PAGESIZE 4096

void read_off_memory_status(statm_t *result)
{
  unsigned long dummy;
  const char* statm_path = "/proc/self/statm";

  FILE *f = fopen(statm_path,"r");
  if(!f){
    perror(statm_path);
    return;
  }
  if(7 != fscanf(f,"%ld %ld %ld %ld %ld %ld %ld",
    &result->size, &result->resident,&result->share,&result->text,&result->lib,&result->data,&result->dt))
  {
    perror(statm_path);
    return;
  }
  fclose(f);
}

uint32_t get_ram_usage() {
    statm_t res = {0};
    read_off_memory_status(&res);
    return PAGESIZE * res.size;
}

float lerp(float a, float b, float t) {
    return a*(1-t) + b*t;
}

float unlerp(float a, float b, float t) {
    return (t - a) / (b - a);
}

float remap(float prev_lower, float prev_upper, float new_lower, float new_upper, float a) {
    return lerp(new_lower, new_upper, unlerp(prev_lower, prev_upper, a));
}

#define PUSHFN(T) void T##_queue_push(T##_queue *q, T item) {\
    q->items[q->end] = item;\
    q->end = (q->end + 1) % q->size;}

#define POPFN(T) T T##_queue_pop(T##_queue *q) {\
    T item = q ->items[q->start];\
    q->start = (q->start + 1) % q->size;\
    return item;}

#define LENFN(T) unsigned int T##_queue_len(T##_queue *q) {\
    return (q->end - q->start) + q->size * (q->end < q->start);}

#define DEFINE_QUEUE(T) QUEUE_STRUCT(T) PUSHFN(T) POPFN(T) LENFN(T)

//QUEUE_STRUCT(uint8_t)
PUSHFN(uint8_t)
POPFN(uint8_t)
LENFN(uint8_t)
//DEFINE_QUEUE(uint8_t)


void vec3i_queue_push(vec3i_queue *vq, vec3i item) {
    vq->items[vq->end] = item;
    vq->end = (vq->end + 1) % vq->size;
}

vec3i vec3i_queue_pop(vec3i_queue *vq) {
    vec3i item = vq->items[vq->start];
    vq->start = (vq->start + 1) % vq->size;
    return item;
}

int vec3i_queue_len(vec3i_queue *vq) {
    int size_tentative = vq->end - vq->start;
    if (size_tentative >= 0) {
        return size_tentative;
    } else {
        return size_tentative + vq->size;
    }
}

vec3s vec3i_to_vec3s(vec3i a) {
    return (vec3s) {spread(a)};
}

vec3l vec3s_to_vec3l(vec3s a) {
    return (vec3l) {
        fast_floorf(a.x),
        fast_floorf(a.y),
        fast_floorf(a.z),
    };
}

void vec3l_queue_push(vec3l_queue *vq, vec3l item) {
    vq->items[vq->end] = item;
    vq->end = (vq->end + 1) % vq->size;
}

vec3l vec3l_queue_pop(vec3l_queue *vq) {
    vec3l item = vq->items[vq->start];
    vq->start = (vq->start + 1) % vq->size;
    return item;
}

int vec3l_queue_len(vec3l_queue *vq) {
    int size_tentative = vq->end - vq->start;
    if (size_tentative >= 0) {
        return size_tentative;
    } else {
        return size_tentative + vq->size;
    }
}

void util_srand(unsigned int seed) {
    //stb_srand(seed);
    srand(seed);
}

int util_rand_intn(int min, int max) {
    return rand() % (max-min) + min;
    //return stb_rand() % (max-min) + min;
}

float util_rand_floatn(float min, float max) {
    return rand() / (double)RAND_MAX * (max-min) + min;
    //return stb_frand() * (max-min) + min;
}

void test_util() {
    // fequals
    assert_bool_equal("feq 0.4 0.4", fequals(0.4, 0.4), true);
    assert_bool_equal("feq 0.5 0.5", fequals(0.5, 0.5), true);
    assert_bool_equal("feq 0.8 0.8", fequals(0.8, 0.8), true);
    assert_bool_equal("feq 0.8 big", fequals(0.8, 123098.34234), false);
    assert_bool_equal("feq 0.81 0.8", fequals(0.81, 0.8), false);    

    // vec3i
    assert_vec3i_equal("123 + 456 = 579", vec3i_add((vec3i){1,2,3}, (vec3i){4,5,6}), 5,7,9);
    assert_vec3i_equal("123 - 456 = -3-3-3", vec3i_sub((vec3i){1,2,3}, (vec3i){4,5,6}), -3,-3,-3);
    assert_vec3i_equal("123 * 2 = 246", vec3i_mul((vec3i){1,2,3}, 2), 2,4,6);
    assert_vec3i_equal("123 / 2 = 011", vec3i_div((vec3i){1,2,3}, 2), 0,1,1);

    // vec3l queue
    vec3l buf[400];
    vec3l_queue vq = {
        .size = 400,
        .items = buf,
        .start = 0,
        .end = 0,
    };
    assert_int_equal("empty queue len", 0, vec3l_queue_len(&vq));
    vec3l_queue_push(&vq, (vec3l){1,0,0});
    assert_int_equal("1item queue len", 1, vec3l_queue_len(&vq));
    assert_int_equal("1item start", 0, vq.start);
    assert_int_equal("1item end", 1, vq.end);
    vec3l a = vec3l_queue_pop(&vq);
    assert_int_equal("empty queue len again", 0, vec3l_queue_len(&vq));
    assert_int_equal("emptyagain start", 1, vq.start);
    assert_int_equal("emptyagain end", 1, vq.end);

    for (int i = 0; i < 390; i++) {
        vec3l_queue_push(&vq, (vec3l){i,0,i});
    }
    assert_int_equal("390 queue len", 390, vec3l_queue_len(&vq));

    for (int i = 0; i < 270; i++) {
        vec3l_queue_pop(&vq);
    }
    assert_int_equal("120 queue len", 120, vec3l_queue_len(&vq));

    for (int i = 0; i < 200; i++) {
        vec3l_queue_push(&vq, (vec3l) {0, i, 0});
    }
    assert_int_equal("320 queue len", 320, vec3l_queue_len(&vq));

    assert_float_equal("unlerp1", unlerp(180, 192, 186), 0.5);
    assert_float_equal("unlerp2", unlerp(10, -10, 0), 0.5);
    assert_float_equal("unlerp3", unlerp(10, -30, 0), 0.25);
    assert_float_equal("lerp1", lerp(10, 20, 0), 10);
    assert_float_equal("lerp2", lerp(10, 20, 0.5), 15);
    assert_float_equal("lerp3", lerp(10, 20, 1), 20);
    assert_float_equal("remap twilight 1", remap(180, 200, 0.5, 1, 190), 0.75);

}