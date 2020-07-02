#ifndef UTIL_H
#define UTIL_H

#include <stdbool.h>

typedef struct {
    long int x;
    long int y;
    long int z;
} vec3l;

typedef struct {
    int x;
    int y;
    int z;
} vec3i;

float min(float a, float b);
float max(float a, float b);

int mod(int val, int modulus);
int signum(float x);

void assert_int_equal(char *desc, int a, int b);
void assert_float_equal(char *desc, float a, float b);
void assert_bool(char *desc, bool a, bool b);

bool fequals(float a, float b);

unsigned long int get_ram_usage();

void test_util();

extern bool enable_debug;

#define debugf(...) if(enable_debug) {printf(__VA_ARGS__);}

#endif