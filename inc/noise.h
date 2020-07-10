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

noise2d n2d_create(int64_t seed, int octaves, double start_freq, double freq_coefficient, double start_amplitude, double amplitude_coefficient);
void n2d_reseed(noise2d *n, int64_t seed);
double n2d_sample(noise2d *n, double x, double y);

typedef struct {
    int64_t seed;
    int octaves;
    double start_freq;
    double start_amplitude;
    double freq_coefficient;
    double amplitude_coefficient;

    struct osn_context* osn;
} noise3d;

noise3d n3d_create(int64_t seed, int octaves, double start_freq, double freq_coefficient, double start_amplitude, double amplitude_coefficient);
void n3d_reseed(noise3d *n, int64_t seed);
double n3d_sample(noise3d *n, double x, double y, double z);

#endif