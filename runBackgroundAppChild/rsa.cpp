#define _CRT_SECURE_NO_WARNINGS

#include "rsa.h"

bool rsa::miller_rabin(uint64_t n, int k)
{
    if (n <= 1 || n == 4) return false;
    if (n <= 3) return true;

    // Find d such that n-1 = 2^r * d
    uint64_t d = n - 1;
    int r = 0;
    while (d % 2 == 0) {
        d /= 2;
        r++;
    }

    // Witness selection
    std::random_device rd;
    std::mt19937_64 gen(rd());

    // Test k rounds
    for (int i = 0; i < k; i++) {
        uint64_t a = 2 + gen() % (n - 4);
        uint64_t x = mod_pow(a, d, n);

        if (x == 1 || x == n - 1)
            continue;

        bool probably_prime = false;
        for (int j = 0; j < r - 1; j++) {
            x = mod_pow(x, 2, n);
            if (x == n - 1) {
                probably_prime = true;
                break;
            }
        }

        if (!probably_prime)
            return false;
    }

    return true;
}

uint64_t rsa::find_prime(uint64_t min, uint64_t max)
{   
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(min, max);

    while (true) {
        // Generate a random odd number within the specified range
        uint64_t candidate = dis(gen) | 1;  // Ensure odd number in range

        // Quick small prime divisibility check
        if (candidate < 3 ||
            (candidate % 3 == 0) ||
            (candidate % 5 == 0) ||
            (candidate % 7 == 0) ||
            (candidate % 11 == 0)) {
            continue;
        }

        // Perform Miller-Rabin primality test with multiple rounds
        const int test_rounds = 20;  // Crypto-grade confidence
        if (miller_rabin(candidate, test_rounds)) {
            return candidate;
        }
    }
}

uint64_t rsa::mod_pow(uint64_t base, uint64_t exp, uint64_t mod)
{
   uint64_t result = 1;
    base %= mod;
    while (exp > 0) {
        if (exp & 1) result = (result * base) % mod;
        base = (base * base) % mod;
        exp >>= 1;
    }
    return result;
}

uint64_t rsa::mod_mul(uint64_t a, uint64_t b, uint64_t mod)
{
    uint64_t result = 0;
    a %= mod;

    while (b > 0) {
        if (b & 1) {
            result = (result + a) % mod;
        }
        a = (a + a) % mod;
        b >>= 1;
    }

    return result;
}

uint64_t rsa::gcd(uint64_t a, uint64_t b)
{
    while (b != 0) {
        uint64_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

uint64_t rsa::mod_inverse(uint64_t a, uint64_t m)
{
    int64_t m0 = m, t, q;
    int64_t x0 = 0, x1 = 1;

    if (m == 1) return 0;

    while (a > 1) {
        q = a / m;
        t = m;
        m = a % m;
        a = t;
        t = x0;
        x0 = x1 - q * x0;
        x1 = t;
    }

    if (x1 < 0) x1 += m0;
    return x1;
}

rsa::rsa() : gen(rd()), e(0), d(0), q(0), n(0), p(0), phi(0)
{
}

void rsa::generate_keys()
{
    p = find_prime();
    do {
        q = find_prime();
    } while (q == p);

    n = p * q;
    phi = (p - 1) * (q - 1);

    // Dynamically choose e
    std::uniform_int_distribution<uint64_t> dis(3, phi - 1);
    do {
        e = dis(gen);
    } while (gcd(e, phi) != 1);

    d = mod_inverse(e, phi);
}

uint64_t rsa::encrypt(uint64_t m)
{
    uint64_t temp = e;
    if (m >= n) {
        std::cerr << "Message too large" << std::endl;
        return 0;
    }

    uint64_t result = 1;
    uint64_t base = m;

    while (temp > 0) {
        if (temp & 1)
            result = mod_mul(result, base, n);

        base = mod_mul(base, base, n);
        temp >>= 1;
    }

    return result;
}

uint64_t rsa::decrypt(uint64_t c)
{
    uint64_t temp = d;

    if (c >= n) {
        std::cerr << "Ciphertext too large" << std::endl;
        return 0;
    }

    uint64_t result = 1;
    uint64_t base = c;

    while (temp > 0) {
        if (temp & 1)
            result = mod_mul(result, base, n);

        base = mod_mul(base, base, n);
        temp >>= 1;
    }

    return result;
}

std::pair<uint64_t, uint64_t> rsa::getPublicKey()
{
    return std::make_pair(n, e);
}

void rsa::setPublicKey(std::pair<uint64_t, uint64_t> publicKey) // e, n
{
    this->n = publicKey.first;
    this->e = publicKey.second;
}

void rsa::print_keys()
{
    std::cout << "Public Key (n, e): (" << n << ", " << e << ")\n";
    std::cout << "Private Key (n, d): (" << n << ", " << d << ")\n";
}

uint64_t rsa::signMessage(const std::string& message)
{
    // Compute hash of the message
    uint64_t hash = HashClass::computeSecureHash(message);

    std::cout << "hash : " << hash << std::endl;

    // Sign the hash using private key (decrypt with private key)
    return mod_pow(hash, d, n);
}

bool rsa::verify(uint64_t signature)
{
    //getCurrentDate
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&now_c);

    char buffer[11];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", now_tm);
    std::string departureDate = std::string(buffer);

    std::string certificateData = "parent" + departureDate;

    uint64_t hash = HashClass::computeSecureHash(certificateData);


    uint64_t recovered_hash = mod_pow(signature, e, n);
    recovered_hash = HashClass::computeSecureHash(certificateData);

    return (hash == recovered_hash);

}

