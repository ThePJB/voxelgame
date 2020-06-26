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
