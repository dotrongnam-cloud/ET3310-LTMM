#pragma once
#include <array>
#include <cstdint>
#include <iostream>

namespace des {
    uint64_t permute(uint64_t input, const int* table, int in_len, int out_len);
    void printHex(std::ostream& os, uint64_t value, int hex_digits);
}