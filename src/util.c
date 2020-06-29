#include <stdio.h>

int signum(float x) {
    if (x > 0) {
        return 1;
    } else {
        return -1;
    }
}

int mod(int val, int modulus) {
    // ok i think this is because its fake modulus (divisor)
    return (val % modulus + modulus) % modulus;
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