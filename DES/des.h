#pragma once
#include <array>
#include <cstdint>
#include <iostream>

namespace des {
    uint64_t permute(uint64_t input, const int* table, int in_len, int out_len);
    void printHex(std::ostream& os, uint64_t value, int hex_digits);

    uint32_t leftRotate28(uint32_t value, int shift);
    std::array<uint64_t, 16> generateSubkeys(uint64_t key, std::ostream* log = nullptr);

    uint32_t feistelF(uint32_t R, uint64_t subkey, std::ostream* log = nullptr);
    uint64_t processBlock(uint64_t block,
                          const std::array<uint64_t, 16>& subkeys,
                          bool decrypt,
                          std::ostream* log = nullptr);
}