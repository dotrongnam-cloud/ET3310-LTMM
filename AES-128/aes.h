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

// Phép nhân trong trường hữu hạn GF(2^8)
uint8_t xtime(uint8_t x);
uint8_t gfMul(uint8_t a, uint8_t b);

// Mở rộng khóa 128-bit thành 11 khóa vòng
RoundKeys keyExpansion(const Block& key, std::ostream* log = nullptr);

// Chuyển đổi giữa mảng 16 byte và ma trận trạng thái 4x4 (theo từng cột)
State bytesToState(const Block& input);
Block stateToBytes(const State& state);

// Các phép biến đổi vòng (Mã hóa & Giải mã)
void addRoundKey(State& state, const Block& round_key);
void subBytes(State& state);
void invSubBytes(State& state);
void shiftRows(State& state);
void invShiftRows(State& state);
void mixColumns(State& state);
void invMixColumns(State& state);

// Mã hóa và giải mã 1 khối 128-bit kèm ghi log từng trạng thái
Block encryptBlock(const Block& plaintext, const RoundKeys& round_keys, std::ostream* log = nullptr);
Block decryptBlock(const Block& ciphertext, const RoundKeys& round_keys, std::ostream* log = nullptr);

} // namespace aes