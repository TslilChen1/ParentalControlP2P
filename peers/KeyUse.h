#ifndef KEY_USE_H
#define KEY_USE_H

#include <vector>
#include <cstdint>
#include <algorithm>
#include <stdexcept>


class KeyUse {
public:
    struct Keys {
        std::vector<uint8_t> encryption_key;
        std::vector<uint8_t> mac_key;
        std::vector<uint8_t> iv;
    };

    static std::vector<uint8_t> encrypt(const std::vector<uint8_t>& message,
                                      const std::vector<uint8_t>& key,
                                      const std::vector<uint8_t>& iv);

    static std::vector<uint8_t> decrypt(const std::vector<uint8_t>& encrypted,
                                      const std::vector<uint8_t>& key,
                                      const std::vector<uint8_t>& iv);

    static std::vector<uint8_t> sign(const std::vector<uint8_t>& message,
                                   const std::vector<uint8_t>& mac_key);

    static bool verify(const std::vector<uint8_t>& message,
                      const std::vector<uint8_t>& received_mac,
                      const std::vector<uint8_t>& mac_key);

    static KeyUse::Keys extractServerKeys(const std::vector<std::vector<uint8_t>>& keys);
    static KeyUse::Keys extractClientKeys(const std::vector<std::vector<uint8_t>>& keys);

};

#endif
