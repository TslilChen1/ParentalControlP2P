#include "KeyUse.h"


std::vector<uint8_t> KeyUse::encrypt(const std::vector<uint8_t>& message,
                                    const std::vector<uint8_t>& key,
                                    const std::vector<uint8_t>& iv) {

    if (key.empty()) {
        throw std::invalid_argument("Key cannot be empty");
    }

    std::vector<uint8_t> result = message;

    // Apply IV to first block
    for (size_t i = 0; i < iv.size() && i < result.size(); i++) {
        result[i] ^= iv[i];
    }

    // XOR with key (repeating if key is shorter than message)
    for (size_t i = 0; i < result.size(); i++) {
        result[i] ^= key[i % key.size()];
    }

    return result;

}

std::vector<uint8_t> KeyUse::decrypt(const std::vector<uint8_t>& encrypted,
                                    const std::vector<uint8_t>& key,
                                    const std::vector<uint8_t>& iv) {
    if (key.empty()) {
        throw std::invalid_argument("Key cannot be empty");
    }

    std::vector<uint8_t> result = encrypted;

    // XOR with key (repeating if key is shorter than encrypted)
    for (size_t i = 0; i < result.size(); i++) {
        result[i] ^= key[i % key.size()];
    }

    // Apply IV to first block
    for (size_t i = 0; i < iv.size() && i < result.size(); i++) {
        result[i] ^= iv[i];
    }

    return result;
}

std::vector<uint8_t> KeyUse::sign(const std::vector<uint8_t>& message,
                                 const std::vector<uint8_t>& mac_key) {
    std::vector<uint8_t> mac(32, 0);
    
    for(size_t i = 0; i < message.size(); ++i) {
        mac[i % 32] ^= message[i] ^ mac_key[i % mac_key.size()];
        
        for(size_t j = 0; j < 32; ++j) {
            mac[j] = (mac[j] + mac_key[(i+j) % mac_key.size()]) % 256;
        }
    }
    
    return mac;
}

bool KeyUse::verify(const std::vector<uint8_t>& message,
                    const std::vector<uint8_t>& received_mac,
                    const std::vector<uint8_t>& mac_key) {
    auto calculated_mac = sign(message, mac_key);
    return calculated_mac == received_mac;
}

KeyUse::Keys KeyUse::extractServerKeys(const std::vector<std::vector<uint8_t>>& keys)
{
    KeyUse::Keys result;
    // Server keys (for receiving)
    result.mac_key = keys[1];      // Server MAC Key
    result.encryption_key = keys[3];// Server Encryption Key
    result.iv = keys[5];           // Server IV
    return result;
       
}
KeyUse::Keys KeyUse::extractClientKeys(const std::vector<std::vector<uint8_t>>& keys)
{
    KeyUse::Keys result;

    // Client keys
    result.mac_key = keys[0];        // Client MAC Key
    result.encryption_key = keys[2];  // Client Encryption Key
    result.iv = keys[4];             // Client IV


    return result;
}
