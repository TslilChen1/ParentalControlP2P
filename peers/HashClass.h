#pragma once
#include <cstdint>
#include <string>

class HashClass
{   
public:
    static uint64_t computeSecureHash(const std::string& message);

private:
    // Bit rotation helper function
    static uint64_t rotateLeft(uint64_t x, int k);

    // Optional: Bit mixing function for additional complexity
    static uint64_t mixBits(uint64_t x);
};

