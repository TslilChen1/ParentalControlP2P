#ifndef RSA_H
#define RSA_H

#include "HashClass.h"

#include <iostream>
#include <random>
#include <stdint.h>
#include <cstdio>
#include <ctime>
#include <chrono>


class rsa{
private:
    uint64_t p, q, n, phi, e, d;
    std::random_device rd;
    std::mt19937_64 gen;

    bool miller_rabin(uint64_t n, int k = 5);


    //uint64_t find_prime(uint64_t min = 100, uint64_t max = 0xFFFFFFFF);
    uint64_t find_prime(uint64_t min = 100, uint64_t max = 0xFFFFF);


    uint64_t mod_pow(uint64_t base, uint64_t exp, uint64_t mod);
    uint64_t mod_mul(uint64_t a, uint64_t b, uint64_t mod);

    uint64_t gcd(uint64_t a, uint64_t b);

    uint64_t mod_inverse(uint64_t a, uint64_t m);
public:
    rsa();  // Initialize generator with random seed

    void generate_keys();

    uint64_t encrypt(uint64_t m);

    uint64_t decrypt(uint64_t c);

    std::pair<uint64_t, uint64_t> getPublicKey();

    void setPublicKey(std::pair<uint64_t, uint64_t> publicKey);

    void print_keys();

    uint64_t signMessage(const std::string& message);
    bool verify(uint64_t signature);
};

#endif // RSA_H
