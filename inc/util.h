#ifndef UTIL_H
#define UTIL_H

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

int mod(int val, int modulus);
int signum(float x);
void assert_int_equal(char *desc, int a, int b);

#endif