#include "HashClass.h"

uint64_t HashClass::computeSecureHash(const std::string& message)
{
    // Initial prime-based constants
    uint64_t hash = 0x6A09E667F3BCC908ULL; // First 64 bits of fractional parts of square roots of first 8 primes
    const uint64_t PRIME1 = 0x428A2F98D728AE22ULL;
    const uint64_t PRIME2 = 0x7137449123EF65CDULL;
    const uint64_t PRIME3 = 0xB5C0FBCFEC4D3B2FULL;

    // Multi-round processing
    for (char c : message) {
        // Bitwise mixing operations
        hash ^= static_cast<uint64_t>(c);
        hash *= PRIME1;

        // Rotate bits for additional mixing
        hash = rotateLeft(hash, 17);

        // Non-linear transformation
        hash ^= (hash >> 33);
        hash *= PRIME2;
        hash ^= (hash >> 29);
        hash *= PRIME3;
    }

    // Additional finalization steps
    hash ^= (hash >> 31);
    hash *= PRIME1;
    hash ^= (hash >> 33);
    return hash;

}

uint64_t HashClass::rotateLeft(uint64_t x, int k)
{
	return (x << k) | (x >> (64 - k));
}

uint64_t HashClass::mixBits(uint64_t x)
{
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return x;
}
