#include "noise.h"
#include "simplex.h"

noise2d create_noise2d(int64_t seed, int octaves, double start_freq, double freq_coefficient, double start_amplitude, double amplitude_coefficient) {
    noise2d ret = (noise2d) {
        .seed = seed,
        .octaves = octaves,
        .start_amplitude = start_amplitude,
        .amplitude_coefficient = amplitude_coefficient,
        .start_freq = start_freq,
        .freq_coefficient = freq_coefficient,
    };

    open_simplex_noise(ret.seed, &ret.osn);

    return ret;
}

void reseed(noise2d *n, int64_t seed) {
    n->seed = seed;
    open_simplex_noise(n->seed, &n->osn);
}

double sample(noise2d *n, double x, double y) {
    double A = n->start_amplitude;
    double f = n->start_freq;
    double acc = 0;

    const double vec = 539847; // to get unrelated noise for each octave

    for (int i = 0; i < n->octaves; i++) {
        acc += A*open_simplex_noise2(n->osn, i*vec+f*x, i*vec+f*y);
        f *= n->freq_coefficient;
        A *= n->amplitude_coefficient;
    }

    return acc;
}