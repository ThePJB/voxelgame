#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>
#include <cglm/struct.h>



// ------------------------ math
#define RAD_TO_DEG 57.295779513082320876798154814105f
#define DEG_TO_RAD 0.017453292519943295769236907684886f

int floor_div(int a, int b);
int32_t fast_floorf(float x);
int32_t fast_floord(double x);
#define min(A,B) (A < B ? A : B)
#define max(A,B) (A > B ? A : B)

//float min(float a, float b);
//float max(float a, float b);
int mod(int val, int modulus);
int signum(float x);
bool fequals(float a, float b);
float lerp(float a, float b, float t);
float unlerp(float a, float b, float t);
float remap(float prev_lower, float prev_upper, float new_lower, float new_upper, float a);

// ------------------------ random numbers
void util_srand(unsigned int);
int util_rand_intn(int min, int max);
float util_rand_floatn(float min, float max);



// ------------------------- vectors
typedef struct {
    int32_t x;
    int32_t y;
    int32_t z;
} vec3l;

typedef struct {
    int x;
    int y;
    int z;
} vec3i;

vec3i from_vec3s(vec3s a);
vec3i vec3i_add(vec3i a, vec3i b);
vec3i vec3i_sub(vec3i a, vec3i b);
vec3i vec3i_mul(vec3i a, int b);
vec3i vec3i_div(vec3i a, int b);
void print_vec3i(vec3i a);

vec3l vec3l_add(vec3l a, vec3l b);
vec3l vec3l_sub(vec3l a, vec3l b);
vec3l vec3l_mul(vec3l a, int b);
vec3l vec3l_div(vec3l a, int b);
void print_vec3l(vec3l a);

vec3s vec3i_to_vec3s(vec3i a);
vec3l vec3s_to_vec3l(vec3s a);

#define spread(X) X.x, X.y, X.z

// ------------------------ directions
typedef enum {
    DIR_PX,
    DIR_MX,
    DIR_PY,
    DIR_MY,
    DIR_PZ,
    DIR_MZ,
    NUM_DIRS,
} direction;

static vec3i unit_vec3i[NUM_DIRS] = {
    (vec3i) {1,0,0},
    (vec3i) {-1,0,0},
    (vec3i) {0,1,0},
    (vec3i) {0,-1,0},
    (vec3i) {0,0,1},
    (vec3i) {0,0,-1},
};

static vec3l unit_vec3l[NUM_DIRS] = {
    (vec3l) {1,0,0},
    (vec3l) {-1,0,0},
    (vec3l) {0,1,0},
    (vec3l) {0,-1,0},
    (vec3l) {0,0,1},
    (vec3l) {0,0,-1},
};

static vec3s unit_vec3s[NUM_DIRS] = {
    (vec3s) {1,0,0},
    (vec3s) {-1,0,0},
    (vec3s) {0,1,0},
    (vec3s) {0,-1,0},
    (vec3s) {0,0,1},
    (vec3s) {0,0,-1},
};

static char *dir_name[NUM_DIRS] = {
    "+X",
    "-X",
    "+Y",
    "-Y",
    "+Z",
    "-Z",
};



// -------------------------- data structures
typedef struct {
    vec3i *items;
    unsigned int start;
    unsigned int end;
    unsigned int size;
} vec3i_queue;

void vec3i_queue_push(vec3i_queue *vq, vec3i item);
vec3i vec3i_queue_pop(vec3i_queue *vq);
int vec3i_queue_len(vec3i_queue *vq);

typedef struct {
    vec3l *items;
    uint32_t start;
    uint32_t end;
    uint32_t size;
} vec3l_queue;

void vec3l_queue_push(vec3l_queue *vq, vec3l item);
vec3l vec3l_queue_pop(vec3l_queue *vq);
int vec3l_queue_len(vec3l_queue *vq);



#define QUEUE_STRUCT(T) typedef struct {T *items; unsigned int start; unsigned int end; unsigned int size;} T##_queue;

QUEUE_STRUCT(uint8_t)

#define MKPAIR(T) typedef struct {T l; T r;}T##_pair
MKPAIR(vec3l);
MKPAIR(vec3i);
MKPAIR(int);
MKPAIR(int32_t);
MKPAIR(float);

#define MKMAYBE(T) typedef struct {T value; bool ok;} maybe_##T
MKMAYBE(int32_t);
MKMAYBE(uint8_t);

// ------------------------- debug output
extern bool enable_debug;

#define debugf(...) if(enable_debug) {printf("%s:%d:",__FILE__,__LINE__); printf(__VA_ARGS__);}

#define panicf(...) printf("%s:%d: panic -- ", __FILE__, __LINE__); printf(__VA_ARGS__); exit(1)
//#define unwrap(X) X.value; if (!X.ok) panic("failed unwrapping maybe")

// ------------------------- unit tests
void assert_int_equal(char *desc, int a, int b);
void assert_float_equal(char *desc, float a, float b);
void assert_vec3i_equal(char *desc, vec3i a, int bx, int by, int bz);
void assert_bool_equal(char *desc, bool a, bool b);


// -------------------------- other
uint32_t get_ram_usage();

void test_util();

#endif