#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>

int signum(float x) {
    if (x > 0) {
        return 1;
    } else {
        return -1;
    }
}

float min(float a, float b) { return a < b? a : b; }
float max(float a, float b) { return a > b? a : b; }

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
void assert_bool(char *desc, bool a, bool b) {
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

unsigned long int get_ram_usage() {
    statm_t res = {0};
    read_off_memory_status(&res);
    return PAGESIZE * res.size;
}

void test_util() {
    // fequals
    assert_bool("feq 0.4 0.4", fequals(0.4, 0.4), true);
    assert_bool("feq 0.5 0.5", fequals(0.5, 0.5), true);
    assert_bool("feq 0.8 0.8", fequals(0.8, 0.8), true);
    assert_bool("feq 0.8 big", fequals(0.8, 123098.34234), false);
    assert_bool("feq 0.81 0.8", fequals(0.81, 0.8), false);    
}