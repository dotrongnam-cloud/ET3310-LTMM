#pragma once
#include <array>
#include <cstdint>
#include <iostream>
#include <string>

namespace aes {

using Block     = std::array<uint8_t, 16>;
using State     = std::array<std::array<uint8_t, 4>, 4>;
using RoundKeys = std::array<Block, 11>;

// Hàm tiện ích in kiểm định
void printBlockHex(std::ostream& os, const Block& block);
void printStateMatrix(std::ostream& os, const std::string& label, const State& state);

// Phép nhân trong trường hữu hạn GF(2^8) với đa thức m(x) = x^8 + x^4 + x^3 + x + 1
uint8_t xtime(uint8_t x);
uint8_t gfMul(uint8_t a, uint8_t b);

// Mở rộng khóa 128-bit thành 11 khóa vòng (Round 0 .. Round 10)
RoundKeys keyExpansion(const Block& key, std::ostream* log = nullptr);

} // namespace aes