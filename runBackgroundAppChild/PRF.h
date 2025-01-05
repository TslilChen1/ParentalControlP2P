#ifndef PRF_H
#define PRF_H

#include <vector>
#include <string>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstring>

class PRF {
public:
    // Convert uint64_t to byte vector
    static std::vector<uint8_t> uint64ToByteVector(uint64_t value);

    // Main method to generate Master Secret
    static std::vector<uint8_t> generateMasterSecret(
        uint64_t premasterSecret,
        uint64_t clientRandom,
        uint64_t serverRandom
    );

    // Derive session keys from Master Secret
    static std::vector<std::vector<uint8_t>> deriveSessionKeys(
        const std::vector<uint8_t>& masterSecret,
        uint64_t clientRandom,
        uint64_t serverRandom
    );

    // Utility method to print byte vector
    static void printByteVector(
        const std::vector<uint8_t>& vec,
        const std::string& name = ""
    );

    // Demonstration method
    static void demonstratePRF();

    // Simplified PRF key expansion
    static std::vector<uint8_t> PRF_KeyExpansion(
        const std::vector<uint8_t>& secret,
        const std::vector<uint8_t>& seed,
        size_t outputLength
    );

    static std::vector<std::vector<uint8_t>> DTLSKeyExpansion(
        const std::vector<uint8_t>& masterSecret,
        uint64_t clientRandom,
        uint64_t serverRandom,
        size_t macKeyLength,
        size_t encKeyLength,
        size_t ivLength);


    // Simple SHA-256 implementation 
    static std::vector<uint8_t> sha256(const std::vector<uint8_t>& data);

private:

    // Simple HMAC implementation
    static std::vector<uint8_t> hmac(
        const std::vector<uint8_t>& key,
        const std::vector<uint8_t>& data
    );

   

    // Utility method to concatenate vectors
    static std::vector<uint8_t> concatenateVectors(
        const std::vector<uint8_t>& a,
        const std::vector<uint8_t>& b
    );

   
};

#endif // PRF_H