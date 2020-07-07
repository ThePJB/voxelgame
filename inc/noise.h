#ifndef NOISE_H
#define NOISE_H

#include <stdint.h>
#include "simplex.h"

typedef struct {
    int64_t seed;
    int octaves;
    double start_freq;
    double start_amplitude;
    double freq_coefficient;
    double amplitude_coefficient;

    struct osn_context* osn;
} noise2d;

noise2d create_noise2d(int64_t seed, int octaves, double start_freq, double freq_coefficient, double start_amplitude, double amplitude_coefficient);
void reseed(noise2d *n, int64_t seed);
double sample(noise2d *n, double x, double y);

#endif