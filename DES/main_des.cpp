#include "des.h"
#include "des_tables.h"
#include <iostream>

int main() {
    uint64_t plaintext = 0x0123456789ABCDEFULL;
    uint64_t key       = 0x133457799BBCDFF1ULL;

    uint64_t ip   = des::permute(plaintext, des::IP, 64, 64);
    uint64_t back = des::permute(ip, des::IP_INV, 64, 64);

    std::cout << "Plaintext : "; des::printHex(std::cout, plaintext, 16); std::cout << "\n";
    std::cout << "IP        : "; des::printHex(std::cout, ip, 16);        std::cout << "\n";
    std::cout << "IP_INV    : "; des::printHex(std::cout, back, 16);      std::cout << "\n";

    std::cout << "\n=== KEY SCHEDULE ===\n";
    des::generateSubkeys(key, &std::cout);

    return 0;
}