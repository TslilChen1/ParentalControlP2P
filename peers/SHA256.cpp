#include "SHA256.h"


const void SHA256::processBlock(uint32_t* state, const uint8_t* block)
{
    uint32_t w[64];
    for (size_t i = 0; i < 16; i++) {
        w[i] = (block[i * 4] << 24) | (block[i * 4 + 1] << 16) |
            (block[i * 4 + 2] << 8) | block[i * 4 + 3];
    }
    for (size_t i = 16; i < 64; i++) {
        uint32_t s0 = (w[i - 15] >> 7) | (w[i - 15] << (32 - 7));
        s0 ^= (w[i - 15] >> 18) | (w[i - 15] << (32 - 18));
        s0 ^= (w[i - 15] >> 3);
        uint32_t s1 = (w[i - 2] >> 17) | (w[i - 2] << (32 - 17));
        s1 ^= (w[i - 2] >> 19) | (w[i - 2] << (32 - 19));
        s1 ^= (w[i - 2] >> 10);
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }

    uint32_t a = state[0];
    uint32_t b = state[1];
    uint32_t c = state[2];
    uint32_t d = state[3];
    uint32_t e = state[4];
    uint32_t f = state[5];
    uint32_t g = state[6];
    uint32_t h = state[7];

    for (size_t i = 0; i < 64; i++) {
        uint32_t S1 = (e >> 6) | (e << (32 - 6));
        S1 ^= (e >> 11) | (e << (32 - 11));
        S1 ^= (e >> 25) | (e << (32 - 25));
        uint32_t ch = (e & f) ^ (~e & g);
        uint32_t temp1 = h + S1 + ch + k[i] + w[i];
        uint32_t S0 = (a >> 2) | (a << (32 - 2));
        S0 ^= (a >> 13) | (a << (32 - 13));
        S0 ^= (a >> 22) | (a << (32 - 22));
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        uint32_t temp2 = S0 + maj;

        h = g;
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    state[5] += f;
    state[6] += g;
    state[7] += h;
}

const std::string SHA256::sha256(const std::string& input)
{
    uint32_t state[8] = {
       0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
       0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
    };

    size_t inputLen = input.size();
    size_t totalLen = inputLen * 8;
    size_t numBlocks = (totalLen + 64) / 512 + 1;
    std::vector<uint8_t> padded(numBlocks * 64, 0);
    memcpy(padded.data(), input.data(), inputLen);
    padded[inputLen] = 0x80;

    for (size_t i = 0; i < 8; i++) {
        padded[padded.size() - 8 + i] = (totalLen >> (56 - i * 8)) & 0xff;
    }

    for (size_t i = 0; i < numBlocks; i++) {
        processBlock(state, padded.data() + i * 64);
    }

    std::ostringstream oss;
    for (size_t i = 0; i < 8; i++) {
        oss << std::hex << std::setw(8) << std::setfill('0') << state[i];
    }
    return oss.str();
}
