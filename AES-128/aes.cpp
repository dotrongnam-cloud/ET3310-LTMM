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

State bytesToState(const Block& input) {
    State state{};
    // Nạp 16 byte đầu vào ma trận 4x4 theo thứ tự từng cột (column-major)
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            state[row][col] = input[4 * col + row];
        }
    }
    return state;
}

Block stateToBytes(const State& state) {
    Block output{};
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            output[4 * col + row] = state[row][col];
        }
    }
    return output;
}

void addRoundKey(State& state, const Block& round_key) {
    for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
            state[row][col] ^= round_key[4 * col + row];
        }
    }
}

void subBytes(State& state) {
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            state[r][c] = SBOX[state[r][c]];
        }
    }
}

void invSubBytes(State& state) {
    for (int r = 0; r < 4; ++r) {
        for (int c = 0; c < 4; ++c) {
            state[r][c] = INV_SBOX[state[r][c]];
        }
    }
}

void shiftRows(State& state) {
    // Hàng 0: giữ nguyên
    // Hàng 1: dịch vòng trái 1 byte
    uint8_t t = state[1][0];
    state[1][0] = state[1][1];
    state[1][1] = state[1][2];
    state[1][2] = state[1][3];
    state[1][3] = t;

    // Hàng 2: dịch vòng trái 2 byte
    std::swap(state[2][0], state[2][2]);
    std::swap(state[2][1], state[2][3]);

    // Hàng 3: dịch vòng trái 3 byte (tương đương dịch phải 1 byte)
    t = state[3][3];
    state[3][3] = state[3][2];
    state[3][2] = state[3][1];
    state[3][1] = state[3][0];
    state[3][0] = t;
}

void invShiftRows(State& state) {
    // Hàng 1: dịch vòng phải 1 byte
    uint8_t t = state[1][3];
    state[1][3] = state[1][2];
    state[1][2] = state[1][1];
    state[1][1] = state[1][0];
    state[1][0] = t;

    // Hàng 2: dịch vòng phải 2 byte
    std::swap(state[2][0], state[2][2]);
    std::swap(state[2][1], state[2][3]);

    // Hàng 3: dịch vòng phải 3 byte (tương đương dịch trái 1 byte)
    t = state[3][0];
    state[3][0] = state[3][1];
    state[3][1] = state[3][2];
    state[3][2] = state[3][3];
    state[3][3] = t;
}

void mixColumns(State& state) {
    for (int c = 0; c < 4; ++c) {
        uint8_t s0 = state[0][c];
        uint8_t s1 = state[1][c];
        uint8_t s2 = state[2][c];
        uint8_t s3 = state[3][c];

        state[0][c] = gfMul(0x02, s0) ^ gfMul(0x03, s1) ^ s2 ^ s3;
        state[1][c] = s0 ^ gfMul(0x02, s1) ^ gfMul(0x03, s2) ^ s3;
        state[2][c] = s0 ^ s1 ^ gfMul(0x02, s2) ^ gfMul(0x03, s3);
        state[3][c] = gfMul(0x03, s0) ^ s1 ^ s2 ^ gfMul(0x02, s3);
    }
}

void invMixColumns(State& state) {
    for (int c = 0; c < 4; ++c) {
        uint8_t s0 = state[0][c];
        uint8_t s1 = state[1][c];
        uint8_t s2 = state[2][c];
        uint8_t s3 = state[3][c];

        state[0][c] = gfMul(0x0E, s0) ^ gfMul(0x0B, s1) ^ gfMul(0x0D, s2) ^ gfMul(0x09, s3);
        state[1][c] = gfMul(0x09, s0) ^ gfMul(0x0E, s1) ^ gfMul(0x0B, s2) ^ gfMul(0x0D, s3);
        state[2][c] = gfMul(0x0D, s0) ^ gfMul(0x09, s1) ^ gfMul(0x0E, s2) ^ gfMul(0x0B, s3);
        state[3][c] = gfMul(0x0B, s0) ^ gfMul(0x0D, s1) ^ gfMul(0x09, s2) ^ gfMul(0x0E, s3);
    }
}

Block encryptBlock(const Block& plaintext, const RoundKeys& round_keys, std::ostream* log) {
    State state = bytesToState(plaintext);

    if (log != nullptr) {
        *log << "Round 0 (Initial AddRoundKey)\n";
        printStateMatrix(*log, "Input State", state);
    }

    addRoundKey(state, round_keys[0]);

    if (log != nullptr) {
        printStateMatrix(*log, "After AddRoundKey (K0)", state);
    }

    // Vòng 1 đến vòng 9: đầy đủ 4 bước
    for (int r = 1; r <= 9; ++r) {
        if (log != nullptr) {
            *log << "-------------------------------------------------------------\n";
            *log << "Round " << r << "\n";
        }

        subBytes(state);
        if (log != nullptr) printStateMatrix(*log, "After SubBytes", state);

        shiftRows(state);
        if (log != nullptr) printStateMatrix(*log, "After ShiftRows", state);

        mixColumns(state);
        if (log != nullptr) printStateMatrix(*log, "After MixColumns", state);

        addRoundKey(state, round_keys[r]);
        if (log != nullptr) printStateMatrix(*log, "After AddRoundKey (K" + std::to_string(r) + ")", state);
    }

    // Vòng 10 (vòng cuối): bỏ MixColumns
    if (log != nullptr) {
        *log << "-------------------------------------------------------------\n";
        *log << "Round 10 (Final Round - No MixColumns)\n";
    }

    subBytes(state);
    if (log != nullptr) printStateMatrix(*log, "After SubBytes", state);

    shiftRows(state);
    if (log != nullptr) printStateMatrix(*log, "After ShiftRows", state);

    addRoundKey(state, round_keys[10]);
    if (log != nullptr) printStateMatrix(*log, "After AddRoundKey (K10)", state);

    return stateToBytes(state);
}

Block decryptBlock(const Block& ciphertext, const RoundKeys& round_keys, std::ostream* log) {
    State state = bytesToState(ciphertext);

    if (log != nullptr) {
        *log << "Initial AddRoundKey (K10)\n";
        printStateMatrix(*log, "Ciphertext State", state);
    }

    addRoundKey(state, round_keys[10]);

    if (log != nullptr) {
        printStateMatrix(*log, "After AddRoundKey (K10)", state);
    }

    // Giải mã vòng ngược từ khóa K9 về K1
    for (int r = 9; r >= 1; --r) {
        if (log != nullptr) {
            *log << "-------------------------------------------------------------\n";
            *log << "Decryption Round " << (10 - r) << " (using K" << r << ")\n";
        }

        invShiftRows(state);
        if (log != nullptr) printStateMatrix(*log, "After InvShiftRows", state);

        invSubBytes(state);
        if (log != nullptr) printStateMatrix(*log, "After InvSubBytes", state);

        addRoundKey(state, round_keys[r]);
        if (log != nullptr) printStateMatrix(*log, "After AddRoundKey (K" + std::to_string(r) + ")", state);

        invMixColumns(state);
        if (log != nullptr) printStateMatrix(*log, "After InvMixColumns", state);
    }

    // Vòng cuối giải mã (dùng K0, không có InvMixColumns)
    if (log != nullptr) {
        *log << "-------------------------------------------------------------\n";
        *log << "Decryption Round 10 (Final Round - using K0)\n";
    }

    invShiftRows(state);
    if (log != nullptr) printStateMatrix(*log, "After InvShiftRows", state);

    invSubBytes(state);
    if (log != nullptr) printStateMatrix(*log, "After InvSubBytes", state);

    addRoundKey(state, round_keys[0]);
    if (log != nullptr) printStateMatrix(*log, "After AddRoundKey (K0)", state);

    return stateToBytes(state);
}
} // namespace aes