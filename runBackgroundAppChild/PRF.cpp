#include "PRF.h"

std::vector<uint8_t> PRF::uint64ToByteVector(uint64_t value) {
    std::vector<uint8_t> bytes(8);
    for (int i = 7; i >= 0; --i) {
        bytes[i] = value & 0xFF;
        value >>= 8;
    }
    return bytes;
}

std::vector<uint8_t> PRF::sha256(const std::vector<uint8_t>& data) {
    // Placeholder SHA-256 simulation
    std::vector<uint8_t> hash(32, 0);
    uint64_t seed = 0;

    // Simple non-cryptographic hash generation
    for (auto byte : data) {
        seed = seed * 31 + byte;
    }

    // Distribute seed bytes across hash
    for (int i = 0; i < 32; ++i) {
        hash[i] = (seed >> (i * 8)) & 0xFF;
    }

    return hash;
}

std::vector<uint8_t> PRF::hmac(
    const std::vector<uint8_t>& key,
    const std::vector<uint8_t>& data
) {
    // XOR key with inner and outer pad
    std::vector<uint8_t> inner_key = key;
    std::vector<uint8_t> outer_key = key;

    // Pad keys if needed
    while (inner_key.size() < 64) inner_key.push_back(0);
    while (outer_key.size() < 64) outer_key.push_back(0);

    // XOR with pad values
    for (int i = 0; i < 64; ++i) {
        inner_key[i] ^= 0x36;
        outer_key[i] ^= 0x5C;
    }

    // Combine inner key and data
    std::vector<uint8_t> inner_message = inner_key;
    inner_message.insert(inner_message.end(), data.begin(), data.end());

    // First hash
    std::vector<uint8_t> inner_hash = sha256(inner_message);

    // Combine outer key and first hash
    std::vector<uint8_t> outer_message = outer_key;
    outer_message.insert(outer_message.end(), inner_hash.begin(), inner_hash.end());

    // Final hash
    return sha256(outer_message);
}

std::vector<uint8_t> PRF::generateMasterSecret(
    uint64_t premasterSecret,
    uint64_t clientRandom,
    uint64_t serverRandom
) {
    // Convert inputs to byte vectors
    std::vector<uint8_t> premasterBytes = uint64ToByteVector(premasterSecret);
    std::vector<uint8_t> clientRandomBytes = uint64ToByteVector(clientRandom);
    std::vector<uint8_t> serverRandomBytes = uint64ToByteVector(serverRandom);

    // Combine random inputs
    std::vector<uint8_t> seed = concatenateVectors(clientRandomBytes, serverRandomBytes);

    // Label for key derivation
    std::string label = "master secret";
    std::vector<uint8_t> labelBytes(label.begin(), label.end());

    // Seed = label + combined randoms
    std::vector<uint8_t> fullSeed = concatenateVectors(labelBytes, seed);

    // Generate 48-byte Master Secret
    return PRF_KeyExpansion(premasterBytes, fullSeed, 48);
}

std::vector<std::vector<uint8_t>> PRF::deriveSessionKeys(
    const std::vector<uint8_t>& masterSecret,
    uint64_t clientRandom,
    uint64_t serverRandom
) {
    // Convert randoms to byte vectors
    std::vector<uint8_t> clientRandomBytes = uint64ToByteVector(clientRandom);
    std::vector<uint8_t> serverRandomBytes = uint64ToByteVector(serverRandom);

    std::string label = "key expansion";
    std::vector<uint8_t> labelBytes(label.begin(), label.end());

    // Seed = label + server random + client random
    std::vector<uint8_t> seed = concatenateVectors(
        labelBytes,
        concatenateVectors(serverRandomBytes, clientRandomBytes)
    );

    // Derive multiple keys
    std::vector<std::vector<uint8_t>> sessionKeys;

    // Derive 4 keys of 16 bytes each
    for (int i = 0; i < 4; ++i) {
        sessionKeys.push_back(PRF_KeyExpansion(masterSecret, seed, 16));
    }

    return sessionKeys;
}

std::vector<uint8_t> PRF::PRF_KeyExpansion(
    const std::vector<uint8_t>& secret,
    const std::vector<uint8_t>& seed,
    size_t outputLength
) {
    std::vector<uint8_t> output;
    output.reserve(outputLength);

    // Initial A value is the seed
    std::vector<uint8_t> A = seed;

    while (output.size() < outputLength) {
        // HMAC of current A value and seed
        std::vector<uint8_t> hmacResult = hmac(secret, A);

        // Concatenate HMAC result to output
        output.insert(
            output.end(),
            hmacResult.begin(),
            hmacResult.begin() + std::min(hmacResult.size(), outputLength - output.size())
        );

        // Update A for next iteration
        A = hmac(secret, A);
    }

    // Truncate to desired length
    output.resize(outputLength);
    return output;
}

std::vector<uint8_t> PRF::concatenateVectors(
    const std::vector<uint8_t>& a,
    const std::vector<uint8_t>& b
) {
    std::vector<uint8_t> result = a;
    result.insert(result.end(), b.begin(), b.end());
    return result;
}

void PRF::printByteVector(const std::vector<uint8_t>& vec, const std::string& name) {
    if (!name.empty()) std::cout << name << ": ";
    for (auto byte : vec) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(byte) << " ";
    }
    std::cout << std::dec << std::endl;
}

void PRF::demonstratePRF() {
    // Simulated inputs as uint64_t
    uint64_t premasterSecret = 0x0102030405060708ULL;
    uint64_t clientRandom = 0x1011121314151617ULL;
    uint64_t serverRandom = 0x2021222324252627ULL;

    // Generate Master Secret
    std::vector<uint8_t> masterSecret = generateMasterSecret(
        premasterSecret, clientRandom, serverRandom
    );
    printByteVector(masterSecret, "Master Secret");

    // Derive Session Keys
    std::vector<std::vector<uint8_t>> sessionKeys = deriveSessionKeys(
        masterSecret, clientRandom, serverRandom
    );

    // Print derived keys
    for (size_t i = 0; i < sessionKeys.size(); ++i) {
        printByteVector(sessionKeys[i], "Session Key " + std::to_string(i + 1));
    }
}

/*
// Main function for demonstration
int main() {
    // Demonstrate PRF functionality
    PRF::demonstratePRF();
    return 0;
}*/

std::vector<std::vector<uint8_t>> PRF::DTLSKeyExpansion(
    const std::vector<uint8_t>& masterSecret,
    uint64_t clientRandom,
    uint64_t serverRandom,
    size_t macKeyLength,
    size_t encKeyLength,
    size_t ivLength)
    {
    // Convert randoms to byte vectors
    std::vector<uint8_t> serverRandomBytes = PRF::uint64ToByteVector(serverRandom);
    std::vector<uint8_t> clientRandomBytes = PRF::uint64ToByteVector(clientRandom);

    // Create seed for key expansion
    std::vector<uint8_t> seed = concatenateVectors(serverRandomBytes, clientRandomBytes);

    // Calculate total key block length
    size_t totalLength = 2 * (macKeyLength + encKeyLength + ivLength);

    // Generate key block
    std::vector<uint8_t> keyBlock = PRF::PRF_KeyExpansion(masterSecret, seed, totalLength);

    // Split key block into individual keys
    std::vector<std::vector<uint8_t>> keys;
    size_t offset = 0;

    // Client MAC key
    keys.push_back(std::vector<uint8_t>(keyBlock.begin() + offset,
        keyBlock.begin() + offset + macKeyLength));
    offset += macKeyLength;

    // Server MAC key
    keys.push_back(std::vector<uint8_t>(keyBlock.begin() + offset,
        keyBlock.begin() + offset + macKeyLength));
    offset += macKeyLength;

    // Client encryption key
    keys.push_back(std::vector<uint8_t>(keyBlock.begin() + offset,
        keyBlock.begin() + offset + encKeyLength));
    offset += encKeyLength;

    // Server encryption key
    keys.push_back(std::vector<uint8_t>(keyBlock.begin() + offset,
        keyBlock.begin() + offset + encKeyLength));
    offset += encKeyLength;

    // Client IV
    keys.push_back(std::vector<uint8_t>(keyBlock.begin() + offset,
        keyBlock.begin() + offset + ivLength));
    offset += ivLength;

    // Server IV
    keys.push_back(std::vector<uint8_t>(keyBlock.begin() + offset,
        keyBlock.begin() + offset + ivLength));

    return keys;
}
