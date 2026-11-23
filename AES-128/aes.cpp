#include "aes.h"
#include "aes_tables.h"
#include <iomanip>

namespace aes {

void printBlockHex(std::ostream& os, const Block& block) {
    os << std::uppercase << std::hex << std::setfill('0');
    for (int i = 0; i < 16; ++i) {
        os << std::setw(2) << static_cast<int>(block[i]);
        if (i % 4 == 3 && i != 15) os << " ";
    }
    os << std::dec << std::setfill(' ');
}

void printStateMatrix(std::ostream& os, const std::string& label, const State& state) {
    os << "   " << label << ":\n";
    os << std::uppercase << std::hex << std::setfill('0');
    for (int r = 0; r < 4; ++r) {
        os << "      [ ";
        for (int c = 0; c < 4; ++c) {
            os << std::setw(2) << static_cast<int>(state[r][c]) << " ";
        }
        os << "]\n";
    }
    os << std::dec << std::setfill(' ');
}

uint8_t xtime(uint8_t x) {
    // Nhân với x (0x02) trong GF(2^8): dịch trái 1 bit, nếu bit cao nhất là 1 thì XOR 0x1B
    return static_cast<uint8_t>((x << 1) ^ ((x & 0x80) ? 0x1B : 0x00));
}

uint8_t gfMul(uint8_t a, uint8_t b) {
    uint8_t res = 0;
    for (int i = 0; i < 8; ++i) {
        if (b & 1) res ^= a;
        a = xtime(a);
        b >>= 1;
    }
    return res;
}

RoundKeys keyExpansion(const Block& key, std::ostream* log) {
    // Mảng w gồm 44 từ (word), mỗi từ gồm 4 byte
    std::array<std::array<uint8_t, 4>, 44> w{};

    // 4 từ đầu w[0..3] lấy trực tiếp từ khóa gốc 16 byte
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            w[i][j] = key[4 * i + j];
        }
    }

    // Sinh tiếp 40 từ còn lại w[4..43]
    for (int i = 4; i < 44; ++i) {
        std::array<uint8_t, 4> temp = w[i - 1];

        if (i % 4 == 0) {
            // Hàm g(w): RotWord -> SubWord -> XOR Rcon[i/4]
            uint8_t first = temp[0];
            temp[0] = SBOX[temp[1]] ^ RCON[i / 4];
            temp[1] = SBOX[temp[2]];
            temp[2] = SBOX[temp[3]];
            temp[3] = SBOX[first];
        }

        for (int j = 0; j < 4; ++j) {
            w[i][j] = w[i - 4][j] ^ temp[j];
        }
    }

    // Gom cứ 4 từ liên tiếp thành 1 khóa vòng 16 byte (Round 0 .. Round 10)
    RoundKeys round_keys{};
    for (int r = 0; r <= 10; ++r) {
        for (int col = 0; col < 4; ++col) {
            for (int row = 0; row < 4; ++row) {
                round_keys[r][4 * col + row] = w[4 * r + col][row];
            }
        }
        if (log != nullptr) {
            *log << "Round Key " << std::setw(2) << r << " : ";
            printBlockHex(*log, round_keys[r]);
            *log << "\n";
        }
    }

    return round_keys;
}

} // namespace aes